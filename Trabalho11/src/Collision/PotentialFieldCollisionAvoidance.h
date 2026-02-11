#ifndef POTENTIAL_FIELD_COLLISION_AVOIDANCE_H
#define POTENTIAL_FIELD_COLLISION_AVOIDANCE_H

#include "ICollisionAvoidance.h"
#include <memory>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

// =============================================================================
// Método 2: Comunicação Indireta — Blackboard (Grade de Ocupação Compartilhada)
// =============================================================================
//
// Padrões de Projeto:
//   - Strategy: ICollisionAvoidance permite trocar o algoritmo em runtime
//   - Blackboard: A OccupancyGrid é uma estrutura de dados compartilhada
//     que funciona como um "quadro-negro". Os agentes NÃO conversam entre
//     si — eles leem e escrevem informações neste ambiente comum.
//   - Singleton-like: A grade de ocupação é uma instância única compartilhada
//     por todos os agentes, com acesso controlado.
//
// CONCEITO: A comunicação ocorre através de uma estrutura de dados
// compartilhada. Os agentes não conversam entre si, mas leem e escrevem
// informações neste ambiente comum.
//
// Funcionamento:
//   1. Cada agente ESCREVE no Blackboard: reserva seu próximo espaço na
//      grade de ocupação.
//   2. Cada agente LÊ o Blackboard: verifica quais células vizinhas estão
//      reservadas por outros agentes.
//   3. Se a célula desejada está reservada, o agente ajusta sua rota para
//      evitar conflito.
//   4. Os agentes NUNCA se comunicam diretamente — toda informação passa
//      pela grade compartilhada.
// =============================================================================

// Tipo para identificar células na grade
struct GridCellKey {
    int x, y;
    bool operator==(const GridCellKey& other) const {
        return x == other.x && y == other.y;
    }
};

// Hash para GridCellKey (necessário para unordered_map)
struct GridCellKeyHash {
    size_t operator()(const GridCellKey& key) const {
        return std::hash<int>()(key.x) ^ (std::hash<int>()(key.y) << 16);
    }
};

// Dados de reserva no Blackboard
struct CellReservation {
    GameAgent* owner;       // Quem reservou (para o agente ignorar sua própria reserva)
    float expirationTime;   // Quando a reserva expira (em frames)
};

// ============================================================================
// Blackboard: Grade de Ocupação Compartilhada
// ============================================================================
// Estrutura de dados compartilhada onde os agentes escrevem suas intenções
// (reservas de células) e leem as intenções dos outros.
// Nenhum agente acessa dados de outro agente diretamente.
// ============================================================================
class OccupancyGrid {
private:
    int width, height;
    float cellSize;
    // Mapa de reservas: célula -> lista de reservas
    std::unordered_map<GridCellKey, std::vector<CellReservation>, GridCellKeyHash> reservations;
    int currentFrame = 0;

public:
    OccupancyGrid() : width(0), height(0), cellSize(1.0f) {}

    void initialize(int w, int h, float cs) {
        width = w;
        height = h;
        cellSize = cs;
        reservations.clear();
    }

    // Avança o frame e limpa reservas expiradas
    void beginFrame() {
        currentFrame++;
        // Remove reservas expiradas
        for (auto it = reservations.begin(); it != reservations.end();) {
            auto& resList = it->second;
            resList.erase(
                std::remove_if(resList.begin(), resList.end(),
                    [this](const CellReservation& r) {
                        return r.expirationTime < currentFrame;
                    }),
                resList.end()
            );
            if (resList.empty()) {
                it = reservations.erase(it);
            } else {
                ++it;
            }
        }
    }

    // ESCRITA no Blackboard: Um agente reserva uma célula
    // (Comunicação indireta: agente escreve no ambiente compartilhado)
    bool reserveCell(int cellX, int cellY, GameAgent* owner, float durationFrames = 3.0f) {
        if (cellX < 0 || cellX >= width || cellY < 0 || cellY >= height) return false;

        GridCellKey key = {cellX, cellY};
        reservations[key].push_back({owner, static_cast<float>(currentFrame) + durationFrames});
        return true;
    }

    // LEITURA do Blackboard: Um agente verifica se uma célula está reservada
    // (Comunicação indireta: agente lê do ambiente compartilhado)
    // Retorna true se a célula está reservada por OUTRO agente
    bool isCellReserved(int cellX, int cellY, GameAgent* queryAgent) const {
        GridCellKey key = {cellX, cellY};
        auto it = reservations.find(key);
        if (it == reservations.end()) return false;

        for (auto& res : it->second) {
            if (res.owner != queryAgent && res.expirationTime >= currentFrame) {
                return true;
            }
        }
        return false;
    }

