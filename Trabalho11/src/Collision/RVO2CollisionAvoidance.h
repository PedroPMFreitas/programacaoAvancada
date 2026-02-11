#ifndef RVO2_COLLISION_AVOIDANCE_H
#define RVO2_COLLISION_AVOIDANCE_H

#include "ICollisionAvoidance.h"
#include "RVO.h"
#include <memory>
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <functional>

// =============================================================================
// Método 1: Comunicação Direta — RVO2 (ORCA) + Padrão Mediator
// =============================================================================
//
// Padrões de Projeto:
//   - Strategy: ICollisionAvoidance permite trocar o algoritmo em runtime
//   - Mediator: O CollisionNegotiationMediator centraliza a comunicação
//     entre agentes para negociação de velocidades. Os agentes não se
//     acoplam entre si — comunicam-se apenas através do mediador.
//
// CONCEITO: A solução se dá por comunicação direta. Os agentes trocam
// mensagens (posição, velocidade desejada) e negociam diretamente entre si
// para resolver quem tem prioridade de passagem e como desviar.
//
// O Mediator encapsula o simulador RVO2, que implementa o algoritmo ORCA
// (Optimal Reciprocal Collision Avoidance). Cada agente registra sua
// intenção de movimento no mediador, e o mediador resolve reciprocamente
// as velocidades seguras para todos.
//
// Integração com RVO2 (conforme documentação oficial):
//   1. Inicialização: Instanciar RVO::RVOSimulator e configurar parâmetros
//      de tempo e vizinhança.
//   2. Registro: addAgent(position) retorna um índice (ID) para cada agente.
//   3. Loop Principal (Update):
//      a) Calcular velocidade preferencial do agente em direção ao alvo
//         (ignorando obstáculos temporariamente).
//      b) Enviar intenção ao RVO2: sim->setAgentPrefVelocity(id, velocity).
//      c) Chamar sim->doStep() — negociação direta entre agentes.
//      d) Recuperar velocidade segura: sim->getAgentVelocity(id).
//      e) Atualizar renderização com os valores seguros.
// =============================================================================

// Dados de intenção de movimento que um agente comunica ao mediador
struct AgentMovementIntent {
    GameAgent* agent;
    Vector2 position;
    Vector2 preferredVelocity;
    size_t rvoIndex;  // ID retornado pelo RVO2 ao registrar o agente
};

// Dados de resultado da negociação que o mediador retorna ao agente
struct NegotiatedVelocity {
    GameAgent* agent;
    Vector2 safeVelocity;  // Velocidade segura calculada pela negociação
};

// ============================================================================
// Mediator: Centraliza a negociação de velocidades entre agentes
// ============================================================================
// Os agentes NÃO conhecem uns aos outros — apenas comunicam com o mediador.
// O mediador usa internamente o RVO2 para resolver as velocidades ORCA.
// Isso desacopla os agentes entre si (princípio do padrão Mediator).
// ============================================================================
class CollisionNegotiationMediator {
private:
    std::unique_ptr<RVO::RVOSimulator> simulator;
    std::unordered_map<GameAgent*, size_t> agentToRvoId;
    std::vector<AgentMovementIntent> registeredIntents;
    bool needsRebuild = true;

    // Parâmetros do RVO2
    float neighborDist;
    size_t maxNeighbors;
    float timeHorizon;
    float timeHorizonObst;
    float agentRadius;
    float maxSpeed;
    float timeStep;

public:
    CollisionNegotiationMediator()
        : neighborDist(50.0f), maxNeighbors(10), timeHorizon(5.0f),
          timeHorizonObst(5.0f), agentRadius(8.0f), maxSpeed(2.0f),
          timeStep(1.0f / 60.0f) {
        simulator = std::make_unique<RVO::RVOSimulator>();
    }

    // Configura parâmetros do simulador (Inicialização)
    void configure(float ts, float radius, float speed) {
        timeStep = ts;
        agentRadius = radius;
        maxSpeed = speed;
        needsRebuild = true;
    }

    // Um agente se registra no mediador (comunicação direta: "estou aqui")
    void registerAgent(GameAgent* agent, Vector2 position) {
        if (agentToRvoId.find(agent) == agentToRvoId.end()) {
            needsRebuild = true;
        }
    }

    // Um agente envia sua intenção de movimento ao mediador
    // (comunicação direta: "quero ir nesta direção com esta velocidade")
    void sendMovementIntent(GameAgent* agent, Vector2 position, Vector2 preferredVelocity) {
        registeredIntents.push_back({agent, position, preferredVelocity, 0});
    }

