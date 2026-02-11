#ifndef SIMULATION_BENCHMARK_H
#define SIMULATION_BENCHMARK_H

#include "src/Observer/GameAgentManager.h"
#include "src/Collision/SimulationLogger.h"
#include "src/Collision/RVO2CollisionAvoidance.h"
#include "src/Collision/PotentialFieldCollisionAvoidance.h"
#include "src/Collision/ReactiveCollisionAvoidance.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <functional>

// =============================================================================
// SimulationBenchmark — Executa baterias de testes automatizadas
// =============================================================================
// Testa os 3 métodos de evasão de colisão com diferentes quantidades de agentes.
// Coleta métricas e salva via SimulationLogger para gerar gráficos.
// =============================================================================
class SimulationBenchmark {
private:
    GameAgentManager* agentManager;
    IGridAdapter* gridAdapter;
    GridType gridType;
    
    // Configurações do benchmark
    std::vector<int> agentCounts = {5, 10, 15, 20, 30};  // Quantidades de agentes para testar
    int maxFrames = 3600;        // Máximo de frames por teste (60s a 60fps)
    float timeoutSeconds = 60.0f;  // Timeout por teste
    float deltaTime = 1.0f / 60.0f;
    
    struct MethodConfig {
        std::string name;       // Nome para o CSV
        std::function<std::unique_ptr<ICollisionAvoidance>()> factory;
    };

public:
    SimulationBenchmark(GameAgentManager* mgr, IGridAdapter* adapter, GridType type)
        : agentManager(mgr), gridAdapter(adapter), gridType(type) {}
    
    void setAgentCounts(const std::vector<int>& counts) { agentCounts = counts; }
    void setMaxFrames(int frames) { maxFrames = frames; }
    void setTimeoutSeconds(float t) { timeoutSeconds = t; }
    
    // Executa a bateria completa de testes
    void runFullBenchmark() {
        std::cout << "\n========================================================" << std::endl;
        std::cout << "  INICIANDO BATERIA DE TESTES DE EVASAO DE COLISAO" << std::endl;
        std::cout << "========================================================" << std::endl;
        std::cout << "Metodos: Direta, Indireta, Sem_Comunicacao" << std::endl;
        std::cout << "Agentes: ";
        for (int c : agentCounts) std::cout << c << " ";
        std::cout << std::endl;
        std::cout << "Max frames por teste: " << maxFrames << std::endl;
        std::cout << "========================================================\n" << std::endl;
        
        // Define os 3 métodos
        std::vector<MethodConfig> methods = {
            {
                "Direta",
                []() { return std::make_unique<RVO2CollisionAvoidance>(); }
            },
            {
                "Indireta",
                []() { return std::make_unique<PotentialFieldCollisionAvoidance>(); }
            },
            {
                "Sem_Comunicacao",
                []() { return std::make_unique<ReactiveCollisionAvoidance>(); }
            }
        };
        
        SimulationLogger::getInstance()->clear();
        
        int totalTests = methods.size() * agentCounts.size();
        int currentTest = 0;
        
        for (const auto& method : methods) {
            for (int numAgents : agentCounts) {
                currentTest++;
                std::cout << "[Benchmark " << currentTest << "/" << totalTests << "] "
                          << method.name << " com " << numAgents << " agentes..." << std::endl;
                
                runSingleTest(method, numAgents);
            }
        }
        
        // Salva resultados
        SimulationLogger::getInstance()->saveToCSV("resultados_simulacao.csv");
        
        std::cout << "\n========================================================" << std::endl;
        std::cout << "  BATERIA DE TESTES CONCLUIDA!" << std::endl;
        std::cout << "  " << totalTests << " testes executados." << std::endl;
        std::cout << "  Resultados salvos em: resultados_simulacao.csv" << std::endl;
        std::cout << "========================================================\n" << std::endl;
    }

private:
    void runSingleTest(const MethodConfig& method, int numAgents) {
        // 1. Limpa estado anterior
        agentManager->clearAllAgents();
        agentManager->resetMetrics();
        
        // 2. Cria agentes aleatórios
        agentManager->addRandomAgents(numAgents);
        
        // 3. Calcula distâncias ideais (linha reta)
        agentManager->calculateIdealDistances();
        
        // 4. Ativa o método de evasão
        auto strategy = method.factory();
        agentManager->setCollisionAvoidance(std::move(strategy));
        
        // 5. Executa a simulação frame a frame
        auto wallClockStart = std::chrono::high_resolution_clock::now();
        
        int frameCount = 0;
        bool allReached = false;
        
        while (frameCount < maxFrames && !allReached) {
            agentManager->updateAll(deltaTime);
            frameCount++;
            
            // Verifica se todos chegaram
            allReached = agentManager->allAgentsReachedTarget();
            
            // Timeout por tempo de parede
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - wallClockStart).count();
            if (elapsed > timeoutSeconds) {
                std::cout << "  [TIMEOUT] " << timeoutSeconds << "s excedido no frame " 
                          << frameCount << std::endl;
                break;
            }
        }
        
        auto wallClockEnd = std::chrono::high_resolution_clock::now();
        
        // 6. Coleta métricas
        float tempoSimulacao = frameCount * deltaTime;  // Tempo simulado
        double tempoReal = std::chrono::duration<double>(wallClockEnd - wallClockStart).count();
        
        SimulationRecord record;
        record.metodoUtilizado = method.name;
        record.quantidadeAgentes = numAgents;
        record.tempoComputacionalMedio_ms = static_cast<float>(agentManager->getAverageAvoidanceTimeMs());
        record.totalColisoes = agentManager->getCollisionCount();
        record.tempoTotalConclusao_s = tempoSimulacao;
        record.distanciaExtraPercorrida = agentManager->getAverageExtraDistance();
        
        SimulationLogger::getInstance()->addRecord(record);
        
        // Log do resultado
        int reached = agentManager->getReachedTargetCount();
        std::cout << "  Resultado: " << reached << "/" << numAgents << " chegaram"
                  << " | " << frameCount << " frames"
                  << " | " << std::fixed << std::setprecision(2) << tempoSimulacao << "s simulados"
                  << " | " << std::setprecision(3) << tempoReal << "s reais"
                  << std::endl;
        
        // 7. Desativa evasão e limpa
        agentManager->setCollisionAvoidanceEnabled(false);
    }
};

#endif // SIMULATION_BENCHMARK_H
