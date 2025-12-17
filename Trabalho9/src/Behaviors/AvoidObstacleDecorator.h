#ifndef AVOID_OBSTACLE_DECORATOR_H
#define AVOID_OBSTACLE_DECORATOR_H

#include "AgentBehaviorDecorator.h"
#include "Core/Agent.h"
#include "src/GridManager.h"
#include "src/Adapters/RectangularGridAdapter.h"
#include <iostream>

class AvoidObstacleDecorator : public AgentBehaviorDecorator {
public:
    AvoidObstacleDecorator(std::unique_ptr<IAgentBehavior> behavior)
        : AgentBehaviorDecorator(std::move(behavior)) {}

    void execute(Agent& agent) override {
        // Check for obstacles before executing the wrapped behavior
        IGridAdapter* grid = GridManager::getInstance()->getGrid();
        
        // Check if next cell has an obstacle (simplified check)
        if (grid && !grid->isValidCoordinate(agent.position.x + 1, agent.position.y)) {
            std::cout << "Agent at (" << agent.position.x << ", " << agent.position.y << ") is avoiding obstacle." << std::endl;
            // Implement obstacle avoidance logic
        } else {
            AgentBehaviorDecorator::execute(agent);
        }
    }
};

#endif // AVOID_OBSTACLE_DECORATOR_H
