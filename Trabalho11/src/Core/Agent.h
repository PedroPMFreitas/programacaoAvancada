#ifndef AGENT_H
#define AGENT_H

#include "Core/Cell.h"

// Forward declaration
class IGridAdapter;

class Agent {
public:
    Cell position;
    // Other agent properties like speed, etc.
    // For now, keep it simple.
};

#endif // AGENT_H