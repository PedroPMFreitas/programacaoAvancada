#ifndef ORTHOGONAL_MOVEMENT_ADAPTER_H
#define ORTHOGONAL_MOVEMENT_ADAPTER_H

#include "include/Interfaces/IPathMovement.h"
#include "Trabalho5_Legacy.h"

class OrthogonalMovementAdapter : public IPathMovement {
public:
    std::vector<Node*> GetNeighbors(Grid& legacyGrid, Node* node) const override {
        // Since Pathfinder::GetNeighbors is private in the legacy code, 
        // the logic is re-implemented here to fulfill the adapter's contract
        // without modifying the original source file.
        std::vector<Node*> neighbors;
        int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
        for (auto& dir : directions) {
            int newX = node->x + dir[0];
            int newY = node->y + dir[1];
            if (legacyGrid.IsWalkable(newX, newY)) {
                Node* neighbor = legacyGrid.GetNode(newX, newY);
                if (neighbor) {
                    neighbors.push_back(neighbor);
                }
            }
        }
        return neighbors;
    }
};

#endif // ORTHOGONAL_MOVEMENT_ADAPTER_H
