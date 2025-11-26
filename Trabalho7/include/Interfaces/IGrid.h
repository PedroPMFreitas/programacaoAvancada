#ifndef IGRID_H
#define IGRID_H

#include "../Trabalho6_Legacy.h"
#include <vector>
#include <memory>

class IPathMovement;

class IGrid {
public:
    virtual ~IGrid() = default;
    virtual void Draw() = 0;
    virtual void SetObstacle(int x, int y, bool isObstacle) = 0;
    virtual bool IsWalkable(int x, int y) const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual float GetCellSize() const = 0;
    virtual Grid& GetLegacyGrid() = 0;
    virtual void SetPathMovementStrategy(std::unique_ptr<IPathMovement> strategy) = 0;
    virtual std::vector<Node*> GetNeighbors(Node* node) = 0;
};

#endif // IGRID_H
