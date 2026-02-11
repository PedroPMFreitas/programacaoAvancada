#ifndef AGENT_MANAGER_INIT_HANDLER_H
#define AGENT_MANAGER_INIT_HANDLER_H

#include "BaseInitHandler.h"
#include "src/GridManager.h"
#include "Trabalho9_Legacy.h"
#include <memory>

// Handler para inicialização do AgentManager
class AgentManagerInitHandler : public BaseInitHandler {
private:
    std::unique_ptr<AgentManager>& agentManager;

public:
    AgentManagerInitHandler(std::unique_ptr<AgentManager>& am) : agentManager(am) {}
    
    std::string getName() const override {
        return "AgentManagerInitHandler";
    }

protected:
    bool doHandle() override {
        try {
            auto gridAdapter = GridManager::getInstance()->getGrid();
            if (gridAdapter) {
                agentManager = std::make_unique<AgentManager>(&gridAdapter->GetLegacyGrid());
                return agentManager != nullptr;
            }
            return false;
        } catch (...) {
            return false;
        }
    }
};

#endif // AGENT_MANAGER_INIT_HANDLER_H
