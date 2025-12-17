#include "GridManager.h"
#include "Adapters/RectangularGridAdapter.h"
#include "Adapters/HexagonalGridAdapter.h"
#include <cmath>

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
        // Calcula dimensões para preencher a tela
        // Para hexágonos pointy-top com raio 12:
        // horizSpacing = sqrt(3) * 12 ≈ 20.78
        // vertSpacing = 12 * 2 * 0.75 = 18
        float hexRadius = 12.0f;
        float horizSpacing = std::sqrt(3.0f) * hexRadius;
        float vertSpacing = hexRadius * 2.0f * 0.75f;
        
        // screenWidth e screenHeight são passados como width/cellSize e height/cellSize
        // então precisamos recalcular para pixels (800x600)
        int screenWidth = 800;
        int screenHeight = 600;
        
        int cols = (int)(screenWidth / horizSpacing) + 1;
        int rows = (int)(screenHeight / vertSpacing) + 1;
        
        grid = std::make_unique<HexagonalGridAdapter>(cols, rows);
    }
}

IGridAdapter* GridManager::getGrid() {
    return grid.get();
}

IAppFactory* GridManager::getAppFactory() {
    return appFactory.get();
}
