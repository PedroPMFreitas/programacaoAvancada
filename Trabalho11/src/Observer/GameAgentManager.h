#ifndef GAME_AGENT_MANAGER_H
#define GAME_AGENT_MANAGER_H

#include "GameAgent.h"
#include "AgentRespawnObserver.h"
#include "AgentEventLogger.h"
#include "GameStatisticsObserver.h"
#include "Trabalho9_Legacy.h"
#include "src/Interfaces/IGridAdapter.h"
#include "src/Adapters/HexagonalGridAdapter.h"
#include "src/Collision/CollisionManager.h"
#include "src/Collision/CollisionObserver.h"
#include "src/Collision/ICollisionAvoidance.h"
#include "Core/GridType.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <set>

// Gerenciador de agentes do jogo com suporte a Observer e diferentes tipos de grid
class GameAgentManager {
private:
    std::vector<std::unique_ptr<GameAgent>> agents;
    IGridAdapter* gridAdapter;
    GridType gridType;
    
    // Observers globais que serão adicionados a todos os agentes
    std::unique_ptr<AgentRespawnObserver> respawnObserver;
    std::unique_ptr<AgentEventLogger> eventLogger;
    std::unique_ptr<GameStatisticsObserver> statsObserver;
    
    // Sistema de colisão
    std::unique_ptr<CollisionObserver> collisionObserver;
    bool collisionEnabled = true;
    
    // Strategy de evasão de colisão (RVO2, etc.)
    std::unique_ptr<ICollisionAvoidance> collisionAvoidance;
    bool collisionAvoidanceEnabled = false;
    
    // === Métricas de desempenho para SimulationLogger ===
    int collisionCount = 0;                  // Total de colisões únicas detectadas
    double totalAvoidanceTimeMs = 0.0;       // Soma do tempo gasto no algoritmo de evasão (ms)
    int avoidanceFrameCount = 0;             // Quantos frames o algoritmo rodou
    float collisionDetectionRadius = 8.0f;   // Raio para contar colisões reais (= raio do agente)
    // Rastreia pares que já estão em colisão para não contar duplicatas por frame
    std::set<std::pair<GameAgent*, GameAgent*>> activeCollisionPairs;

public:
    GameAgentManager(IGridAdapter* adapter, GridType type = GridType::RECTANGULAR) 
        : gridAdapter(adapter), gridType(type) {
        // Inicializa observers globais
        respawnObserver = std::make_unique<AgentRespawnObserver>(3.0f);
        eventLogger = std::make_unique<AgentEventLogger>(true, false);
        statsObserver = std::make_unique<GameStatisticsObserver>();
        
        // Inicializa sistema de colisão
        collisionObserver = std::make_unique<CollisionObserver>(true);
        CollisionManager::getInstance()->addObserver(collisionObserver.get());
        
        // Configura raios baseado no tipo de grid
        if (type == GridType::HEXAGONAL) {
            CollisionManager::getInstance()->setWarningRadius(20.0f);
            CollisionManager::getInstance()->setCollisionRadius(10.0f);
        } else {
            CollisionManager::getInstance()->setWarningRadius(25.0f);
            CollisionManager::getInstance()->setCollisionRadius(12.0f);
        }
    }
    
    ~GameAgentManager() {
        if (collisionObserver) {
            CollisionManager::getInstance()->removeObserver(collisionObserver.get());
        }
    }
    
    void setGridAdapter(IGridAdapter* adapter, GridType type) {
        gridAdapter = adapter;
        gridType = type;
    }
    
    // Controle do sistema de colisão
    void setCollisionEnabled(bool enabled) { 
        collisionEnabled = enabled; 
        CollisionManager::getInstance()->setEnabled(enabled);
    }
    bool isCollisionEnabled() const { return collisionEnabled; }
    
    void toggleCollisionVisualization() {
        auto* cm = CollisionManager::getInstance();
        bool current = cm->getShowWarningZone();
        cm->setShowWarningZone(!current);
        cm->setShowCollisionZone(!current);
    }
    
    // Converte coordenadas do grid para posição no mundo
    Vector2 gridToWorld(int col, int row) const {
        if (gridType == GridType::HEXAGONAL) {
            auto* hexAdapter = dynamic_cast<HexagonalGridAdapter*>(gridAdapter);
            if (hexAdapter) {
                return hexAdapter->hexToPixel(col, row);
            }
        }
        // Grid retangular
        float cellSize = gridAdapter->GetCellSize();
        return {col * cellSize + cellSize / 2, row * cellSize + cellSize / 2};
    }
    
