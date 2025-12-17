#include "src/Core/Application.h"
#include "src/GridManager.h"
#include "src/ChainOfResponsibility/BaseInitHandler.h"
#include "src/ChainOfResponsibility/WindowInitHandler.h"
#include "src/ChainOfResponsibility/GridInitHandler.h"
#include "src/ChainOfResponsibility/PathfinderInitHandler.h"
#include "src/ChainOfResponsibility/AgentManagerInitHandler.h"
#include "src/Commands/CommandManager.h"
#include "src/Commands/SetObstacleCommand.h"
#include "src/Commands/SpawnAgentCommand.h"
#include "src/Observer/GameAgentManager.h"
#include "src/Adapters/HexagonalGridAdapter.h"
#include <iostream>

Application::Application(std::unique_ptr<IAppFactory> f) : factory(std::move(f)) {
    // Inicialização será feita via Chain of Responsibility
    gridAdapter = nullptr;
    currentGridType = GridType::RECTANGULAR;
}

Application::~Application() {
    CloseWindow();
}

// Chain of Responsibility para inicialização do sistema
bool Application::InitializeWithChain() {
    std::cout << "\n=== Iniciando sistema via Chain of Responsibility ===" << std::endl;
    
    // Cria a cadeia de handlers de inicialização
    auto windowHandler = std::make_shared<WindowInitHandler>(
        screenWidth, screenHeight, "Trabalho 9 - Design Patterns (Chain, Command, Observer)");
    
    auto gridHandler = std::make_shared<GridInitHandler>(
        std::move(factory), GridType::RECTANGULAR, 
        screenWidth / cellSize, screenHeight / cellSize);
    
    auto pathfinderHandler = std::make_shared<PathfinderInitHandler>(pathfinder);
    auto agentManagerHandler = std::make_shared<AgentManagerInitHandler>(legacyAgentManager);
    
    // Configura a cadeia
    windowHandler->setNext(gridHandler);
    gridHandler->setNext(pathfinderHandler);
    pathfinderHandler->setNext(agentManagerHandler);
    
    // Executa a cadeia
    bool success = windowHandler->handle();
    
    if (success) {
        gridAdapter = GridManager::getInstance()->getGrid();
        
        // Inicializa o novo sistema de agentes com Observer (suporta hex e retangular)
        if (gridAdapter) {
            gameAgentManager = std::make_unique<GameAgentManager>(gridAdapter, currentGridType);
        }
        
        std::cout << "=== Sistema inicializado com sucesso! ===\n" << std::endl;
    } else {
        std::cout << "=== FALHA na inicialização do sistema! ===\n" << std::endl;
    }
    
    return success;
}

