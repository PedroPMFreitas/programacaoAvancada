#include "GridManager.h"
#include "Adapters/RectangularGridAdapter.h"
#include "Adapters/HexagonalGridAdapter.h"

GridManager* GridManager::instance = nullptr;

GridManager* GridManager::getInstance() {
    if (instance == nullptr) {
        instance = new GridManager();
    }
    return instance;
}

void GridManager::init(std::unique_ptr<IAppFactory> appFactory, GridType initialGridType, int width, int height) {
    this->appFactory = std::move(appFactory);
    switchGrid(initialGridType, width, height);
}

void GridManager::switchGrid(GridType type, int width, int height) {
    if (type == GridType::RECTANGULAR) {
        grid = std::make_unique<RectangularGridAdapter>(width, height);
    } else if (type == GridType::HEXAGONAL) {
        // The "size" of a hexagonal grid is not as straightforward as width/height.
        // For now, we'll use a simple conversion.
        int q_max = width / 2;
        int r_max = height / 2;
        grid = std::make_unique<HexagonalGridAdapter>(q_max, r_max);
    }
}

IGridAdapter* GridManager::getGrid() {
    return grid.get();
}

IAppFactory* GridManager::getAppFactory() {
    return appFactory.get();
}
