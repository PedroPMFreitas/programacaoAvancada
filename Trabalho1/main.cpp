#include "raylib/build/raylib/include/raylib.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <fstream>
#include <string>
#include <iomanip>

const float MOVEMENT_THRESHOLD = 2.0f;

struct Poligono{
    Color color;
    Vector2 centro;
    int lados;
    float raio = 50;
    float rotat = 0;
};
struct Linha{
    Color color;
    Vector2 inicio;
    Vector2 fim;
};
struct Circulo{
    Vector2 centro;
    Color color;
    float raio;
};
struct MouseMovement {
    double timestamp;
    int x, y;
};
struct ClickEvent {
    double timestamp;
    int x, y;
    int shapeIndex;
    std::string shapeType;
};
struct SessionInfo {
    std::chrono::steady_clock::time_point startTime;
    std::vector<ClickEvent> clicks;
    std::vector<MouseMovement> movements;
    int totalClicks = 0;
};
SessionInfo currentSession;
bool isLogging = true;

void atualizarVerticesPoligonoGlobal(const Poligono& poligono, std::vector<Vector2> &g_verticesDoPoligono) {
    g_verticesDoPoligono.clear(); 
    float anguloPasso = 360.0f / poligono.lados;
    for (int i = 0; i < poligono.lados; i++) {
        float anguloAtual = poligono.rotat + (anguloPasso * i);
        float anguloRad = anguloAtual * DEG2RAD;
        Vector2 vertice;
        vertice.x = poligono.centro.x + poligono.raio * cosf(anguloRad);
        vertice.y = poligono.centro.y + poligono.raio * sinf(anguloRad);
        g_verticesDoPoligono.push_back(vertice);
    }
}
void startSessionLog() {
    currentSession.startTime = std::chrono::steady_clock::now();
    currentSession.clicks.clear();
    currentSession.movements.clear();
    currentSession.totalClicks = 0;
    std::cout << "Sessão iniciada. Logging ativado." << std::endl;
}
void logMouseMovement(int x, int y) {
    if (!isLogging) return;
    
    static Vector2 lastMousePos = {(float)x, (float)y};
    static bool isFirstMovement = true;
    
    if (isFirstMovement) {
        lastMousePos.x = x;
        lastMousePos.y = y;
        isFirstMovement = false;
        return;
    }
    float distance = sqrt(pow(x - lastMousePos.x, 2) + pow(y - lastMousePos.y, 2));
    
    if (distance >= MOVEMENT_THRESHOLD) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double>(now - currentSession.startTime);
        
        MouseMovement movement;
        movement.timestamp = duration.count();
        movement.x = x;
        movement.y = y;
        
        currentSession.movements.push_back(movement);
        lastMousePos.x = x;
        lastMousePos.y = y;
    }
}
void logClick(int x, int y, int shapeIndex, const std::string& shapeType) {
    if (!isLogging) return;
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration<double>(now - currentSession.startTime);
    
    ClickEvent click;
    click.timestamp = duration.count();
    click.x = x;
    click.y = y;
    click.shapeIndex = shapeIndex;
    click.shapeType = shapeType;
    
    currentSession.clicks.push_back(click);
    currentSession.totalClicks++;
    
    std::cout << "Clique registrado: " << click.timestamp << "s, Acao: " 
              << shapeType << " (Index: " << shapeIndex << ")" << std::endl;
}
void exportSessionData() {
    auto endTime = std::chrono::steady_clock::now();
    auto totalDuration = std::chrono::duration<double>(endTime - currentSession.startTime);
    
    std::ofstream sessionFile("session_summary.csv");
    if (sessionFile.is_open()) {
        sessionFile << "TempoTotalSegundos,TotalCliques,TotalMovimentos\n";
        sessionFile << totalDuration.count() << "," 
                   << currentSession.totalClicks << "," 
                   << currentSession.movements.size() << "\n";
        sessionFile.close();
        std::cout << "Resumo da sessão exportado para session_summary.csv" << std::endl;
    }
    std::ofstream movementsFile("mouse_movements.csv");
    if (movementsFile.is_open()) {
        movementsFile << "Timestamp,PosX,PosY\n";
        for (const auto& movement : currentSession.movements) {
            movementsFile << std::fixed << std::setprecision(3) 
                         << movement.timestamp << ","
                         << movement.x << ","
                         << movement.y << "\n";
        }
        movementsFile.close();
        std::cout << "Movimentos do mouse exportados para mouse_movements.csv" << std::endl;
    }
    std::ofstream clicksFile("clicks_data.csv");
    if (clicksFile.is_open()) {
        clicksFile << "Timestamp,PosX,PosY,ShapeIndex,ShapeType\n";
        for (const auto& click : currentSession.clicks) {
            clicksFile << std::fixed << std::setprecision(3) 
                      << click.timestamp << ","
                      << click.x << ","
                      << click.y << ","
                      << click.shapeIndex << ","
                      << click.shapeType << "\n";
        }
        clicksFile.close();
        std::cout << "Dados de cliques exportados para clicks_data.csv" << std::endl;
    }
}
    int main() {
        InitWindow(1000, 800, "Trabalho de Comp Avançada");
        SetTargetFPS(60);

        std::vector<Vector2> g_verticesDoPoligono;
        std::vector<Poligono> poligonos;
        std::vector<Linha> linhas;
        std::vector<Circulo> circulos;

        int ip = -1;
        int sw, lad;
        bool clickp = false, clickl = false, clickc = false;
        Vector2 mousePos;

        startSessionLog();

        while (!WindowShouldClose()) {

            mousePos = GetMousePosition();
            logMouseMovement(mousePos.x, mousePos.y);

            //imput
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                clickc = false;
                clickl = false;
                clickp = false;
                ip = -2;

                for (auto i = 0; i < poligonos.size(); i++) {
                    atualizarVerticesPoligonoGlobal(poligonos[i], g_verticesDoPoligono);
                    if (CheckCollisionPointPoly(mousePos, g_verticesDoPoligono.data(), poligonos[i].lados)) {
                        clickp = true;
                        ip = i;
                        logClick(mousePos.x, mousePos.y, ip, "Selecao_Poligono");
                        std::cout << "Poligono selecionado. Use WASD/QE/Setas/C para alterar.\n";
                        break; 
                    }
                }
                if (ip == -2) {
                    for (auto i = 0; i < linhas.size(); i++) {
                        if (CheckCollisionPointLine(mousePos, linhas[i].inicio, linhas[i].fim, 10)) {
                            clickl = true;
                            ip = i;
                            logClick(mousePos.x, mousePos.y, ip, "Selecao_Linha");
                            std::cout << "Linha selecionada. Use WASD/QE/Setas/C para alterar.\n";
                            break;
                        }
                    }
                }
                if (ip == -2) {
                    for (auto i = 0; i < circulos.size(); i++) {
                        if (CheckCollisionPointCircle(mousePos, circulos[i].centro, circulos[i].raio)) {
                            clickc = true;
                            ip = i;
                            logClick(mousePos.x, mousePos.y, ip, "Selecao_Circulo");
                            std::cout << "Circulo selecionado. Use WASD/Setas/C para alterar.\n";
                            break;
                        }
                    }
                }
            }
            
            //cria
            if (ip == -2 && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                std::cout << "Clique em area vazia. Escolha a forma:\n";
                std::cout << "1- Circulo | 2- Linha | 3- Poligono\n";
                std::cin >> sw;

                if (sw == 3) {
                    std::cout << "Quantos lados? ";
                    std::cin >> lad;
                    Poligono p = {RED, mousePos, lad, 50.0f, 0.0f};
                    poligonos.push_back(p);
                    logClick(mousePos.x, mousePos.y, poligonos.size() - 1, "Criacao_Poligono");
                } else if (sw == 2) {
                    Linha l = {BLUE, {mousePos.x - 50, mousePos.y}, {mousePos.x + 50, mousePos.y}};
                    linhas.push_back(l);
                    logClick(mousePos.x, mousePos.y, linhas.size() - 1, "Criacao_Linha");
                } else if (sw == 1) {
                    Circulo c = {mousePos, PINK, 10.0f};
                    circulos.push_back(c);
                    logClick(mousePos.x, mousePos.y, circulos.size() - 1, "Criacao_Circulo");
                }
                ip = -1;
            }
            if (ip > -1) {
                if (clickp) {
                    if (IsKeyDown(KEY_E)) poligonos[ip].rotat += 2;
                    if (IsKeyDown(KEY_Q)) poligonos[ip].rotat -= 2;
                    if (IsKeyDown(KEY_W)) poligonos[ip].centro.y -= 2;
                    if (IsKeyDown(KEY_S)) poligonos[ip].centro.y += 2;
                    if (IsKeyDown(KEY_A)) poligonos[ip].centro.x -= 2;
                    if (IsKeyDown(KEY_D)) poligonos[ip].centro.x += 2;
                    if (IsKeyDown(KEY_UP)) poligonos[ip].raio += 1;
                    if (IsKeyDown(KEY_DOWN) && poligonos[ip].raio > 10) poligonos[ip].raio -= 1;
                    if (IsKeyPressed(KEY_C)) {
                        poligonos[ip].color = (Color){(unsigned char)GetRandomValue(50, 250), (unsigned char)GetRandomValue(50, 250), (unsigned char)GetRandomValue(50, 250), 255};
                    }
                }
                if (clickl) {
                    float angulo = 2 * DEG2RAD;
                    if (IsKeyDown(KEY_E)) {
                        Vector2 p = {linhas[ip].fim.x - linhas[ip].inicio.x, linhas[ip].fim.y - linhas[ip].inicio.y};
                        linhas[ip].fim.x = linhas[ip].inicio.x + (p.x * cosf(angulo) - p.y * sinf(angulo));
                        linhas[ip].fim.y = linhas[ip].inicio.y + (p.x * sinf(angulo) + p.y * cosf(angulo));
                    }
                    if (IsKeyDown(KEY_Q)) {
                        Vector2 p = {linhas[ip].fim.x - linhas[ip].inicio.x, linhas[ip].fim.y - linhas[ip].inicio.y};
                        linhas[ip].fim.x = linhas[ip].inicio.x + (p.x * cosf(-angulo) - p.y * sinf(-angulo));
                        linhas[ip].fim.y = linhas[ip].inicio.y + (p.x * sinf(-angulo) + p.y * cosf(-angulo));
                    }
                    if (IsKeyDown(KEY_W)) { linhas[ip].inicio.y -= 2; linhas[ip].fim.y -= 2; }
                    if (IsKeyDown(KEY_S)) { linhas[ip].inicio.y += 2; linhas[ip].fim.y += 2; }
                    if (IsKeyDown(KEY_A)) { linhas[ip].inicio.x -= 2; linhas[ip].fim.x -= 2; }
                    if (IsKeyDown(KEY_D)) { linhas[ip].inicio.x += 2; linhas[ip].fim.x += 2; }
                    if (IsKeyDown(KEY_UP)) {
                        Vector2 meio = {(linhas[ip].inicio.x + linhas[ip].fim.x)/2, (linhas[ip].inicio.y + linhas[ip].fim.y)/2};
                        Vector2 dir = {linhas[ip].fim.x - meio.x, linhas[ip].fim.y - meio.y};
                        linhas[ip].fim = {linhas[ip].fim.x + dir.x*0.01f, linhas[ip].fim.y + dir.y*0.01f};
                        linhas[ip].inicio = {linhas[ip].inicio.x - dir.x*0.01f, linhas[ip].inicio.y - dir.y*0.01f};
                    }
                    if (IsKeyDown(KEY_DOWN)) {
                        Vector2 meio = {(linhas[ip].inicio.x + linhas[ip].fim.x)/2, (linhas[ip].inicio.y + linhas[ip].fim.y)/2};
                        Vector2 dir = {linhas[ip].fim.x - meio.x, linhas[ip].fim.y - meio.y};
                        if(sqrt(pow(dir.x,2)+pow(dir.y,2)) > 10) {
                            linhas[ip].fim = {linhas[ip].fim.x - dir.x*0.01f, linhas[ip].fim.y - dir.y*0.01f};
                            linhas[ip].inicio = {linhas[ip].inicio.x + dir.x*0.01f, linhas[ip].inicio.y + dir.y*0.01f};
                        }
                    }
                    if (IsKeyPressed(KEY_C)) {
                        linhas[ip].color = (Color){(unsigned char)GetRandomValue(50, 250), (unsigned char)GetRandomValue(50, 250), (unsigned char)GetRandomValue(50, 250), 255};
                    }
                }
                if (clickc) {
                    if (IsKeyDown(KEY_W)) circulos[ip].centro.y -= 2;
                    if (IsKeyDown(KEY_S)) circulos[ip].centro.y += 2;
                    if (IsKeyDown(KEY_A)) circulos[ip].centro.x -= 2;
                    if (IsKeyDown(KEY_D)) circulos[ip].centro.x += 2;
                    if (IsKeyDown(KEY_UP)) circulos[ip].raio += 1;
                    if (IsKeyDown(KEY_DOWN) && circulos[ip].raio > 10) circulos[ip].raio -= 1;
                    if (IsKeyPressed(KEY_C)) {
                        circulos[ip].color = (Color){(unsigned char)GetRandomValue(50, 250), (unsigned char)GetRandomValue(50, 250), (unsigned char)GetRandomValue(50, 250), 255};
                    }
                }
            }
            //desenho
            BeginDrawing();
            ClearBackground(BLACK);

            for (const auto& poligono : poligonos) {
                DrawPoly(poligono.centro, poligono.lados, poligono.raio, poligono.rotat, poligono.color);
                DrawPolyLinesEx(poligono.centro, poligono.lados, poligono.raio, poligono.rotat, 2.0, DARKGRAY);
            }
            for (const auto& linha : linhas) {
                DrawLineEx(linha.inicio, linha.fim, 5.0, linha.color);
            }
            for (const auto& circulo : circulos) {
                DrawCircleV(circulo.centro, circulo.raio, circulo.color);
            }

            EndDrawing();
        }

        exportSessionData();
        CloseWindow();
        return 0;
    }
