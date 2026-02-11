#ifndef PATHFINDING_DECORATOR_H
#define PATHFINDING_DECORATOR_H

#include "AgentBehaviorDecorator.h"
#include "Core/Agent.h"
#include "Core/Cell.h"
#include "src/GridManager.h"
#include <list>
#include <iostream>

class PathfindingDecorator : public AgentBehaviorDecorator {
private:
    std::list<Cell> path;
    Cell target;

public:
    PathfindingDecorator(std::unique_ptr<IAgentBehavior> behavior, Cell target)
        : AgentBehaviorDecorator(std::move(behavior)), target(target) {}

    void execute(Agent& agent) override {
        // In a real scenario, this would involve running a pathfinding algorithm
        if (path.empty() || path.back() != target) {
            std::cout << "Agent at (" << agent.position.x << ", " << agent.position.y << ") is calculating path to (" << target.x << ", " << target.y << ")" << std::endl;
            path.clear();
            path.push_back({agent.position.x + 1, agent.position.y});
            path.push_back(target);
        }

        if (!path.empty()) {
            Cell nextStep = path.front();
            if (agent.position == nextStep) {
                path.pop_front();
                if (!path.empty()) {
                    nextStep = path.front();
                } else {
                    std::cout << "Agent reached target!" << std::endl;
                    AgentBehaviorDecorator::execute(agent);
                    return;
                }
            }
            
            std::cout << "Agent moving to (" << nextStep.x << ", " << nextStep.y << ") as part of pathfinding." << std::endl;
            agent.position = nextStep;
        }

        AgentBehaviorDecorator::execute(agent);
    }
};

#endif // PATHFINDING_DECORATOR_H
