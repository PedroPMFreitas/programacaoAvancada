#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include "Factories/IAppFactory.h"
#include "Interfaces/IGrid.h"
#include "Interfaces/IGridAdapter.h"
#include "Interfaces/IAlgorithm.h"
#include "Core/GridType.h"
#include "Trabalho6_Legacy.h"

class Application {
public:
    Application(std::unique_ptr<IAppFactory> factory);
    ~Application();

    void Run();

private:
    void Initialize();
    void HandleInput();
    void Update();
    void Render();
    void reinitializeGrid(GridType newGridType);

    const int screenWidth = 800;
    const int screenHeight = 600;
    const float cellSize = 20.0f;

    std::unique_ptr<IGrid> grid;
    IGridAdapter* gridAdapter;
    std::unique_ptr<IAlgorithm> pathfinder;
    std::unique_ptr<AgentManager> agentManager;

    GridType currentGridType;
    Vector2 spawnPos = {-1, -1};
    Vector2 targetPos = {-1, -1};
    bool placingSpawn = false;
    bool placingTarget = false;
};

#endif // APPLICATION_H
