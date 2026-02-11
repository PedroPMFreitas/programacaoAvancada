#ifndef SIMULATION_LOGGER_H
#define SIMULATION_LOGGER_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

// =============================================================================
// SimulationLogger — Registra métricas de desempenho para exportação CSV
// =============================================================================
// Coleta dados ao final de cada bateria de testes e salva em CSV.
// Usado para gerar gráficos comparativos entre os 3 métodos de evasão.
// =============================================================================

// Estrutura com os dados de uma simulação individual
struct SimulationRecord {
    std::string metodoUtilizado;         // "Direta", "Indireta", "Sem_Comunicacao"
    int quantidadeAgentes;               // Número de agentes na cena
    float tempoComputacionalMedio_ms;    // Tempo médio do algoritmo por frame (ms)
    int totalColisoes;                   // Colisões que ocorreram de fato
    float tempoTotalConclusao_s;         // Tempo do início até último agente chegar
    float distanciaExtraPercorrida;      // Diferença entre distância ideal e real (média)
};

class SimulationLogger {
private:
    std::vector<SimulationRecord> records;
    static SimulationLogger* instance;

    SimulationLogger() = default;

public:
    SimulationLogger(const SimulationLogger&) = delete;
    SimulationLogger& operator=(const SimulationLogger&) = delete;

    static SimulationLogger* getInstance() {
        if (!instance) {
            instance = new SimulationLogger();
        }
        return instance;
    }

    // Adiciona um registro de simulação
    void addRecord(const SimulationRecord& record) {
        records.push_back(record);
        std::cout << "[SimulationLogger] Registro adicionado: "
                  << record.metodoUtilizado
                  << " | Agentes=" << record.quantidadeAgentes
                  << " | TempoCPU=" << std::fixed << std::setprecision(3) 
                  << record.tempoComputacionalMedio_ms << "ms"
                  << " | Colisoes=" << record.totalColisoes
                  << " | Conclusao=" << std::setprecision(2) 
                  << record.tempoTotalConclusao_s << "s"
                  << " | DistExtra=" << std::setprecision(1) 
                  << record.distanciaExtraPercorrida << "px"
                  << std::endl;
    }

    // Salva todos os registros em arquivo CSV
    void saveToCSV(const std::string& filename = "resultados_simulacao.csv") {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[SimulationLogger] ERRO: Nao foi possivel abrir " 
                      << filename << std::endl;
            return;
        }

        // Cabeçalho do CSV
        file << "Metodo_Utilizado,"
             << "Quantidade_Agentes,"
             << "Tempo_Computacional_Medio_ms,"
             << "Total_Colisoes,"
             << "Tempo_Total_Conclusao_s,"
             << "Distancia_Extra_Percorrida"
             << "\n";

        // Dados
        for (const auto& record : records) {
            file << record.metodoUtilizado << ","
                 << record.quantidadeAgentes << ","
                 << std::fixed << std::setprecision(4) 
                 << record.tempoComputacionalMedio_ms << ","
                 << record.totalColisoes << ","
                 << std::setprecision(4) 
                 << record.tempoTotalConclusao_s << ","
                 << std::setprecision(2) 
                 << record.distanciaExtraPercorrida
                 << "\n";
        }

        file.close();
        std::cout << "[SimulationLogger] Dados salvos em: " << filename
                  << " (" << records.size() << " registros)" << std::endl;
    }

    // Limpa todos os registros
    void clear() {
        records.clear();
    }

    // Quantidade de registros
    int getRecordCount() const { return static_cast<int>(records.size()); }

    // Acesso ao vetor de registros
    const std::vector<SimulationRecord>& getRecords() const { return records; }
};

inline SimulationLogger* SimulationLogger::instance = nullptr;

#endif // SIMULATION_LOGGER_H
