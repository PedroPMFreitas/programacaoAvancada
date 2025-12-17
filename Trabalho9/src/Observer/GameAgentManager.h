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
#include "Core/GridType.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>

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
    
    void updateAll(float deltaTime) {
        // Atualiza movimento dos agentes
        for (auto& agent : agents) {
            updateAgent(agent.get(), deltaTime);
        }
        
        // Processa colisões
        if (collisionEnabled) {
            std::vector<GameAgent*> agentPtrs;
            for (auto& agent : agents) {
                agentPtrs.push_back(agent.get());
            }
            CollisionManager::getInstance()->processCollisions(agentPtrs);
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
                if (agent->isAlive()) {
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
};

#endif // GAME_AGENT_MANAGER_H
