#include "/home/pedro/raylib/build/raylib/include/raylib.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <string>
#include <iomanip>

struct Poligono{
    Color color;
    Vector2 centro;
    int lados;
    float raio = 50;
    float rotat = 0;
};
struct Triangulo{
    Vector2 a,b,c;
    int index_a, index_b, index_c;
};
struct Aresta { 
    Vector2 a, b;
    bool operator==(const Aresta& other) const {
        return (a.x == other.a.x && a.y == other.a.y && b.x == other.b.x && b.y == other.b.y) ||
               (a.x == other.b.x && a.y == other.b.y && b.x == other.a.x && b.y == other.a.y);
    }
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
struct PerformanceData {
    int pointCount;
    long long durationMicroseconds;
};

bool PontoDentroCircuncirculo(const Vector2& p, const Triangulo& t) {
    float ax = t.a.x - p.x;
    float ay = t.a.y - p.y;
    float bx = t.b.x - p.x;
    float by = t.b.y - p.y;
    float cx = t.c.x - p.x;
    float cy = t.c.y - p.y;

    float det = (ax * ax + ay * ay) * (bx * cy - cx * by) -
                (bx * bx + by * by) * (ax * cy - cx * ay) +
                (cx * cx + cy * cy) * (ax * by - bx * ay);

    return det > 0.0f;
}
Vector2 calcularCircuncentro(const Triangulo& t) {
    float D = 2 * (t.a.x * (t.b.y - t.c.y) + t.b.x * (t.c.y - t.a.y) + t.c.x * (t.a.y - t.b.y));
    
    if (std::abs(D) < 1e-6) {
        return {(t.a.x + t.b.x + t.c.x) / 3, (t.a.y + t.b.y + t.c.y) / 3};
    }

    float Ux = ((t.a.x * t.a.x + t.a.y * t.a.y) * (t.b.y - t.c.y) + 
                (t.b.x * t.b.x + t.b.y * t.b.y) * (t.c.y - t.a.y) + 
                (t.c.x * t.c.x + t.c.y * t.c.y) * (t.a.y - t.b.y)) / D;
    
    float Uy = ((t.a.x * t.a.x + t.a.y * t.a.y) * (t.c.x - t.b.x) + 
                (t.b.x * t.b.x + t.b.y * t.b.y) * (t.a.x - t.c.x) + 
                (t.c.x * t.c.x + t.c.y * t.c.y) * (t.b.x - t.a.x)) / D;

    return {Ux, Uy};
}

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

void exportPerformanceData(const std::vector<PerformanceData>& data) {
    std::ofstream performanceFile("delaunay_performance.csv");
    if (performanceFile.is_open()) {
        performanceFile << "NumberOfPoints,ExecutionTimeMicroseconds\n";
        for (const auto& entry : data) {
            performanceFile << entry.pointCount << "," << entry.durationMicroseconds << "\n";
        }
        performanceFile.close();
        std::cout << "Dados de desempenho exportados para delaunay_performance.csv" << std::endl;
    } else {
        std::cerr << "Erro ao abrir o arquivo delaunay_performance.csv para escrita." << std::endl;
    }
}

std::vector<Triangulo> delaunay(std::vector<Vector2> &pontos){
    std::vector<Triangulo> triangulos;

    float minX = 0, minY = 0, maxX = 800, maxY = 800;
    Vector2 p1 = { minX - 1000, minY - 1000 };
    Vector2 p2 = { maxX + 1000, minY - 1000 };
    Vector2 p3 = { (minX + maxX) / 2, maxY + 1000 };
    triangulos.push_back({ p1, p2, p3, -1, -1, -1 }); 
    
    for (int point_idx = 0; point_idx < pontos.size(); point_idx++) {
        const Vector2& p = pontos[point_idx];
        std::vector<Aresta> borda;
        std::vector<Triangulo> novos;

        for (auto& t : triangulos) {
            if (PontoDentroCircuncirculo(p, t)) {
                borda.push_back({ t.a, t.b });
                borda.push_back({ t.b, t.c });
                borda.push_back({ t.c, t.a });
            } else {
                novos.push_back(t);
            }
        }

        std::vector<Aresta> bordaUnica;
        for (auto& e : borda) {
            if (std::count(borda.begin(), borda.end(), e) == 1)
                bordaUnica.push_back(e);
        }

        for (auto& e : bordaUnica) {
            int idx_a = -1, idx_b = -1, idx_c = point_idx;
            
            for (int i = 0; i < pontos.size(); i++) {
                if (std::abs(pontos[i].x - e.a.x) < 1e-6 && std::abs(pontos[i].y - e.a.y) < 1e-6) {
                    idx_a = i;
                    break;
                }
            }
            for (int i = 0; i < pontos.size(); i++) {
                if (std::abs(pontos[i].x - e.b.x) < 1e-6 && std::abs(pontos[i].y - e.b.y) < 1e-6) {
                    idx_b = i;
                    break;
                }
            }
            
            if (idx_a == -1 || idx_b == -1) {
                novos.push_back({ e.a, e.b, p, -1, -1, idx_c });
            } else {
                novos.push_back({ e.a, e.b, p, idx_a, idx_b, idx_c });
            }
        }
        triangulos = novos;
    }

    triangulos.erase(std::remove_if(triangulos.begin(), triangulos.end(),
        [&](const Triangulo& t) {
            return (t.a.x < minX - 500 || t.b.x < minX - 500 || t.c.x < minX - 500 ||
                    t.a.y < minY - 500 || t.b.y < minY - 500 || t.c.y < minY - 500 ||
                    t.a.x > maxX + 500 || t.b.x > maxX + 500 || t.c.x > maxX + 500 ||
                    t.a.y > maxY + 500 || t.b.y > maxY + 500 || t.c.y > maxY + 500);
        }),
        triangulos.end());
    
    return triangulos;
}

int main() {
    InitWindow(800, 800, "Trabalho de Comp Avançada");
    SetTargetFPS(60);

    std::vector<Vector2> g_verticesDoPoligono;
    std::vector<Poligono> poligonos;
    std::vector<Linha> linhas;
    std::vector<Circulo> circulos;
    std::vector<Vector2> pontos;
    std::vector<Triangulo> triangulos;
    std::vector<Color>cores;

    std::vector<PerformanceData> performanceLog;

    int ip = -1;
    int sw, lad;
    bool clickp = false, clickl = false, clickc = false;
    Vector2 mousePos;
    
    while (!WindowShouldClose()) {
        mousePos = GetMousePosition();

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
                    std::cout << "Poligono selecionado. Use WASD/QE/Setas/C para alterar.\n";
                    break; 
                }
            }
            if (ip == -2) {
                for (auto i = 0; i < linhas.size(); i++) {
                    if (CheckCollisionPointLine(mousePos, linhas[i].inicio, linhas[i].fim, 10)) {
                        clickl = true;
                        ip = i;
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
            } else if (sw == 2) {
                Linha l = {BLUE, {mousePos.x - 50, mousePos.y}, {mousePos.x + 50, mousePos.y}};
                linhas.push_back(l);
            } else if (sw == 1) {
                Circulo c = {mousePos, PINK, 10.0f};
                circulos.push_back(c);
            }
            ip = -1;
        } else if(IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
            pontos.push_back(mousePos);
            cores.push_back({(unsigned char)GetRandomValue(100, 220), 
                (unsigned char)GetRandomValue(100, 220), 
                (unsigned char)GetRandomValue(100, 220), 255});
            if(pontos.size() >= 3){
                auto start = std::chrono::high_resolution_clock::now();
                triangulos = delaunay(pontos);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                
                performanceLog.push_back({(int)pontos.size(), duration});
                std::cout << "Pontos: " << pontos.size() << ", Tempo: " << duration << " us\n";
            }
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
        if (triangulos.size() > 0) {
            for (size_t i = 0; i < pontos.size(); ++i) {
                std::vector<Vector2> verticesCelula;
                
                for (const auto& t : triangulos) {
                    if ((t.index_a == (int)i || t.index_b == (int)i || t.index_c == (int)i) && 
                        t.index_a >= 0 && t.index_b >= 0 && t.index_c >= 0) { 
                        Vector2 circuncentro = calcularCircuncentro(t);
                        verticesCelula.push_back(circuncentro);
                    }
                }

                if (verticesCelula.size() > 2) {
                    Vector2 pontoAtual = pontos[i];
                    std::sort(verticesCelula.begin(), verticesCelula.end(), [&](const Vector2& v1, const Vector2& v2) {
                        return atan2(v1.y - pontoAtual.y, v1.x - pontoAtual.x) < 
                               atan2(v2.y - pontoAtual.y, v2.x - pontoAtual.x);
                    });

                    if (verticesCelula.size() >= 3) {
                        for (size_t j = 0; j < verticesCelula.size(); j++) {
                            Vector2 current = verticesCelula[j];
                            Vector2 next = verticesCelula[(j + 1) % verticesCelula.size()];
                            DrawLineEx(current, next, 1.0f, Fade(cores[i], 0.7f));
                        }
                    }
                }
            }
        }        
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
        for (const auto& triangulo : triangulos){
            DrawTriangleLines(triangulo.a, triangulo.b, triangulo.c, WHITE);
        }
        for (const auto& ponto : pontos){
            DrawCircleV(ponto, 4 ,RED);
        }

        EndDrawing();
    }

    exportPerformanceData(performanceLog);
    CloseWindow();
    return 0;
}

