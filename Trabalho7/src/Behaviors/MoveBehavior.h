#ifndef MOVE_BEHAVIOR_H
#define MOVE_BEHAVIOR_H

#include "Interfaces/IAgentBehavior.h"
#include "Core/Agent.h"
#include <iostream>

class MoveBehavior : public IAgentBehavior {
public:
    void execute(Agent& agent) override {
        // Simple move behavior: move agent one step in a predefined direction (e.g., right)
        // In a real scenario, this would involve more complex logic, e.g., pathfinding, target.
        std::cout << "Agent at (" << agent.position.x << ", " << agent.position.y << ") is moving." << std::endl;
        agent.position.x++; // Example: move right
    }
};

#endif // MOVE_BEHAVIOR_H
