#ifndef CONCRETE_GRID_FACTORY_H
#define CONCRETE_GRID_FACTORY_H

#include "src/Interfaces/IGridFactory.h"
#include "Adapters/GridAdapter.h"
#include "Adapters/OrthogonalMovementAdapter.h"

class StandardGridFactory : public IGridFactory {
public:
    std::unique_ptr<IGrid> CreateGrid(int width, int height, float cellSize) override {
        auto grid = std::make_unique<GridAdapter>(width, height, cellSize);
        grid->SetPathMovementStrategy(CreatePathMovement());
        return grid;
    }

    std::unique_ptr<IPathMovement> CreatePathMovement() override {
        return std::make_unique<OrthogonalMovementAdapter>();
    }
};

#endif // CONCRETE_GRID_FACTORY_H