    // LEITURA: Retorna o nível de ocupação de uma célula (0.0 = livre, 1.0+ = ocupada)
    float getCellOccupancy(int cellX, int cellY, GameAgent* queryAgent) const {
        GridCellKey key = {cellX, cellY};
        auto it = reservations.find(key);
        if (it == reservations.end()) return 0.0f;

        float occupancy = 0.0f;
        for (auto& res : it->second) {
            if (res.owner != queryAgent && res.expirationTime >= currentFrame) {
                occupancy += 1.0f;
            }
        }
        return occupancy;
    }

    // Converte posição do mundo para célula da grade
    GridCellKey worldToCell(Vector2 worldPos) const {
        return {
            static_cast<int>(worldPos.x / cellSize),
            static_cast<int>(worldPos.y / cellSize)
        };
    }

    // Converte célula para posição central no mundo
    Vector2 cellToWorld(int cellX, int cellY) const {
        return {
            cellX * cellSize + cellSize / 2.0f,
            cellY * cellSize + cellSize / 2.0f
        };
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    float getCellSize() const { return cellSize; }
};

// ============================================================================
// Strategy concreta: Comunicação Indireta usando Blackboard (OccupancyGrid)
// ============================================================================
class PotentialFieldCollisionAvoidance : public ICollisionAvoidance {
private:
    OccupancyGrid blackboard;   // O "quadro-negro" compartilhado
    bool active;
    float timeStep;
    float agentRadius;
    float maxSpeed;

    // Parâmetros
    float reservationRadius;     // Quantas células ao redor reservar
    float avoidanceStrength;     // Força de desvio quando célula está ocupada
    int lookAheadCells;          // Quantas células à frente verificar

    // Cache
    std::vector<Vector2> correctedVels;
    std::vector<GameAgent*> storedAgents;
    std::vector<Vector2> storedPreferredVels;

public:
    PotentialFieldCollisionAvoidance()
        : active(true), timeStep(1.0f / 60.0f), agentRadius(8.0f), maxSpeed(2.0f),
          reservationRadius(1.0f), avoidanceStrength(0.8f), lookAheadCells(2) {}

    std::string getName() const override {
        return "Comunicacao Indireta";
    }

    void initialize(float ts, float radius, float speed) override {
        timeStep = ts;
        agentRadius = radius;
        maxSpeed = speed;

        // A grade de ocupação tem resolução baseada no raio do agente
        float cellSize = radius * 2.0f;
        int gridW = static_cast<int>(800.0f / cellSize) + 1;
        int gridH = static_cast<int>(600.0f / cellSize) + 1;

        blackboard.initialize(gridW, gridH, cellSize);

        std::cout << "[Metodo 2] Comunicacao Indireta - Blackboard (Grade de Ocupacao)" << std::endl;
        std::cout << "  Padrao: Strategy + Blackboard" << std::endl;
        std::cout << "  Agentes reservam celulas na grade compartilhada" << std::endl;
        std::cout << "  gridSize=" << gridW << "x" << gridH
                  << " cellSize=" << cellSize
                  << " lookAhead=" << lookAheadCells << std::endl;
    }

    void syncAgents(const std::vector<GameAgent*>& agents) override {
        storedAgents.clear();
        storedAgents.reserve(agents.size());

        // Início do frame: limpa reservas expiradas
        blackboard.beginFrame();

        // Fase de ESCRITA: Cada agente reserva suas células no Blackboard
        // (Comunicação indireta: agentes escrevem no ambiente compartilhado)
        for (auto* agent : agents) {
            if (!agent->isAlive()) continue;
            storedAgents.push_back(agent);

            Vector2 pos = agent->getPosition();
            GridCellKey cell = blackboard.worldToCell(pos);

            // Reserva a célula atual e vizinhas (presença do agente)
            int radius = static_cast<int>(reservationRadius);
            for (int dy = -radius; dy <= radius; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    blackboard.reserveCell(cell.x + dx, cell.y + dy, agent, 1.5f);
                }
            }
        }
    }

    void setPreferredVelocities(
        const std::vector<GameAgent*>& agents,
        const std::vector<Vector2>& preferredVelocities) override {

        storedPreferredVels = preferredVelocities;

        // Fase de ESCRITA adicional: cada agente reserva as células do seu
        // caminho futuro (comunica sua intenção de rota ao Blackboard)
        for (size_t i = 0; i < storedAgents.size() && i < preferredVelocities.size(); ++i) {
            Vector2 pos = storedAgents[i]->getPosition();
            Vector2 vel = preferredVelocities[i];

            float velMag = std::sqrt(vel.x * vel.x + vel.y * vel.y);
            if (velMag < 0.001f) continue;

            // Reserva células ao longo do caminho pretendido
            for (int step = 1; step <= lookAheadCells; ++step) {
                float factor = static_cast<float>(step) * blackboard.getCellSize();
                Vector2 futurePos = {
                    pos.x + (vel.x / velMag) * factor,
                    pos.y + (vel.y / velMag) * factor
                };
                GridCellKey futureCell = blackboard.worldToCell(futurePos);
                blackboard.reserveCell(futureCell.x, futureCell.y, storedAgents[i], 2.0f);
            }
        }
    }

