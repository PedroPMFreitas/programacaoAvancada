#ifndef IDLE_BEHAVIOR_H
#define IDLE_BEHAVIOR_H

#include "Interfaces/IAgentBehavior.h"
#include "Core/Agent.h"
#include <iostream>

class IdleBehavior : public IAgentBehavior {
public:
    void execute(Agent& agent) override {
        std::cout << "Agent at (" << agent.position.x << ", " << agent.position.y << ") is idle." << std::endl;
        // Idle behavior: agent does nothing
    }
};

#endif // IDLE_BEHAVIOR_H