    // O mediador resolve todas as negociações usando RVO2 (ORCA)
    // e retorna as velocidades seguras para cada agente
    std::vector<NegotiatedVelocity> negotiate() {
        std::vector<NegotiatedVelocity> results;
        if (registeredIntents.empty()) return results;

        // Reconstrói o simulador se necessário (agentes foram adicionados/removidos)
        rebuildSimulator();

        // Passo 1: Atualiza posições e envia intenções ao RVO2
        // (Cada agente "comunica" sua posição e velocidade desejada ao mediador)
        for (auto& intent : registeredIntents) {
            auto it = agentToRvoId.find(intent.agent);
            if (it != agentToRvoId.end()) {
                size_t rvoId = it->second;
                // Atualiza posição no simulador
                simulator->setAgentPosition(rvoId,
                    RVO::Vector2(intent.position.x, intent.position.y));
                // Envia intenção: setAgentPrefVelocity(id, velocity)
                simulator->setAgentPrefVelocity(rvoId,
                    RVO::Vector2(intent.preferredVelocity.x, intent.preferredVelocity.y));
            }
        }

        // Passo 2: doStep() — O RVO2 resolve a negociação ORCA
        // Aqui ocorre a comunicação direta: cada par de agentes vizinhos
        // negocia reciprocamente suas velocidades para evitar colisão
        simulator->doStep();

        // Passo 3: Recupera velocidades seguras negociadas
        // getAgentVelocity(id) retorna a velocidade resolvida e segura
        for (auto& intent : registeredIntents) {
            auto it = agentToRvoId.find(intent.agent);
            if (it != agentToRvoId.end()) {
                RVO::Vector2 safeVel = simulator->getAgentVelocity(it->second);
                results.push_back({
                    intent.agent,
                    {static_cast<float>(safeVel.x()), static_cast<float>(safeVel.y())}
                });
            }
        }

        // Limpa intenções para o próximo frame
        registeredIntents.clear();
        return results;
    }

    void clearAgents() {
        agentToRvoId.clear();
        registeredIntents.clear();
        needsRebuild = true;
    }

    // Configuração avançada
    void setNeighborDist(float d) { neighborDist = d; }
    void setMaxNeighbors(size_t n) { maxNeighbors = n; }
    void setTimeHorizon(float t) { timeHorizon = t; }
    float getNeighborDist() const { return neighborDist; }
    size_t getMaxNeighbors() const { return maxNeighbors; }
    float getTimeHorizon() const { return timeHorizon; }

private:
    void rebuildSimulator() {
        // Verifica se os agentes mudaram
        bool agentsChanged = false;
        for (auto& intent : registeredIntents) {
            if (agentToRvoId.find(intent.agent) == agentToRvoId.end()) {
                agentsChanged = true;
                break;
            }
        }
        if (!needsRebuild && !agentsChanged) return;

        // Recria o simulador RVO2 (Inicialização conforme documentação)
        simulator = std::make_unique<RVO::RVOSimulator>();
        simulator->setTimeStep(timeStep);
        simulator->setAgentDefaults(
            neighborDist, maxNeighbors,
            timeHorizon, timeHorizonObst,
            agentRadius, maxSpeed
        );

        agentToRvoId.clear();

        // Registro: addAgent(position) para cada agente — retorna ID
        for (auto& intent : registeredIntents) {
            size_t rvoId = simulator->addAgent(
                RVO::Vector2(intent.position.x, intent.position.y));
            agentToRvoId[intent.agent] = rvoId;
            intent.rvoIndex = rvoId;
        }

        needsRebuild = false;
    }
};

// ============================================================================
// Strategy concreta: Comunicação Direta usando Mediator + RVO2
// ============================================================================
class RVO2CollisionAvoidance : public ICollisionAvoidance {
private:
    CollisionNegotiationMediator mediator;
    bool active;
    float timeStep;
    float agentRadius;
    float maxSpeed;

    std::vector<Vector2> lastCorrectedVelocities;
    std::vector<GameAgent*> lastAgents;

public:
    RVO2CollisionAvoidance() : active(true), timeStep(1.0f / 60.0f),
                                agentRadius(8.0f), maxSpeed(2.0f) {}

    std::string getName() const override {
        return "Comunicacao Direta";
    }

    void initialize(float ts, float radius, float speed) override {
        timeStep = ts;
        agentRadius = radius;
        maxSpeed = speed;
        mediator.configure(ts, radius, speed);

        std::cout << "[Metodo 1] Comunicacao Direta - Mediator + RVO2 (ORCA)" << std::endl;
        std::cout << "  Padrao: Strategy + Mediator" << std::endl;
        std::cout << "  Agentes negociam velocidades via mediador centralizado" << std::endl;
        std::cout << "  radius=" << radius << " maxSpeed=" << speed << std::endl;
    }

    // Sincroniza: cada agente se registra no mediador
    void syncAgents(const std::vector<GameAgent*>& agents) override {
        lastAgents.clear();
        for (auto* agent : agents) {
            if (!agent->isAlive()) continue;
            lastAgents.push_back(agent);
            mediator.registerAgent(agent, agent->getPosition());
        }
    }

    // Cada agente envia sua intenção de movimento ao mediador
    void setPreferredVelocities(
        const std::vector<GameAgent*>& agents,
        const std::vector<Vector2>& preferredVelocities) override {

        for (size_t i = 0; i < lastAgents.size() && i < preferredVelocities.size(); ++i) {
            mediator.sendMovementIntent(
                lastAgents[i],
                lastAgents[i]->getPosition(),
                preferredVelocities[i]
            );
        }
    }

    // O mediador negocia entre todos os agentes via RVO2
    void doStep() override {
        if (!active) return;

        auto results = mediator.negotiate();

        lastCorrectedVelocities.clear();
        lastCorrectedVelocities.resize(lastAgents.size(), {0.0f, 0.0f});

        for (auto& result : results) {
            for (size_t i = 0; i < lastAgents.size(); ++i) {
                if (lastAgents[i] == result.agent) {
                    lastCorrectedVelocities[i] = result.safeVelocity;
                    break;
                }
            }
        }
    }

    std::vector<Vector2> getCorrectedVelocities() override {
        return lastCorrectedVelocities;
    }

    bool isActive() const override { return active; }
    void setActive(bool a) override { active = a; }

    CollisionNegotiationMediator& getMediator() { return mediator; }
};

#endif // RVO2_COLLISION_AVOIDANCE_H
