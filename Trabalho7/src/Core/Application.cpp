#include "include/Core/Application.h"
#include "src/GridManager.h"
#include <iostream>

Application::Application(std::unique_ptr<IAppFactory> factory) {
    GridManager::getInstance()->init(std::move(factory), GridType::RECTANGULAR, 800 / 20, 600 / 20);
    gridAdapter = GridManager::getInstance()->getGrid();
    currentGridType = GridType::RECTANGULAR;

    auto algorithmFactory = GridManager::getInstance()->getAppFactory()->CreateAlgorithmFactory();
    pathfinder = algorithmFactory->CreateAlgorithm();
    agentManager = std::make_unique<AgentManager>(&gridAdapter->GetLegacyGrid());
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

void Application::reinitializeGrid(GridType newGridType) {
    if (currentGridType == newGridType) return;

    currentGridType = newGridType;
    GridManager::getInstance()->switchGrid(newGridType, screenWidth / cellSize, screenHeight / cellSize);
    gridAdapter = GridManager::getInstance()->getGrid();
    
    spawnPos = {-1, -1};
    targetPos = {-1, -1};
    
    agentManager = std::make_unique<AgentManager>(&gridAdapter->GetLegacyGrid());
}

void Application::HandleInput() {
    // Grid type switching
    if (IsKeyPressed(KEY_H)) {
        reinitializeGrid(GridType::HEXAGONAL);
    }
    if (IsKeyPressed(KEY_J)) {
        reinitializeGrid(GridType::RECTANGULAR);
    }

    // Input handling for obstacle and target placement
    if (currentGridType == GridType::RECTANGULAR) {
        Vector2 mousePos = GetMousePosition();
        int gridX = mousePos.x / cellSize;
        int gridY = mousePos.y / cellSize;

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            gridAdapter->SetObstacle(gridX, gridY, true);
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            gridAdapter->SetObstacle(gridX, gridY, false);
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
            if (placingSpawn && gridAdapter->IsWalkable(gridX, gridY)) {
                spawnPos = {(float)gridX, (float)gridY};
                placingSpawn = false;
            } else if (placingTarget && gridAdapter->IsWalkable(gridX, gridY)) {
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
    } else if (currentGridType == GridType::HEXAGONAL) {
        // Placeholder: Hexagonal grid input handling is more complex.
        // For now, obstacle and target placement is disabled.
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            if (placingSpawn) {
                // Placeholder for hexagonal coordinate conversion
                std::cout << "Hexagonal spawn placement not implemented yet." << std::endl;
            } else if (placingTarget) {
                // Placeholder for hexagonal coordinate conversion
                std::cout << "Hexagonal target placement not implemented yet." << std::endl;
            }
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

    if (gridAdapter) {
        gridAdapter->Draw();
        agentManager->DrawAll(gridAdapter->GetLegacyGrid());
    }

    if (spawnPos.x != -1) {
        DrawRectangleRec({spawnPos.x * cellSize, spawnPos.y * cellSize, cellSize, cellSize}, Fade(BLUE, 0.5f));
    }
    if (targetPos.x != -1) {
        DrawRectangleRec({targetPos.x * cellSize, targetPos.y * cellSize, cellSize, cellSize}, Fade(RED, 0.5f));
    }
    
    if (placingSpawn) {
        DrawText("MODO: Posicionando GERAÇÃO (Clique dir)", 10, 10, 20, GREEN);
    } else if (placingTarget) {
        DrawText("MODO: Posicionando ALVO (Clique dir)", 10, 10, 20, GREEN);
    } else {
        DrawText("S: Geração | T: Alvo | ENTER: Criar Agente", 10, 10, 20, GREEN);
    }
    DrawText("R: 5 Agentes Aleatórios", 10, 35, 20, GREEN);
    DrawText("L-Click: Obstáculo | R-Click: Remover", 10, 60, 20, GREEN);
    DrawText(TextFormat("Agentes: %d", agentManager->GetAgentCount()), 10, 85, 20, GREEN);
    DrawText("P: Teste de Performance | M: Salvar Métricas", 10, 110, 20, GREEN);
    DrawText(TextFormat("Grid: %s (H/J para trocar)", currentGridType == GridType::RECTANGULAR ? "Retangular" : "Hexagonal"), 10, 135, 20, GREEN);

    EndDrawing();
}
