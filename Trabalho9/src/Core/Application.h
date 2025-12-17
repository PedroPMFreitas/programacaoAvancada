#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include "src/Interfaces/IAppFactory.h"
#include "src/Interfaces/IGrid.h"
#include "src/Interfaces/IGridAdapter.h"
#include "src/Interfaces/IAlgorithm.h"
#include "src/Interfaces/IInitHandler.h"
#include "Core/GridType.h"
#include "Trabalho9_Legacy.h"

// Forward declarations para os novos padrões
class GameAgentManager;
class CommandManager;

class Application {
public:
    Application(std::unique_ptr<IAppFactory> factory);
    ~Application();

    void Run();

private:
    bool InitializeWithChain();  // Chain of Responsibility para inicialização
    void HandleInput();
    void Update();
    void Render();
    void reinitializeGrid(GridType newGridType);
    void DrawUI();

    const int screenWidth = 800;
    const int screenHeight = 600;
    const float cellSize = 20.0f;

    std::unique_ptr<IAppFactory> factory;
    std::unique_ptr<IGrid> grid;
    IGridAdapter* gridAdapter;
    std::unique_ptr<IAlgorithm> pathfinder;
    
    // Usa o novo GameAgentManager com suporte a Observer
    std::unique_ptr<GameAgentManager> gameAgentManager;
    // Mantém o legacy para compatibilidade
    std::unique_ptr<AgentManager> legacyAgentManager;

    GridType currentGridType;
    Vector2 spawnPos = {-1, -1};
    Vector2 targetPos = {-1, -1};
    bool placingSpawn = false;
    bool placingTarget = false;
    
    // Flags para demonstração dos padrões
    bool useNewAgentSystem = true;  // Usa o novo sistema com Observer
    bool showStatistics = false;
};

#endif // APPLICATION_H
