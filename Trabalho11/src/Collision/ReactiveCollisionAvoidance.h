#ifndef REACTIVE_COLLISION_AVOIDANCE_H
#define REACTIVE_COLLISION_AVOIDANCE_H

#include "ICollisionAvoidance.h"
#include <iostream>
#include <cmath>
#include <vector>

// =============================================================================
// Método 3: Sem Comunicação — Evasão Reativa Local (Sensor de Proximidade)
// =============================================================================
//
// Padrão de Projeto:
//   - Strategy: ICollisionAvoidance permite trocar o algoritmo em runtime
//
// CONCEITO: A solução atua isoladamente. Os agentes NÃO trocam intenções,
// NÃO negociam, NÃO deixam rastros, e NÃO usam estruturas compartilhadas.
//
// Cada agente possui apenas um "sensor de proximidade" com raio máximo
// definido pelo tamanho e velocidade do agente. O sensor detecta obstáculos
// (distância e direção) — sem saber O QUE é o obstáculo. O agente ajusta
// seu vetor de movimento de forma puramente reativa.
//
// Isso é análogo a um robô com sensores infravermelhos: ele detecta
// "algo está a X metros na direção Y" e desvia, sem nenhuma comunicação.
//
// Características:
//   - Sem troca de mensagens (diferente do Método 1)
//   - Sem escrita/leitura em estrutura compartilhada (diferente do Método 2)
//   - Cada agente decide sozinho, baseado apenas no seu sensor
//   - Pode haver oscilações e deadlocks (limitação natural)
// =============================================================================

// Dado de detecção do sensor: "algo está a esta distância nesta direção"
// O sensor NÃO identifica o que é — apenas distância e ângulo
struct SensorReading {
    float distance;    // Distância ao obstáculo detectado
    float directionX;  // Componente X da direção do obstáculo (normalizado)
    float directionY;  // Componente Y da direção do obstáculo (normalizado)
};

// ============================================================================
// Sensor de Proximidade: encapsula a percepção local do agente
// ============================================================================
// O sensor varre o ambiente ao redor do agente e retorna apenas
// leituras de distância e direção. Não expõe dados de outros agentes.
// ============================================================================
class ProximitySensor {
private:
    float maxRange;  // Raio máximo de detecção

public:
    ProximitySensor(float range) : maxRange(range) {}

    // Varre o ambiente e retorna leituras do sensor
    // Recebe apenas uma lista de posições (não de agentes!)
    // O sensor não sabe o que cada posição representa
    std::vector<SensorReading> scan(Vector2 myPosition,
                                     const std::vector<Vector2>& obstaclePositions) const {
        std::vector<SensorReading> readings;

        for (const auto& obstPos : obstaclePositions) {
            float dx = obstPos.x - myPosition.x;
            float dy = obstPos.y - myPosition.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            // Só detecta dentro do raio máximo
            if (dist < maxRange && dist > 0.001f) {
                readings.push_back({
                    dist,
                    dx / dist,  // Direção normalizada X
                    dy / dist   // Direção normalizada Y
                });
            }
        }

        return readings;
    }

    float getMaxRange() const { return maxRange; }
    void setMaxRange(float r) { maxRange = r; }
};

// ============================================================================
// Strategy concreta: Sem Comunicação — Evasão Reativa com Sensor Local
// ============================================================================
class ReactiveCollisionAvoidance : public ICollisionAvoidance {
private:
    bool active;
    float timeStep;
    float agentRadius;
    float maxSpeed;

    // Parâmetros reativos
    float detectionRadius;       // Raio de detecção do sensor (baseado em tamanho + velocidade)
    float repulsionStrength;     // Intensidade da reação de evasão
    float criticalDistance;      // Distância de emergência (reação máxima)

    // Cache
    std::vector<Vector2> correctedVels;
    std::vector<GameAgent*> storedAgents;
    std::vector<Vector2> storedPreferredVels;

    // Sensor de proximidade (cada agente conceitualmente tem o seu)
    std::unique_ptr<ProximitySensor> sensor;

public:
    ReactiveCollisionAvoidance()
        : active(true), timeStep(1.0f / 60.0f), agentRadius(8.0f), maxSpeed(2.0f),
          detectionRadius(50.0f), repulsionStrength(1.5f), criticalDistance(15.0f) {}

    std::string getName() const override {
        return "Sem Comunicacao";
    }