    // Converte posição no mundo para coordenadas do grid
    Cell worldToGrid(Vector2 worldPos) const {
        if (gridType == GridType::HEXAGONAL) {
            auto* hexAdapter = dynamic_cast<HexagonalGridAdapter*>(gridAdapter);
            if (hexAdapter) {
                return hexAdapter->pixelToHex(worldPos.x, worldPos.y);
            }
        }
        // Grid retangular
        float cellSize = gridAdapter->GetCellSize();
        return {(int)(worldPos.x / cellSize), (int)(worldPos.y / cellSize)};
    }
    
    // Encontra caminho usando o adapter apropriado
    std::vector<Vector2> findPath(Vector2 startGrid, Vector2 endGrid) {
        if (gridType == GridType::HEXAGONAL) {
            auto* hexAdapter = dynamic_cast<HexagonalGridAdapter*>(gridAdapter);
            if (hexAdapter) {
                return hexAdapter->FindPathHex(startGrid, endGrid);
            }
        }
        // Grid retangular - usa o pathfinder legacy
        return Pathfinder::FindPath(gridAdapter->GetLegacyGrid(), startGrid, endGrid, "random");
    }
    
    GameAgent* addAgent(Vector2 start, Vector2 target) {
        Vector2 worldStart = gridToWorld((int)start.x, (int)start.y);
        
        auto agent = std::make_unique<GameAgent>(worldStart, target);
        
        // Adiciona os observers ao novo agente
        agent->addObserver(respawnObserver.get());
        agent->addObserver(eventLogger.get());
        agent->addObserver(statsObserver.get());
        
        // Notifica spawn
        agent->notifyObservers(AgentEvents::AGENT_SPAWNED, agent.get());
        
        GameAgent* agentPtr = agent.get();
        agents.push_back(std::move(agent));
        
        return agentPtr;
    }
    
    void addRandomAgents(int count) {
        for (int i = 0; i < count; i++) {
            Vector2 start, target;
            do {
                start = {
                    (float)GetRandomValue(0, gridAdapter->GetWidth() - 1),
                    (float)GetRandomValue(0, gridAdapter->GetHeight() - 1)
                };
            } while (!gridAdapter->IsWalkable((int)start.x, (int)start.y));
            
            do {
                target = {
                    (float)GetRandomValue(0, gridAdapter->GetWidth() - 1),
                    (float)GetRandomValue(0, gridAdapter->GetHeight() - 1)
                };
            } while (!gridAdapter->IsWalkable((int)target.x, (int)target.y) ||
                     (start.x == target.x && start.y == target.y));
            
            addAgent(start, target);
        }
    }
    
    void removeAgent(GameAgent* agent) {
        agents.erase(
            std::remove_if(agents.begin(), agents.end(),
                [agent](const std::unique_ptr<GameAgent>& a) {
                    return a.get() == agent;
                }),
            agents.end()
        );
    }
    
    // === Strategy Pattern: Evasão de colisão ===
    void setCollisionAvoidance(std::unique_ptr<ICollisionAvoidance> strategy) {
        collisionAvoidance = std::move(strategy);
        if (collisionAvoidance) {
            float cellSize = gridAdapter->GetCellSize();
            collisionAvoidance->initialize(1.0f / 60.0f, cellSize / 3.0f, 2.0f);
            collisionAvoidanceEnabled = true;
            std::cout << "[Strategy] Evasao de colisao: " 
                      << collisionAvoidance->getName() << std::endl;
        }
    }
    
    ICollisionAvoidance* getCollisionAvoidance() const {
        return collisionAvoidance.get();
    }
    
    void setCollisionAvoidanceEnabled(bool enabled) {
        collisionAvoidanceEnabled = enabled;
        if (collisionAvoidance) {
            collisionAvoidance->setActive(enabled);
        }
    }
    
    bool isCollisionAvoidanceEnabled() const { return collisionAvoidanceEnabled; }
    
