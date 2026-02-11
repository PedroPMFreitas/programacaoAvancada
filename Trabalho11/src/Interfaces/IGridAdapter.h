#ifndef IGRID_ADAPTER_H
#define IGRID_ADAPTER_H

#include "IGrid.h"
#include "../Core/Cell.h"
#include "../Core/GridType.h"
#include <vector>
#include <list>

class IGridAdapter : public IGrid {
public:
    virtual ~IGridAdapter() = default;

    // Methods from IGrid
    virtual void Draw() override = 0;
    virtual void SetObstacle(int x, int y, bool isObstacle) override = 0;
    virtual bool IsWalkable(int x, int y) const override = 0;
    virtual int GetWidth() const override = 0;
    virtual int GetHeight() const override = 0;
    virtual float GetCellSize() const override = 0;
    virtual Grid& GetLegacyGrid() override = 0;
    virtual void SetPathMovementStrategy(std::unique_ptr<IPathMovement> strategy) override = 0;
    virtual std::vector<Node*> GetNeighbors(Node* node) override = 0;

    // Methods from the original IGridAdapter (now part of the unified interface)
    virtual std::list<Cell> getNeighbors(Cell cell) const = 0;
    virtual bool isValidCoordinate(int x, int y) const = 0;
    virtual Cell getCell(int x, int y) const = 0;
};

#endif // IGRID_ADAPTER_H