    void initialize(float ts, float radius, float speed) override {
        timeStep = ts;
        agentRadius = radius;
        maxSpeed = speed;

        // O raio de detecção é definido pelo tamanho e velocidade máxima do agente
        // Raio = agentRadius * fator + maxSpeed * horizonte temporal
        detectionRadius = agentRadius * 3.0f + maxSpeed * 15.0f;
        criticalDistance = agentRadius * 2.5f;

        // Cria o sensor com o raio calculado
        sensor = std::make_unique<ProximitySensor>(detectionRadius);

        std::cout << "[Metodo 3] Sem Comunicacao - Evasao Reativa Local" << std::endl;
        std::cout << "  Padrao: Strategy" << std::endl;
        std::cout << "  Cada agente usa sensor de proximidade isolado" << std::endl;
        std::cout << "  detectionRadius=" << detectionRadius
                  << " criticalDistance=" << criticalDistance
                  << " repulsionStrength=" << repulsionStrength << std::endl;
    }

    void syncAgents(const std::vector<GameAgent*>& agents) override {
        storedAgents.clear();
        storedAgents.reserve(agents.size());
        for (auto* agent : agents) {
            if (agent->isAlive()) {
                storedAgents.push_back(agent);
            }
        }
    }

    void setPreferredVelocities(
        const std::vector<GameAgent*>& agents,
        const std::vector<Vector2>& preferredVelocities) override {
        storedPreferredVels = preferredVelocities;
    }

    void doStep() override {
        if (!active || !sensor) return;

        correctedVels.clear();
        correctedVels.reserve(storedAgents.size());

        // Pré-coleta todas as posições (o sensor só vê posições, não agentes)
        std::vector<Vector2> allPositions;
        allPositions.reserve(storedAgents.size());
        for (auto* agent : storedAgents) {
            allPositions.push_back(agent->getPosition());
        }

        for (size_t i = 0; i < storedAgents.size(); ++i) {
            Vector2 myPos = storedAgents[i]->getPosition();
            Vector2 prefVel = (i < storedPreferredVels.size()) ?
                storedPreferredVels[i] : Vector2{0.0f, 0.0f};

            // Monta lista de posições de obstáculos (exclui a própria posição)
            std::vector<Vector2> obstacles;
            obstacles.reserve(allPositions.size() - 1);
            for (size_t j = 0; j < allPositions.size(); ++j) {
                if (j != i) {
                    obstacles.push_back(allPositions[j]);
                }
            }

            // === SENSOR DE PROXIMIDADE: varre o ambiente ===
            // Retorna apenas leituras de distância+direção
            // O agente NÃO sabe o que detectou — apenas "algo está ali"
            auto readings = sensor->scan(myPos, obstacles);

            // === REAÇÃO AUTÔNOMA: calcula vetor de evasão ===
            Vector2 totalRepulsion = {0.0f, 0.0f};
            int readingsInCritical = 0;

            for (const auto& reading : readings) {
                float forceMagnitude;

                if (reading.distance < criticalDistance) {
                    // Zona crítica: reação de emergência (forte mas controlada)
                    forceMagnitude = repulsionStrength * 2.0f *
                        (1.0f - reading.distance / criticalDistance);
                    readingsInCritical++;
                } else {
                    // Zona normal: reação proporcional inversa
                    float normalizedDist = reading.distance / detectionRadius;
                    forceMagnitude = repulsionStrength * (1.0f - normalizedDist);
                }

                // Direção: AFASTA do obstáculo detectado
                totalRepulsion.x -= reading.directionX * forceMagnitude;
                totalRepulsion.y -= reading.directionY * forceMagnitude;
            }

            // Combina velocidade desejada com reação do sensor
            Vector2 correctedVel = {
                prefVel.x + totalRepulsion.x,
                prefVel.y + totalRepulsion.y
            };

            // Anti-deadlock: se muitos obstáculos próximos e agente quase parado,
            // aplica desvio lateral para quebrar simetria
            if (readingsInCritical >= 2) {
                float velMag = std::sqrt(correctedVel.x * correctedVel.x +
                                        correctedVel.y * correctedVel.y);
                if (velMag < maxSpeed * 0.2f && velMag > 0.001f) {
                    float perpX = -correctedVel.y / velMag;
                    float perpY = correctedVel.x / velMag;
                    // Alterna lado baseado no índice para não criar nova simetria
                    float sign = (i % 2 == 0) ? 1.0f : -1.0f;
                    correctedVel.x += perpX * maxSpeed * 0.3f * sign;
                    correctedVel.y += perpY * maxSpeed * 0.3f * sign;
                }
            }

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
    void setDetectionRadius(float r) {
        detectionRadius = r;
        if (sensor) sensor->setMaxRange(r);
    }
    void setRepulsionStrength(float s) { repulsionStrength = s; }
    void setCriticalDistance(float d) { criticalDistance = d; }

    float getDetectionRadius() const { return detectionRadius; }
    float getRepulsionStrength() const { return repulsionStrength; }
    float getCriticalDistance() const { return criticalDistance; }
};

#endif // REACTIVE_COLLISION_AVOIDANCE_H
