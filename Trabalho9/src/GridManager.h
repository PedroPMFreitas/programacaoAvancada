#ifndef GRID_MANAGER_H
#define GRID_MANAGER_H

#include "../src/Interfaces/IGridAdapter.h"
#include "../src/Interfaces/IAppFactory.h"
#include "../src/Core/GridType.h"
#include <memory>

class GridManager {
private:
    static GridManager* instance;
    std::unique_ptr<IGridAdapter> grid;
    std::unique_ptr<IAppFactory> appFactory; // Store the app factory

    GridManager() = default; // Private constructor

public:
    GridManager(const GridManager&) = delete; // Delete copy constructor
    GridManager& operator=(const GridManager&) = delete; // Delete assignment operator

    static GridManager* getInstance();
    void init(std::unique_ptr<IAppFactory> appFactory, GridType initialGridType, int width, int height);
    void switchGrid(GridType type, int width, int height);
    IGridAdapter* getGrid();
    IAppFactory* getAppFactory(); // New method to get the app factory
};

#endif // GRID_MANAGER_H