void Application::Run() {
    if (!InitializeWithChain()) {
        std::cerr << "Erro crítico na inicialização!" << std::endl;
        return;
    }
    
    while (!WindowShouldClose()) {
        // Processa comandos agendados
        CommandManager::getInstance()->processPendingCommands();
        
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
    
    legacyAgentManager = std::make_unique<AgentManager>(&gridAdapter->GetLegacyGrid());
    gameAgentManager = std::make_unique<GameAgentManager>(gridAdapter, currentGridType);
    
    // Limpa histórico de comandos ao trocar de grid
    CommandManager::getInstance()->clearHistory();
}

void Application::HandleInput() {
    // Grid type switching
    if (IsKeyPressed(KEY_H)) {
        reinitializeGrid(GridType::HEXAGONAL);
    }
    if (IsKeyPressed(KEY_J)) {
        reinitializeGrid(GridType::RECTANGULAR);
    }
    
    // Toggle entre sistema novo e legado
    if (IsKeyPressed(KEY_N)) {
        useNewAgentSystem = !useNewAgentSystem;
        std::cout << "Sistema de agentes: " 
                  << (useNewAgentSystem ? "NOVO (Observer)" : "LEGADO") << std::endl;
    }
    
    // Toggle estatísticas
    if (IsKeyPressed(KEY_I)) {
        showStatistics = !showStatistics;
        if (showStatistics && gameAgentManager) {
            gameAgentManager->printStatistics();
        }
    }
    
    // Undo/Redo com Command Pattern
    if (IsKeyPressed(KEY_Z) && IsKeyDown(KEY_LEFT_CONTROL)) {
        if (CommandManager::getInstance()->undo()) {
            std::cout << "[Command] Undo executado!" << std::endl;
        }
    }
    if (IsKeyPressed(KEY_Y) && IsKeyDown(KEY_LEFT_CONTROL)) {
        if (CommandManager::getInstance()->redo()) {
            std::cout << "[Command] Redo executado!" << std::endl;
        }
    }

    // Input handling for obstacle and target placement
    Vector2 mousePos = GetMousePosition();
    int gridX, gridY;
    
    if (currentGridType == GridType::RECTANGULAR) {
        gridX = mousePos.x / cellSize;
        gridY = mousePos.y / cellSize;
    } else {
        // Grid hexagonal - usa o método do adapter
        auto* hexAdapter = dynamic_cast<HexagonalGridAdapter*>(gridAdapter);
        if (hexAdapter) {
            Cell hexCell = hexAdapter->getClickedCell(mousePos);
            gridX = hexCell.x;
            gridY = hexCell.y;
        } else {
            gridX = 0;
            gridY = 0;
        }
    }

    // Usa Command Pattern para obstáculos (funciona para ambos os grids)
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        auto cmd = std::make_shared<SetObstacleCommand>(gridAdapter, gridX, gridY, true);
        CommandManager::getInstance()->executeCommand(cmd);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !placingSpawn && !placingTarget) {
        auto cmd = std::make_shared<SetObstacleCommand>(gridAdapter, gridX, gridY, false);
        CommandManager::getInstance()->executeCommand(cmd);
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
            if (useNewAgentSystem && gameAgentManager) {
                // Usa Command Pattern para criar agente
                auto cmd = std::make_shared<SpawnAgentCommand>(
                    gameAgentManager.get(), spawnPos, targetPos);
                CommandManager::getInstance()->executeCommand(cmd);
            } else if (legacyAgentManager) {
                legacyAgentManager->AddAgent(spawnPos, targetPos);
            }
        }
    }
    
    if (IsKeyPressed(KEY_R)) {
        if (useNewAgentSystem && gameAgentManager) {
            gameAgentManager->addRandomAgents(5);
        } else if (legacyAgentManager) {
            legacyAgentManager->AddRandomAgents(5);
        }
    }
    
    // Toggle sistema de colisões
    if (IsKeyPressed(KEY_C)) {
        if (useNewAgentSystem && gameAgentManager) {
            gameAgentManager->toggleCollisionVisualization();
        }
    }
    
    // Testa o sistema Observer - causa dano aos agentes
    if (IsKeyPressed(KEY_D)) {
        if (useNewAgentSystem && gameAgentManager) {
            std::cout << "[Teste Observer] Causando 50 de dano a todos os agentes..." << std::endl;
            gameAgentManager->damageAllAgents(50);
        }
    }
    
    // Mata todos os agentes (teste de respawn via Observer)
    if (IsKeyPressed(KEY_K)) {
        if (useNewAgentSystem && gameAgentManager) {
            std::cout << "[Teste Observer] Matando todos os agentes..." << std::endl;
            gameAgentManager->damageAllAgents(1000);
        }
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
    if (useNewAgentSystem && gameAgentManager) {
        gameAgentManager->updateAll(GetFrameTime());
    } else if (legacyAgentManager) {
        legacyAgentManager->UpdateAll(GetFrameTime());
    }
}