    void doStep() override {
        if (!active) return;

        correctedVels.clear();
        correctedVels.reserve(storedAgents.size());

        // Fase de LEITURA: cada agente lê o Blackboard para ajustar sua rota
        // (Comunicação indireta: agentes leem do ambiente compartilhado)
        for (size_t i = 0; i < storedAgents.size(); ++i) {
            GameAgent* agent = storedAgents[i];
            Vector2 pos = agent->getPosition();
            Vector2 prefVel = (i < storedPreferredVels.size()) ?
                storedPreferredVels[i] : Vector2{0.0f, 0.0f};

            float velMag = std::sqrt(prefVel.x * prefVel.x + prefVel.y * prefVel.y);

            if (velMag < 0.001f) {
                correctedVels.push_back(prefVel);
                continue;
            }

            // Verifica ocupação das células à frente no caminho desejado
            Vector2 avoidanceForce = {0.0f, 0.0f};
            bool pathBlocked = false;

            for (int step = 1; step <= lookAheadCells; ++step) {
                float factor = static_cast<float>(step) * blackboard.getCellSize();
                Vector2 checkPos = {
                    pos.x + (prefVel.x / velMag) * factor,
                    pos.y + (prefVel.y / velMag) * factor
                };
                GridCellKey checkCell = blackboard.worldToCell(checkPos);

                // LÊ o Blackboard: "esta célula está reservada por outro agente?"
                float occupancy = blackboard.getCellOccupancy(
                    checkCell.x, checkCell.y, agent);

                if (occupancy > 0.0f) {
                    pathBlocked = true;
                    // Peso decresce com a distância (prioriza desvio do mais próximo)
                    float weight = avoidanceStrength * (1.0f - static_cast<float>(step - 1) / lookAheadCells);

                    // Calcula direção perpendicular para desvio
                    Vector2 checkWorld = blackboard.cellToWorld(checkCell.x, checkCell.y);
                    float dx = checkWorld.x - pos.x;
                    float dy = checkWorld.y - pos.y;
                    float dist = std::sqrt(dx * dx + dy * dy);

                    if (dist > 0.001f) {
                        // Força perpendicular ao vetor que aponta para a célula ocupada
                        // Escolhe lado baseado no índice do agente para quebrar simetria
                        float sign = (i % 2 == 0) ? 1.0f : -1.0f;
                        avoidanceForce.x += (-dy / dist) * weight * occupancy * sign;
                        avoidanceForce.y += (dx / dist) * weight * occupancy * sign;
                    }
                }
            }

            // Também verifica células vizinhas laterais
            GridCellKey currentCell = blackboard.worldToCell(pos);
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    float occ = blackboard.getCellOccupancy(
                        currentCell.x + dx, currentCell.y + dy, agent);
                    if (occ > 0.0f) {
                        // Repulsão suave das células vizinhas ocupadas
                        avoidanceForce.x -= dx * avoidanceStrength * 0.15f * occ;
                        avoidanceForce.y -= dy * avoidanceStrength * 0.15f * occ;
                    }
                }
            }

            // Combina velocidade desejada com força de evasão
            Vector2 correctedVel = {
                prefVel.x + avoidanceForce.x,
                prefVel.y + avoidanceForce.y
            };

            // Limita à velocidade máxima
            float speed = std::sqrt(correctedVel.x * correctedVel.x +
                                    correctedVel.y * correctedVel.y);
            if (speed > maxSpeed) {
                correctedVel.x = (correctedVel.x / speed) * maxSpeed;
                correctedVel.y = (correctedVel.y / speed) * maxSpeed;
            }

            correctedVels.push_back(correctedVel);
        }
    }

    std::vector<Vector2> getCorrectedVelocities() override {
        return correctedVels;
    }

    bool isActive() const override { return active; }
    void setActive(bool a) override { active = a; }

    // Parâmetros para ajuste
    void setReservationRadius(float r) { reservationRadius = r; }
    void setAvoidanceStrength(float s) { avoidanceStrength = s; }
    void setLookAheadCells(int n) { lookAheadCells = n; }

    // Acesso ao Blackboard para debug/visualização
    const OccupancyGrid& getBlackboard() const { return blackboard; }
};

#endif // POTENTIAL_FIELD_COLLISION_AVOIDANCE_H
