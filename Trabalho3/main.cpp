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
    std::vector<Vector2> pontos;
    std::vector<Vector2> envConv;
    std::vector<Triangulo> triangulos;
    std::vector<Color> cores_voronoi;

    // Logs de dados
    std::vector<EnvoltoriaPerformanceData> envoltoria_log;
    std::vector<PontoLog> pontos_log;
    long long snapshot_id_counter = 0;

    // Variáveis de Estado
    int sw = 0, ip = -2, x = -2;
    Vector2 mousePos;
    bool show_delaunay = false, show_voronoi = false, show_envoltoria = true; 

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
                    if (IsKeyPressed(KEY_TWO)) { x = 2; }
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
                    if (IsKeyPressed(KEY_A)) { x = 2; }
                    if (IsKeyPressed(KEY_N)) { x = -2; ip = -2; pontos.clear(); envConv.clear(); poligonos.clear(); triangulos.clear(); cores_voronoi.clear(); show_delaunay = false; show_voronoi = false; show_envoltoria = true; }
                    break;
                case 2: 
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { inputState = 2; }
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
                    if (!poligonos.empty() && poligonos.back().lados == sw && ip == 0) { x = -1; ip = -2; }
                    break;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (show_voronoi && triangulos.size() > 0) { for (size_t i = 0; i < pontos.size(); ++i) { std::vector<Vector2> v; for (const auto& t : triangulos) { if ((t.index_a == (int)i || t.index_b == (int)i || t.index_c == (int)i)) v.push_back(calcularCircuncentro(t)); } if (v.size() > 2) { Vector2 p = pontos[i]; std::sort(v.begin(), v.end(), [&](auto v1, auto v2) { return atan2(v1.y-p.y,v1.x-p.x)<atan2(v2.y-p.y,v2.x-p.x);}); for (size_t j=0;j<v.size();j++) DrawLineEx(v[j],v[(j+1)%v.size()],2.0f,Fade(cores_voronoi[i],0.7f)); } } } 
        if (show_delaunay) { for (const auto& t : triangulos) DrawTriangleLines(t.a, t.b, t.c, WHITE); } 
        if (show_envoltoria) { for (size_t i = 0; i < envConv.size(); i++) DrawLineEx(envConv[i], envConv[(i + 1) % envConv.size()], 2.0, DARKGREEN); } 
        if (!poligonos.empty()) { const auto& p=poligonos.back(); if(p.lados>1) for(int i=0;i<p.lados-1;i++) DrawLineEx(p.arestas[i],p.arestas[i+1],2.0f,YELLOW); if(x==4&&p.lados>0&&p.lados<sw) DrawLineEx(p.arestas.back(),GetMousePosition(),2.0f,GREEN); if(p.lados==sw&&sw>2) for(int i=0;i<p.lados;i++) DrawLineEx(p.arestas[i],p.arestas[(i+1)%p.lados],2.0f,BLUE); } 
        for (const Vector2 p : pontos) DrawCircleV(p, 4, RED);

        if (inputState != 0) { DrawText(inputState == 1 ? "Quantos pontos deseja adicionar?" : "Quantos lados tera o seu poligono?", 10, 10, 20, WHITE); DrawText("(Digite e pressione Enter)", 10, 35, 18, WHITE); DrawText(TextFormat("Valor: %s", inputBuffer), 10, 60, 20, LIME); } else { switch (x) { case -2: DrawText("1: Gerar pontos | 2: Criar poligono", 10, 10, 20, WHITE); break; case -1: DrawText("Clique para add ponto | (G)erar | (V)oronoi | (D)elaunay | (E)nvoltoria | (A)dd Poligono | (N)ovo", 10, 10, 20, WHITE); break; case 2: DrawText("Clique para iniciar um poligono", 10, 10, 20, WHITE); break; case 4: DrawText(TextFormat("Faltam %d vertices. Clique direito para adicionar.", ip), 10, 10, 20, WHITE); break; } } 

        EndDrawing();
    }

    exportEnvoltoriaData(envoltoria_log);
    exportPontosLog(pontos_log);

    CloseWindow();
    return 0;
}