void Application::DrawUI() {
    int y = 10;
    int lineHeight = 22;
    
    if (placingSpawn) {
        DrawText("MODO: Posicionando GERAÇÃO (Clique dir)", 10, y, 18, GREEN);
    } else if (placingTarget) {
        DrawText("MODO: Posicionando ALVO (Clique dir)", 10, y, 18, GREEN);
    } else {
        DrawText("S: Geração | T: Alvo | ENTER: Criar Agente", 10, y, 18, GREEN);
    }
    y += lineHeight;
    
    DrawText("R: 5 Agentes Aleatórios | L-Click: Obstáculo", 10, y, 18, GREEN);
    y += lineHeight;
    
    int agentCount = useNewAgentSystem ? 
        (gameAgentManager ? gameAgentManager->getAgentCount() : 0) :
        (legacyAgentManager ? legacyAgentManager->GetAgentCount() : 0);
    DrawText(TextFormat("Agentes: %d", agentCount), 10, y, 18, GREEN);
    y += lineHeight;
    
    DrawText(TextFormat("Grid: %s (H/J para trocar)", 
        currentGridType == GridType::RECTANGULAR ? "Retangular" : "Hexagonal"), 10, y, 18, GREEN);
    y += lineHeight;
    
    // Informações dos padrões de projeto
    DrawText("--- Padrões de Projeto ---", 10, y, 18, BLUE);
    y += lineHeight;
    
    DrawText(TextFormat("Sistema: %s (N: toggle)", 
        useNewAgentSystem ? "Observer" : "Legado"), 10, y, 18, BLUE);
    y += lineHeight;
    
    DrawText(TextFormat("Histórico Commands: %d (Ctrl+Z/Y)", 
        (int)CommandManager::getInstance()->getHistorySize()), 10, y, 18, BLUE);
    y += lineHeight;
    
    DrawText("D: Dano aos agentes | K: Matar todos", 10, y, 18, ORANGE);
    y += lineHeight;
    
    DrawText("C: Toggle Colisões | I: Estatísticas", 10, y, 18, ORANGE);
    y += lineHeight;
    
    // Mostra estatísticas se ativado
    if (showStatistics && gameAgentManager) {
        auto stats = gameAgentManager->getStatsObserver();
        y += 10;
        DrawRectangle(5, y - 5, 250, 100, Fade(BLACK, 0.7f));
        DrawText("=== Estatísticas ===", 10, y, 18, WHITE);
        y += lineHeight;
        DrawText(TextFormat("Mortes: %d", stats->getTotalDeaths()), 10, y, 16, WHITE);
        y += lineHeight - 4;
        DrawText(TextFormat("Respawns: %d", stats->getTotalRespawns()), 10, y, 16, WHITE);
        y += lineHeight - 4;
        DrawText(TextFormat("Alvos alcançados: %d", stats->getTotalTargetsReached()), 10, y, 16, WHITE);
        y += lineHeight - 4;
        DrawText(TextFormat("Caminhos bloqueados: %d", stats->getTotalPathsBlocked()), 10, y, 16, WHITE);
    }
}

void Application::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (gridAdapter) {
        gridAdapter->Draw();
        
        if (useNewAgentSystem && gameAgentManager) {
            gameAgentManager->drawAll();
        } else if (legacyAgentManager) {
            legacyAgentManager->DrawAll(gridAdapter->GetLegacyGrid());
        }
    }

    // Desenha marcadores de spawn e target
    if (spawnPos.x != -1) {
        if (currentGridType == GridType::HEXAGONAL) {
            auto* hexAdapter = dynamic_cast<HexagonalGridAdapter*>(gridAdapter);
            if (hexAdapter) {
                Vector2 center = hexAdapter->hexToPixel((int)spawnPos.x, (int)spawnPos.y);
                // Usa rotação 0 para pointy-top, raio = hexRadius - 1 para não sobrepor bordas
                DrawPoly(center, 6, hexAdapter->GetHexRadius() - 1, 0.0f, Fade(BLUE, 0.5f));
            }
        } else {
            DrawRectangleRec({spawnPos.x * cellSize, spawnPos.y * cellSize, cellSize, cellSize}, Fade(BLUE, 0.5f));
        }
    }
    if (targetPos.x != -1) {
        if (currentGridType == GridType::HEXAGONAL) {
            auto* hexAdapter = dynamic_cast<HexagonalGridAdapter*>(gridAdapter);
            if (hexAdapter) {
                Vector2 center = hexAdapter->hexToPixel((int)targetPos.x, (int)targetPos.y);
                DrawPoly(center, 6, hexAdapter->GetHexRadius() - 1, 0.0f, Fade(RED, 0.5f));
            }
        } else {
            DrawRectangleRec({targetPos.x * cellSize, targetPos.y * cellSize, cellSize, cellSize}, Fade(RED, 0.5f));
        }
    }
    
    DrawUI();

    EndDrawing();
}
