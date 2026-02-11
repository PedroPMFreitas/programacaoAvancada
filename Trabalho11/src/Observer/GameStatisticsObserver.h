#ifndef GAME_STATISTICS_OBSERVER_H
#define GAME_STATISTICS_OBSERVER_H

#include "src/Interfaces/IObserver.h"
#include "GameAgent.h"
#include <iostream>

// Observer que mantém estatísticas do jogo
class GameStatisticsObserver : public IObserver {
private:
    int totalDeaths;
    int totalRespawns;
    int totalTargetsReached;
    int totalPathsBlocked;

public:
    GameStatisticsObserver() 
        : totalDeaths(0), totalRespawns(0), 
          totalTargetsReached(0), totalPathsBlocked(0) {}
    
    void onNotify(const std::string& event, void* data) override {
        if (event == AgentEvents::AGENT_DIED) {
            totalDeaths++;
        } else if (event == AgentEvents::AGENT_SPAWNED) {
            totalRespawns++;
        } else if (event == AgentEvents::AGENT_REACHED_TARGET) {
            totalTargetsReached++;
        } else if (event == AgentEvents::AGENT_PATH_BLOCKED) {
            totalPathsBlocked++;
        }
    }
    
    // Getters
    int getTotalDeaths() const { return totalDeaths; }
    int getTotalRespawns() const { return totalRespawns; }
    int getTotalTargetsReached() const { return totalTargetsReached; }
    int getTotalPathsBlocked() const { return totalPathsBlocked; }
    
    void reset() {
        totalDeaths = 0;
        totalRespawns = 0;
        totalTargetsReached = 0;
        totalPathsBlocked = 0;
    }
    
    void printStatistics() const {
        std::cout << "\n=== Estatísticas do Jogo ===" << std::endl;
        std::cout << "Mortes totais: " << totalDeaths << std::endl;
        std::cout << "Respawns totais: " << totalRespawns << std::endl;
        std::cout << "Alvos alcançados: " << totalTargetsReached << std::endl;
        std::cout << "Caminhos bloqueados: " << totalPathsBlocked << std::endl;
        std::cout << "============================\n" << std::endl;
    }
};

#endif // GAME_STATISTICS_OBSERVER_H
