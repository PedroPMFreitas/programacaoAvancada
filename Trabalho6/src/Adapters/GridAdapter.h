#ifndef GRID_ADAPTER_H
#define GRID_ADAPTER_H

#include "include/Interfaces/IGrid.h"
#include "include/Interfaces/IPathMovement.h"
#include "Trabalho5_Legacy.h"
#include <memory>

class GridAdapter : public IGrid {
private:
    std::unique_ptr<Grid> legacyGrid;
    std::unique_ptr<IPathMovement> movementStrategy;
public:
    GridAdapter(int width, int height, float cellSize) {
        legacyGrid = std::make_unique<Grid>(width, height, cellSize);
    }
    void Draw() override {
        legacyGrid->Draw();
    }
    void SetObstacle(int x, int y, bool isObstacle) override {
        legacyGrid->SetOccupied(x, y, isObstacle);
    }
    bool IsWalkable(int x, int y) const override {
        return legacyGrid->IsWalkable(x, y);
    }
    int GetWidth() const override {
        return legacyGrid->GetWidth();
    }
    int GetHeight() const override {
        return legacyGrid->GetHeight();
    }
    float GetCellSize() const override {
        return legacyGrid->GetCellSize();
    }
    Grid& GetLegacyGrid() override {
        return *legacyGrid;
    }
    void SetPathMovementStrategy(std::unique_ptr<IPathMovement> strategy) override {
        movementStrategy = std::move(strategy);
    }
    std::vector<Node*> GetNeighbors(Node* node) override {
        if (movementStrategy) {
            return movementStrategy->GetNeighbors(*legacyGrid, node);
        }
        return {};
    }
};

#endif // GRID_ADAPTER_H
