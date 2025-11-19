#ifndef IPATH_MOVEMENT_H
#define IPATH_MOVEMENT_H

#include "Trabalho5_Legacy.h"
#include <vector>

class IPathMovement {
public:
    virtual ~IPathMovement() = default;
    virtual std::vector<Node*> GetNeighbors(Grid& legacyGrid, Node* node) const = 0;
};

#endif // IPATH_MOVEMENT_H
