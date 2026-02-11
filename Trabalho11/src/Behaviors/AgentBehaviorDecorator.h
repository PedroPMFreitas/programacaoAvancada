#ifndef AGENT_BEHAVIOR_DECORATOR_H
#define AGENT_BEHAVIOR_DECORATOR_H

#include "src/Interfaces/IAgentBehavior.h"
#include "Core/Agent.h"
#include <memory>

class AgentBehaviorDecorator : public IAgentBehavior {
protected:
    std::unique_ptr<IAgentBehavior> wrappedBehavior;

public:
    AgentBehaviorDecorator(std::unique_ptr<IAgentBehavior> behavior)
        : wrappedBehavior(std::move(behavior)) {}

    void execute(Agent& agent) override {
        if (wrappedBehavior) {
            wrappedBehavior->execute(agent);
        }
    }
};

#endif // AGENT_BEHAVIOR_DECORATOR_H
