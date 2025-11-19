#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include "include/Factories/IAppFactory.h"
#include "include/Interfaces/IGrid.h"
#include "include/Interfaces/IAlgorithm.h"
#include "Trabalho5_Legacy.h"

class Application {
public:
    Application(std::unique_ptr<IAppFactory> appFactory);
    ~Application();

    void Run();

private:
    void Initialize();
    void HandleInput();
    void Update();
    void Render();

    const int screenWidth = 800;
    const int screenHeight = 600;
    const float cellSize = 20.0f;

    std::unique_ptr<IGrid> grid;
    std::unique_ptr<IAlgorithm> pathfinder;
    std::unique_ptr<AgentManager> agentManager;

    Vector2 spawnPos = {-1, -1};
    Vector2 targetPos = {-1, -1};
    bool placingSpawn = false;
    bool placingTarget = false;
};

#endif // APPLICATION_H