    void updateAll(float deltaTime) {
        // Coleta agentes vivos
        std::vector<GameAgent*> aliveAgents;
        for (auto& agent : agents) {
            if (agent->isAlive() && !agent->hasReachedTarget()) {
                aliveAgents.push_back(agent.get());
            }
        }
        
        // Se evasão de colisão está ativa, usa o Strategy (RVO2)
        if (collisionAvoidanceEnabled && collisionAvoidance && collisionAvoidance->isActive() && !aliveAgents.empty()) {
            // Mede tempo do algoritmo de evasão
            auto startTime = std::chrono::high_resolution_clock::now();
            updateWithCollisionAvoidance(aliveAgents, deltaTime);
            auto endTime = std::chrono::high_resolution_clock::now();
            double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            totalAvoidanceTimeMs += elapsedMs;
            avoidanceFrameCount++;
            
            // Conta colisões reais entre agentes (para verificar qualidade do método)
            countCollisions(aliveAgents);
        } else {
            // Atualiza movimento dos agentes sem evasão
            for (auto& agent : agents) {
                updateAgent(agent.get(), deltaTime);
            }
        }
        
        // Processa colisões antigas APENAS se evasão de colisão NÃO está ativa
        // O sistema antigo (CollisionObserver::handleCollision) empurra agentes 5px,
        // o que conflita com as velocidades calculadas pelos métodos de evasão (RVO2, etc.)
        if (collisionEnabled && !(collisionAvoidanceEnabled && collisionAvoidance && collisionAvoidance->isActive())) {
            std::vector<GameAgent*> agentPtrs;
            for (auto& agent : agents) {
                if (agent->isAlive() && !agent->hasReachedTarget()) {
                    agentPtrs.push_back(agent.get());
                }
            }
            CollisionManager::getInstance()->processCollisions(agentPtrs);
        }
    }
    
    // Atualiza agentes usando o Strategy de evasão de colisão (RVO2)
    void updateWithCollisionAvoidance(std::vector<GameAgent*>& aliveAgents, float deltaTime) {
        // 1. Sincroniza posições dos agentes com o simulador RVO2
        collisionAvoidance->syncAgents(aliveAgents);
        
        // 2. Calcula velocidades desejadas (direção ao próximo waypoint do path)
        std::vector<Vector2> preferredVelocities;
        preferredVelocities.reserve(aliveAgents.size());
        
        for (auto* agent : aliveAgents) {
            Vector2 prefVel = {0.0f, 0.0f};
            
            if (!agent->getHasPath()) {
                // Tenta encontrar caminho
                Cell gridCell = worldToGrid(agent->getPosition());
                Vector2 startGrid = {(float)gridCell.x, (float)gridCell.y};
                auto newPath = findPath(startGrid, agent->getTarget());
                
                if (!newPath.empty()) {
                    agent->setPath(newPath);
                } else {
                    agent->pathBlocked();
                }
            }
            
            if (agent->getHasPath()) {
                const auto& path = agent->getPath();
                int currentIdx = agent->getCurrentPathIndex();
                
                if (currentIdx < (int)path.size()) {
                    Vector2 nextCell = path[currentIdx];
                    Vector2 targetWorldPos = gridToWorld((int)nextCell.x, (int)nextCell.y);
                    
                    Vector2 pos = agent->getPosition();
                    Vector2 direction = {targetWorldPos.x - pos.x, targetWorldPos.y - pos.y};
                    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    
                    if (distance < 5.0f) {
                        agent->setCurrentPathIndex(currentIdx + 1);
                        // Recalcula para o próximo waypoint
                        if (currentIdx + 1 < (int)path.size()) {
                            nextCell = path[currentIdx + 1];
                            targetWorldPos = gridToWorld((int)nextCell.x, (int)nextCell.y);
                            direction = {targetWorldPos.x - pos.x, targetWorldPos.y - pos.y};
                            distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                        } else {
                            distance = 0.0f;
                        }
                    }
                    
                    if (distance > 0.001f) {
                        // Velocidade desejada = direção normalizada * velocidade do agente
                        prefVel.x = (direction.x / distance) * agent->getSpeed();
                        prefVel.y = (direction.y / distance) * agent->getSpeed();
                    }
                } else {
                    agent->setHasPath(false);
                    agent->reachTarget();
                }
            }
            
            preferredVelocities.push_back(prefVel);
        }
        
        // 3. Envia velocidades desejadas ao RVO2
        collisionAvoidance->setPreferredVelocities(aliveAgents, preferredVelocities);
        
        // 4. RVO2 calcula velocidades corrigidas (com evasão)
        collisionAvoidance->doStep();
        
        // 5. Aplica velocidades corrigidas aos agentes
        auto correctedVelocities = collisionAvoidance->getCorrectedVelocities();
        
        for (size_t i = 0; i < aliveAgents.size() && i < correctedVelocities.size(); ++i) {
            GameAgent* agent = aliveAgents[i];
            Vector2 vel = correctedVelocities[i];
            
            // Aplica a velocidade corrigida pelo RVO2
            Vector2 pos = agent->getPosition();
            Vector2 newPos = {
                pos.x + vel.x * deltaTime * 60.0f,
                pos.y + vel.y * deltaTime * 60.0f
            };
            agent->setPosition(newPos);
        }
    }
    
