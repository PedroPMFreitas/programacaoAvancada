#ifndef IAGENT_BEHAVIOR_H
#define IAGENT_BEHAVIOR_H

#include "Core/Agent.h"

class IAgentBehavior {
public:
    virtual ~IAgentBehavior() = default;
    virtual void execute(Agent& agent) = 0;
};

#endif // IAGENT_BEHAVIOR_H