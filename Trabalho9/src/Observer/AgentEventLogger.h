#ifndef AGENT_EVENT_LOGGER_H
#define AGENT_EVENT_LOGGER_H

#include "src/Interfaces/IObserver.h"
#include "GameAgent.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>

// Observer que registra eventos dos agentes
class AgentEventLogger : public IObserver {
private:
    bool logToConsole;
    bool logToFile;
    std::string logFilename;
    
public:
    AgentEventLogger(bool console = false, bool file = false, 
                     const std::string& filename = "agent_events.log") 
        : logToConsole(console), logToFile(file), logFilename(filename) {}
    
    void setLogToConsole(bool log) { logToConsole = log; }
    
    void onNotify(const std::string& event, void* data) override {
        std::string message = formatMessage(event, data);
        
        if (logToConsole) {
            std::cout << message << std::endl;
        }
        
        if (logToFile) {
            std::ofstream file(logFilename, std::ios::app);
            if (file.is_open()) {
                file << message << std::endl;
                file.close();
            }
        }
    }

private:
    std::string formatMessage(const std::string& event, void* data) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&time), "%H:%M:%S") << "] ";
        
        if (event == AgentEvents::AGENT_SPAWNED) {
            GameAgent* agent = static_cast<GameAgent*>(data);
            if (agent) {
                ss << "SPAWN: Agente spawnou em (" 
                   << agent->getPosition().x << ", " << agent->getPosition().y << ")";
            }
        } else if (event == AgentEvents::AGENT_DIED) {
            GameAgent* agent = static_cast<GameAgent*>(data);
            if (agent) {
                ss << "MORTE: Agente morreu em (" 
                   << agent->getPosition().x << ", " << agent->getPosition().y << ")";
            }
        } else if (event == AgentEvents::AGENT_REACHED_TARGET) {
            GameAgent* agent = static_cast<GameAgent*>(data);
            if (agent) {
                ss << "ALVO: Agente chegou ao destino (" 
                   << agent->getTarget().x << ", " << agent->getTarget().y << ")";
            }
        }else if (event == AgentEvents::AGENT_PATH_BLOCKED) {
            ss << "BLOQUEADO: Caminho do agente bloqueado";
        } else if (event == AgentEvents::AGENT_HEALTH_CHANGED) {
            ss << "VIDA: Vida do agente alterada";
        } else {
            ss << "EVENTO: " << event;
        }
        
        return ss.str();
    }
};

#endif // AGENT_EVENT_LOGGER_H