    void updateAgent(GameAgent* agent, float deltaTime) {
        if (!agent->isAlive()) return;
        if (agent->hasReachedTarget()) return;  // Já chegou ao destino
        
        if (!agent->getHasPath()) {
            // Converte posição do agente para coordenadas do grid
            Cell gridCell = worldToGrid(agent->getPosition());
            Vector2 startGrid = {(float)gridCell.x, (float)gridCell.y};
            
            auto newPath = findPath(startGrid, agent->getTarget());
            
            if (newPath.empty()) {
                agent->pathBlocked();
                return;
            }
            
            agent->setPath(newPath);
            return;
        }
        
        const auto& path = agent->getPath();
        int currentIdx = agent->getCurrentPathIndex();
        
        if (currentIdx < (int)path.size()) {
            Vector2 nextCell = path[currentIdx];
            Vector2 targetWorldPos = gridToWorld((int)nextCell.x, (int)nextCell.y);
            
            Vector2 pos = agent->getPosition();
            Vector2 direction = {targetWorldPos.x - pos.x, targetWorldPos.y - pos.y};
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            
            if (distance < 5.0f) {
                agent->setCurrentPathIndex(currentIdx + 1);
            } else {
                direction.x /= distance;
                direction.y /= distance;
                
                Vector2 newPos = {
                    pos.x + direction.x * agent->getSpeed() * deltaTime * 60.0f,
                    pos.y + direction.y * agent->getSpeed() * deltaTime * 60.0f
                };
                agent->setPosition(newPos);
            }
        } else {
            if (agent->getHasPath()) {
                agent->setHasPath(false);
                agent->reachTarget();
            }
        }
    }
    
    void drawAll() {
        for (auto& agent : agents) {
            if (!agent->isAlive()) continue;
            if (agent->hasReachedTarget()) continue;  // Agente chegou ao destino - não desenha
            
            Vector2 pos = agent->getPosition();
            
            if (gridType == GridType::HEXAGONAL) {
                // Desenha agente como hexágono
                auto* hexAdapter = dynamic_cast<HexagonalGridAdapter*>(gridAdapter);
                if (hexAdapter) {
                    float hexRadius = hexAdapter->GetHexRadius() - 2;
                    // Hexágono preenchido com cor do agente
                    DrawPoly(pos, 6, hexRadius, 0.0f, agent->getColor());
                    // Borda do hexágono
                    DrawPolyLines(pos, 6, hexRadius, 0.0f, BLACK);
                    
                    // Desenha barra de vida
                    float healthBarWidth = hexRadius * 2.0f;
                    float healthBarHeight = 3;
                    float healthPercent = (float)agent->getHealth() / agent->getMaxHealth();
                    
                    DrawRectangle(
                        pos.x - healthBarWidth / 2, 
                        pos.y - hexRadius - 6, 
                        healthBarWidth, 
                        healthBarHeight, 
                        DARKGRAY
                    );
                    DrawRectangle(
                        pos.x - healthBarWidth / 2, 
                        pos.y - hexRadius - 6, 
                        healthBarWidth * healthPercent, 
                        healthBarHeight, 
                        (healthPercent > 0.5f) ? GREEN : (healthPercent > 0.25f) ? YELLOW : RED
                    );
                }
            } else {
                // Desenha agente como círculo (grid retangular)
                float radius = gridAdapter->GetCellSize() / 3.0f;
                DrawCircle(pos.x, pos.y, radius, agent->getColor());
                
                // Desenha barra de vida
                float healthBarWidth = radius * 2.5f;
                float healthBarHeight = 4;
                float healthPercent = (float)agent->getHealth() / agent->getMaxHealth();
                
                DrawRectangle(
                    pos.x - healthBarWidth / 2, 
                    pos.y - radius - 10, 
                    healthBarWidth, 
                    healthBarHeight, 
                    DARKGRAY
                );
                DrawRectangle(
                    pos.x - healthBarWidth / 2, 
                    pos.y - radius - 10, 
                    healthBarWidth * healthPercent, 
                    healthBarHeight, 
                    (healthPercent > 0.5f) ? GREEN : (healthPercent > 0.25f) ? YELLOW : RED
                );
            }
        }
        
        // Desenha zonas de colisão se habilitado
        if (collisionEnabled) {
            std::vector<GameAgent*> agentPtrs;
            for (auto& agent : agents) {
                if (agent->isAlive() && !agent->hasReachedTarget()) {
                    agentPtrs.push_back(agent.get());
                }
            }
            CollisionManager::getInstance()->drawCollisionZones(agentPtrs);
        }
    }
    
