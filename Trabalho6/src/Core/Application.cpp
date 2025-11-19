#include "include/Core/Application.h"

Application::Application(std::unique_ptr<IAppFactory> appFactory) {
    auto gridFactory = appFactory->CreateGridFactory();
    auto algorithmFactory = appFactory->CreateAlgorithmFactory();

    grid = gridFactory->CreateGrid(screenWidth / cellSize, screenHeight / cellSize, cellSize);
    pathfinder = algorithmFactory->CreateAlgorithm();
    
    agentManager = std::make_unique<AgentManager>(&grid->GetLegacyGrid());
}

Application::~Application() {
    CloseWindow();
}

void Application::Initialize() {
    InitWindow(screenWidth, screenHeight, "Trabalho 6 - Factory Pattern (No CMake)");
    SetTargetFPS(60);
}

void Application::Run() {
    Initialize();
    while (!WindowShouldClose()) {
        HandleInput();
        Update();
        Render();
    }
}

void Application::HandleInput() {
    Vector2 mousePos = GetMousePosition();
    int gridX = mousePos.x / cellSize;
    int gridY = mousePos.y / cellSize;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        grid->SetObstacle(gridX, gridY, true);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        grid->SetObstacle(gridX, gridY, false);
    }

    if (IsKeyPressed(KEY_S)) {
        placingSpawn = true;
        placingTarget = false;
    }
    if (IsKeyPressed(KEY_T)) {
        placingTarget = true;
        placingSpawn = false;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        placingSpawn = false;
        placingTarget = false;
        spawnPos = {-1, -1};
        targetPos = {-1, -1};
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (placingSpawn && grid->IsWalkable(gridX, gridY)) {
            spawnPos = {(float)gridX, (float)gridY};
            placingSpawn = false;
        } else if (placingTarget && grid->IsWalkable(gridX, gridY)) {
            targetPos = {(float)gridX, (float)gridY};
            placingTarget = false;
        }
    }

    if (IsKeyPressed(KEY_ENTER)) {
        if (spawnPos.x != -1 && targetPos.x != -1) {
            agentManager->AddAgent(spawnPos, targetPos);
            // spawnPos = {-1, -1}; // Keep markers on screen
            // targetPos = {-1, -1}; // Keep markers on screen
        }
    }
    
    if (IsKeyPressed(KEY_R)) {
        agentManager->AddRandomAgents(5);
    }

    if (IsKeyPressed(KEY_M)) {
        Metrics::SaveToCSV("manual_performance_data.csv");
        printf("Metrics saved to manual_performance_data.csv\n");
    }
    
    if (IsKeyPressed(KEY_P)) {
        Metrics::Clear();
        RunPerformanceTests();
    }
}

void Application::Update() {
    agentManager->UpdateAll(GetFrameTime());
}

void Application::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (grid) {
        grid->Draw();
        agentManager->DrawAll(grid->GetLegacyGrid());
    }

    if (spawnPos.x != -1) {
        DrawRectangleRec({spawnPos.x * cellSize, spawnPos.y * cellSize, cellSize, cellSize}, Fade(BLUE, 0.5f));
    }
    if (targetPos.x != -1) {
        DrawRectangleRec({targetPos.x * cellSize, targetPos.y * cellSize, cellSize, cellSize}, Fade(RED, 0.5f));
    }
    
    if (placingSpawn) {
        DrawText("MODO: Posicionando GERAÇÃO (Clique dir)", 10, 10, 20, DARKBLUE);
    } else if (placingTarget) {
        DrawText("MODO: Posicionando ALVO (Clique dir)", 10, 10, 20, MAROON);
    } else {
        DrawText("S: Geração | T: Alvo | ENTER: Criar Agente", 10, 10, 20, DARKGRAY);
    }
    DrawText("R: 5 Agentes Aleatórios", 10, 35, 20, DARKGRAY);
    DrawText("L-Click: Obstáculo | R-Click: Remover", 10, 60, 20, DARKGRAY);
    DrawText(TextFormat("Agentes: %d", agentManager->GetAgentCount()), 10, 85, 20, DARKGRAY);
    DrawText("P: Teste de Performance | M: Salvar Métricas", 10, 110, 20, DARKGRAY);

    EndDrawing();
}
