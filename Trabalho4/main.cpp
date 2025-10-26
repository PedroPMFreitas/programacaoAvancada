#include "programacaoAvancada.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <fstream> 
#include <chrono>    


int main() {
    InitWindow(800, 800, "Trabalho de Comp Avançada");
    SetTargetFPS(60);

    // Vetores de Geometria
    std::vector<Poly> poligonos;
    std::vector<Poly> minkowski_results;
    std::vector<Vector2> pontos;
    std::vector<Vector2> envConv;
    std::vector<Triangulo> triangulos;
    std::vector<Color> cores_voronoi;

    // Logs de dados
    std::vector<EnvoltoriaPerformanceData> envoltoria_log;
    std::vector<PontoLog> pontos_log;
    std::vector<MinkowskiPerformanceData> minkowski_log;
    long long snapshot_id_counter = 0;

    // Variáveis de Estado
    int sw = 0, ip = -2, x = -2, robo_selecionado = -1, return_state = -1;
    Vector2 mousePos;
    bool show_delaunay = false, show_voronoi = false, show_envoltoria = false; 

    char inputBuffer[10] = "";
    int inputBufferCount = 0;
    int inputState = 0; 

    auto take_snapshot = [&]() {
        if (pontos.empty()) return;
        snapshot_id_counter++;
        auto start = std::chrono::high_resolution_clock::now();
        envConv = correnteMonotona(pontos);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        envoltoria_log.push_back({snapshot_id_counter, (int)pontos.size(), (int)envConv.size(), (int)pontos.size() - (int)envConv.size(), duration});
        for(const auto& p : pontos) {
            pontos_log.push_back({snapshot_id_counter, p});
        }
    };

    auto recalcularMinkowski = [&]() {
        minkowski_results.clear();
        if (robo_selecionado != -1 && poligonos.size() > 1) {
            Poly centered_robot = poligonos[robo_selecionado];
            Vector2 center = {0, 0};
            for (const auto& v : centered_robot.arestas) {
                center.x += v.x;
                center.y += v.y;
            }
            center.x /= centered_robot.lados;
            center.y /= centered_robot.lados;

            for (auto& v : centered_robot.arestas) {
                v.x -= center.x;
                v.y -= center.y;
            }

            for (size_t i = 0; i < poligonos.size(); ++i) {
                if (i == robo_selecionado) continue;
                
                auto start = std::chrono::high_resolution_clock::now();
                auto resultado = minkowskiSum(centered_robot, poligonos[i]);
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

                if (!resultado.empty()) {
                    minkowski_log.push_back({(long long)centered_robot.lados, (long long)poligonos[i].lados, (long long)resultado[0].lados, duration});
                    minkowski_results.insert(minkowski_results.end(), resultado.begin(), resultado.end());
                }
            }
        }
    };

    while (!WindowShouldClose()) {
        mousePos = GetMousePosition();

        if (inputState != 0) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= '0' && key <= '9') && (inputBufferCount < 9)) { inputBuffer[inputBufferCount++] = (char)key; inputBuffer[inputBufferCount] = '\0'; }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) { if (inputBufferCount > 0) { inputBufferCount--; inputBuffer[inputBufferCount] = '\0'; } }

            if (IsKeyPressed(KEY_ENTER)) {
                sw = atoi(inputBuffer);
                if (inputState == 1) { 
                    for (int i = 0; i < sw; i++) {
                        pontos.push_back({(float)GetRandomValue(50, 750), (float)GetRandomValue(50, 750)});
                        cores_voronoi.push_back({(unsigned char)GetRandomValue(100, 220), (unsigned char)GetRandomValue(100, 220), (unsigned char)GetRandomValue(100, 220), 255});
                    }
                    take_snapshot();
                    if (show_delaunay || show_voronoi) triangulos = delaunay(pontos);
                    x = -1; 
                } else if (inputState == 2) { 
                    Poly p; p.lados = 0; p.arestas.clear(); poligonos.push_back(p); ip = sw; x = 4; 
                }
                inputState = 0; inputBufferCount = 0; inputBuffer[0] = '\0';
            }
        } else {
            switch (x) {
                case -2: 
                    if (IsKeyPressed(KEY_ONE)) { inputState = 1; }
                    if (IsKeyPressed(KEY_TWO)) { return_state = -1; x = 2; }
                    if (IsKeyPressed(KEY_M)){x = 3;}
                    break;
                case -1: 
                    if (IsKeyPressed(KEY_V)) { show_voronoi = !show_voronoi; show_delaunay = false; if (show_voronoi && pontos.size() >= 3) triangulos = delaunay(pontos); }
                    if (IsKeyPressed(KEY_D)) { show_delaunay = !show_delaunay; show_voronoi = false; if (show_delaunay && pontos.size() >= 3) triangulos = delaunay(pontos); }
                    if (IsKeyPressed(KEY_E)) { show_envoltoria = !show_envoltoria; }
                    if (IsKeyPressed(KEY_G)) { inputState = 1; }
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        pontos.push_back(mousePos);
                        cores_voronoi.push_back({(unsigned char)GetRandomValue(100, 220), (unsigned char)GetRandomValue(100, 220), (unsigned char)GetRandomValue(100, 220), 255});
                        take_snapshot();
                        if (show_delaunay || show_voronoi) triangulos = delaunay(pontos);
                    }
                    if (IsKeyPressed(KEY_A)) { return_state = -1; x = 2; }
                    if (IsKeyPressed(KEY_N)) { x = -2; ip = -2; pontos.clear(); envConv.clear(); poligonos.clear(); triangulos.clear(); cores_voronoi.clear(); show_delaunay = false; show_voronoi = false; show_envoltoria = true; }
                    break;
                case 2: 
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { inputState = 2; }
                    break;
                case 3:
                    if (IsKeyPressed(KEY_C)) { return_state = 3; inputState = 2; }
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        mousePos = GetMousePosition();
                        bool robot_found = false;
                        for (size_t i = 0; i < poligonos.size(); ++i) {
                            const auto& p = poligonos[i];
                            if (p.lados > 2 && CheckCollisionPointPoly(mousePos, p.arestas.data(), p.lados)) {
                                robo_selecionado = i;
                                robot_found = true;
                                break;
                            }
                        }
                        if (!robot_found) {
                            robo_selecionado = -1;
                        }
                        recalcularMinkowski();
                    }
                    if (robo_selecionado != -1) {
                        bool moved = false;
                        if (IsKeyDown(KEY_UP)) { for (auto& v : poligonos[robo_selecionado].arestas) v.y -= 5; moved = true; }
                        if (IsKeyDown(KEY_DOWN)) { for (auto& v : poligonos[robo_selecionado].arestas) v.y += 5; moved = true; }
                        if (IsKeyDown(KEY_LEFT)) { for (auto& v : poligonos[robo_selecionado].arestas) v.x -= 5; moved = true; }
                        if (IsKeyDown(KEY_RIGHT)) { for (auto& v : poligonos[robo_selecionado].arestas) v.x += 5; moved = true; }
                        if (moved) recalcularMinkowski();
                    }
                    if (IsKeyPressed(KEY_N)) { x = -2; }
                    break;
                case 4: 
                    if (ip > 0 && !poligonos.empty()) {
                        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                            poligonos.back().arestas.push_back(mousePos);
                            pontos.push_back(mousePos);
                            cores_voronoi.push_back({(unsigned char)GetRandomValue(100, 220), (unsigned char)GetRandomValue(100, 220), (unsigned char)GetRandomValue(100, 220), 255});
                            take_snapshot();
                            if (show_delaunay || show_voronoi) triangulos = delaunay(pontos);
                            poligonos.back().lados++;
                            ip--;
                        }
                    }
                    if (!poligonos.empty() && poligonos.back().lados == sw && ip == 0) {
                        if (return_state == 3) { // Apenas para o menu Minkowski
                            auto& poly = poligonos.back();
                            poly.arestas = correnteMonotona(poly.arestas);
                            poly.lados = poly.arestas.size();
                        }
                        x = return_state; 
                        ip = -2; 
                    }
                    break;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (show_voronoi && triangulos.size() > 0) { for (size_t i = 0; i < pontos.size(); ++i) { std::vector<Vector2> v; for (const auto& t : triangulos) { if ((t.index_a == (int)i || t.index_b == (int)i || t.index_c == (int)i)) v.push_back(calcularCircuncentro(t)); } if (v.size() > 2) { Vector2 p = pontos[i]; std::sort(v.begin(), v.end(), [&](auto v1, auto v2) { return atan2(v1.y-p.y,v1.x-p.x)<atan2(v2.y-p.y,v2.x-p.x);}); for (size_t j=0;j<v.size();j++) DrawLineEx(v[j],v[(j+1)%v.size()],2.0f,Fade(cores_voronoi[i],0.7f)); } } } 
        if (show_delaunay) { for (const auto& t : triangulos) DrawTriangleLines(t.a, t.b, t.c, WHITE); } 
                                if (show_envoltoria) { for (size_t i = 0; i < envConv.size(); i++) DrawLineEx(envConv[i], envConv[(i + 1) % envConv.size()], 2.0, DARKGREEN); } 
                        
                                for (const auto& p : minkowski_results) {
                                    if (p.lados > 1) {
                                        for (int j = 0; j < p.lados; j++) {
                                            DrawLineEx(p.arestas[j], p.arestas[(j + 1) % p.lados], 2.0f, p.color);
                                        }
                                    }
                                }
                        
                                for (size_t i = 0; i < poligonos.size(); ++i) {
                                    const auto& p = poligonos[i];
                                    Color c = (i == robo_selecionado) ? RED : BLUE;
                                    if (p.lados > 1) {
                                        for (int j = 0; j < p.lados; j++) {
                                            DrawLineEx(p.arestas[j], p.arestas[(j + 1) % p.lados], 2.0f, c);
                                        }
                                        if (i == robo_selecionado) {
                                            Vector2 center = {0, 0};
                                            for (const auto& v : p.arestas) {
                                                center.x += v.x;
                                                center.y += v.y;
                                            }
                                            center.x /= p.lados;
                                            center.y /= p.lados;
                                            DrawCircleV(center, 3, WHITE);
                                        }
                                    }
                                }
                        
                                if (!poligonos.empty()) { const auto& p=poligonos.back(); if(x==4&&p.lados>0&&p.lados<sw) DrawLineEx(p.arestas.back(),GetMousePosition(),2.0f,GREEN); }                         if (x != 3) {
        
                            for (const Vector2 p : pontos) DrawCircleV(p, 4, RED);
        
                        }

        if (inputState != 0) { DrawText(inputState == 1 ? "Quantos pontos deseja adicionar?" : "Quantos lados tera o seu poligono?", 10, 10, 20, WHITE); DrawText("(Digite e pressione Enter)", 10, 35, 18, WHITE); DrawText(TextFormat("Valor: %s", inputBuffer), 10, 60, 20, LIME); } else { switch (x) { case -2: DrawText("1: Gerar pontos | 2: Criar poligono | m: Minkowski", 10, 10, 20, WHITE); break; case -1: DrawText("Clique para add ponto | (G)erar | (V)oronoi | (D)elaunay | (E)nvoltoria | (A)dd Poligono | (N)ovo", 10, 10, 20, WHITE); break; case 2: DrawText("Clique para iniciar um poligono", 10, 10, 20, WHITE); break; case 3: DrawText("(C)riar | (N)ovo | Clique para selecionar/mover robo", 10, 10, 20, WHITE); break; case 4: DrawText(TextFormat("Faltam %d vertices. Clique direito para adicionar.", ip), 10, 10, 20, WHITE); break; } } 

        EndDrawing();
    }

    exportEnvoltoriaData(envoltoria_log);
    exportPontosLog(pontos_log);
    exportMinkowskiData(minkowski_log);
    exportDistanceMatrix(poligonos);

    CloseWindow();
    return 0;
}
