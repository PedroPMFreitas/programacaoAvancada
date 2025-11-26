#ifndef SPEED_BOOST_DECORATOR_H
#define SPEED_BOOST_DECORATOR_H

#include "AgentBehaviorDecorator.h"
#include "Core/Agent.h"
#include <iostream>

class SpeedBoostDecorator : public AgentBehaviorDecorator {
public:
    SpeedBoostDecorator(std::unique_ptr<IAgentBehavior> behavior)
        : AgentBehaviorDecorator(std::move(behavior)) {}

    void execute(Agent& agent) override {
        std::cout << "Applying speed boost to agent at (" << agent.position.x << ", " << agent.position.y << ")" << std::endl;
        // In a real scenario, this would modify agent's speed or apply a temporary effect
        AgentBehaviorDecorator::execute(agent); // Call the wrapped behavior
        std::cout << "Speed boost applied." << std::endl;
    }
};

#endif // SPEED_BOOST_DECORATOR_H
