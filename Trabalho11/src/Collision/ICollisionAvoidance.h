#ifndef ICOLLISION_AVOIDANCE_H
#define ICOLLISION_AVOIDANCE_H

#include "src/Observer/GameAgent.h"
#include <vector>
#include <string>

// Interface Strategy para métodos de evasão de colisão
// Padrão de Projeto: Strategy
// Permite trocar o algoritmo de evasão de colisão em tempo de execução
class ICollisionAvoidance {
public:
    virtual ~ICollisionAvoidance() = default;

    // Nome do método para exibição na UI
    virtual std::string getName() const = 0;

    // Inicializa o sistema com os parâmetros do cenário
    virtual void initialize(float timeStep, float agentRadius, float maxSpeed) = 0;

    // Sincroniza os agentes do jogo com o sistema de evasão
    virtual void syncAgents(const std::vector<GameAgent*>& agents) = 0;

    // Define a velocidade desejada (sem evasão) de cada agente
    virtual void setPreferredVelocities(
        const std::vector<GameAgent*>& agents,
        const std::vector<Vector2>& preferredVelocities) = 0;

    // Executa um passo da simulação de evasão
    virtual void doStep() = 0;

    // Obtém a velocidade corrigida (com evasão) para cada agente
    virtual std::vector<Vector2> getCorrectedVelocities() = 0;

    // Verifica se o sistema está ativo
    virtual bool isActive() const = 0;

    // Liga/desliga o sistema
    virtual void setActive(bool active) = 0;
};

#endif // ICOLLISION_AVOIDANCE_H
