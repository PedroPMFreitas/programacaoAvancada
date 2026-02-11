#ifndef PATHFINDER_INIT_HANDLER_H
#define PATHFINDER_INIT_HANDLER_H

#include "BaseInitHandler.h"
#include "src/GridManager.h"
#include "src/Interfaces/IAlgorithm.h"
#include <memory>

// Handler para inicialização do pathfinder
class PathfinderInitHandler : public BaseInitHandler {
private:
    std::unique_ptr<IAlgorithm>& pathfinder;

public:
    PathfinderInitHandler(std::unique_ptr<IAlgorithm>& pf) : pathfinder(pf) {}
    
    std::string getName() const override {
        return "PathfinderInitHandler";
    }

protected:
    bool doHandle() override {
        try {
            auto algorithmFactory = GridManager::getInstance()->getAppFactory()->CreateAlgorithmFactory();
            pathfinder = algorithmFactory->CreateAlgorithm();
            return pathfinder != nullptr;
        } catch (...) {
            return false;
        }
    }
};

#endif // PATHFINDER_INIT_HANDLER_H
