#ifndef AGENT_RESPAWN_OBSERVER_H
#define AGENT_RESPAWN_OBSERVER_H

#include "src/Interfaces/IObserver.h"
#include "GameAgent.h"
#include <iostream>

// Observer que faz o agente renascer quando morre
class AgentRespawnObserver : public IObserver {
private:
    float respawnDelay; // segundos até respawn
    
public:
    AgentRespawnObserver(float delay = 3.0f) : respawnDelay(delay) {}
    
    void onNotify(const std::string& event, void* data) override {
        if (event == AgentEvents::AGENT_DIED) {
            GameAgent* agent = static_cast<GameAgent*>(data);
            if (agent) {
                std::cout << "[Observer] Agente morreu! Respawnando em " 
                          << respawnDelay << " segundos..." << std::endl;
                // Em uma implementação real, usaríamos um timer/scheduler
                // Por simplicidade, respawnamos imediatamente
                agent->respawn();
                std::cout << "[Observer] Agente respawnado no ponto de origem!" << std::endl;
            }
        }
    }
    
    void setRespawnDelay(float delay) { respawnDelay = delay; }
    float getRespawnDelay() const { return respawnDelay; }
};

#endif // AGENT_RESPAWN_OBSERVER_H