    // Causa dano a um agente específico (para testes)
    void damageAgent(int index, int damage) {
        if (index >= 0 && index < (int)agents.size()) {
            agents[index]->takeDamage(damage);
        }
    }
    
    // Causa dano a todos os agentes
    void damageAllAgents(int damage) {
        for (auto& agent : agents) {
            agent->takeDamage(damage);
        }
    }
    
    int getAgentCount() const { return agents.size(); }
    
    GameAgent* getAgent(int index) {
        if (index >= 0 && index < (int)agents.size()) {
            return agents[index].get();
        }
        return nullptr;
    }
    
    void printStatistics() const {
        statsObserver->printStatistics();
    }
    
    GameStatisticsObserver* getStatsObserver() const {
        return statsObserver.get();
    }
    
    // === Métricas de desempenho para benchmarks ===
    
    // Conta colisões reais (únicas por par de agentes) — para medir qualidade do método
    // Incrementa o contador apenas quando dois agentes COMEÇAM a colidir,
    // não a cada frame que estão sobrepostos
    void countCollisions(const std::vector<GameAgent*>& aliveAgents) {
        float radius = collisionDetectionRadius;
        std::set<std::pair<GameAgent*, GameAgent*>> currentPairs;
        
        for (size_t i = 0; i < aliveAgents.size(); ++i) {
            for (size_t j = i + 1; j < aliveAgents.size(); ++j) {
                Vector2 p1 = aliveAgents[i]->getPosition();
                Vector2 p2 = aliveAgents[j]->getPosition();
                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < radius * 2.0f) {
                    auto pair = std::make_pair(
                        std::min(aliveAgents[i], aliveAgents[j]),
                        std::max(aliveAgents[i], aliveAgents[j]));
                    currentPairs.insert(pair);
                    // Só conta se é uma colisão NOVA (não existia no frame anterior)
                    if (activeCollisionPairs.find(pair) == activeCollisionPairs.end()) {
                        collisionCount++;
                    }
                }
            }
        }
        activeCollisionPairs = currentPairs;
    }
    
    // Tempo médio do algoritmo de evasão por frame (em ms)
    double getAverageAvoidanceTimeMs() const {
        if (avoidanceFrameCount == 0) return 0.0;
        return totalAvoidanceTimeMs / avoidanceFrameCount;
    }
    
    // Total de colisões detectadas
    int getCollisionCount() const { return collisionCount; }
    
    // Reseta métricas para nova bateria de testes
    void resetMetrics() {
        collisionCount = 0;
        totalAvoidanceTimeMs = 0.0;
        avoidanceFrameCount = 0;
        activeCollisionPairs.clear();
    }
    
    // Calcula distância extra média percorrida por todos os agentes
    float getAverageExtraDistance() const {
        float totalExtra = 0.0f;
        int count = 0;
        for (const auto& agent : agents) {
            if (agent->hasReachedTarget() || agent->getTotalDistanceTraveled() > 0.1f) {
                totalExtra += agent->getExtraDistance();
                count++;
            }
        }
        return (count > 0) ? totalExtra / count : 0.0f;
    }
    
    // Verifica se todos os agentes chegaram ao destino
    bool allAgentsReachedTarget() const {
        if (agents.empty()) return true;
        for (const auto& agent : agents) {
            if (agent->isAlive() && !agent->hasReachedTarget()) {
                return false;
            }
        }
        return true;
    }
    
    // Quantidade de agentes que chegaram ao destino
    int getReachedTargetCount() const {
        int count = 0;
        for (const auto& agent : agents) {
            if (agent->hasReachedTarget()) count++;
        }
        return count;
    }
    
    // Remove todos os agentes (para reset entre testes)
    void clearAllAgents() {
        agents.clear();
    }
    
    // Calcula a distância ideal (linha reta) para cada agente após spawn
    // Deve ser chamado depois de addAgent/addRandomAgents quando as posições mundiais são conhecidas
    void calculateIdealDistances() {
        for (auto& agent : agents) {
            Vector2 spawnWorld = agent->getSpawnPosition();
            Vector2 targetGrid = agent->getTarget();
            Vector2 targetWorld = gridToWorld((int)targetGrid.x, (int)targetGrid.y);
            float dx = targetWorld.x - spawnWorld.x;
            float dy = targetWorld.y - spawnWorld.y;
            agent->setIdealDistance(std::sqrt(dx * dx + dy * dy));
        }
    }
};

#endif // GAME_AGENT_MANAGER_H
