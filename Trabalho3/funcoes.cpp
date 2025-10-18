#include "programacaoAvancada.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>


bool operator<(const Vector2& a, const Vector2& b) {
    if (a.x != b.x) {
        return a.x < b.x;
    }
    return a.y < b.y;
}

float clockwise(const Vector2 &a, const Vector2 &b, const Vector2 &c){
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

std::vector<Vector2> correnteMonotona(std::vector<Vector2> n){
    if (n.size()<3){
        return n;
    }
    std::sort(n.begin(),n.end());
    std::vector<Vector2>listaU;
    std::vector<Vector2>listaL;
    for (size_t i = 0; i < n.size(); i++){
        while((listaL.size() >= 2) && clockwise(listaL[listaL.size()-2],listaL.back(),n[i]) <= 0){
            listaL.pop_back();
        }
        listaL.push_back(n[i]);
    }
    for (int i = n.size()-1; i >= 0; i--){
        while((listaU.size() >= 2) && clockwise(listaU[listaU.size()-2],listaU.back(),n[i]) <= 0){
            listaU.pop_back();
        }
        listaU.push_back(n[i]);
    }
    listaU.pop_back();
    listaL.pop_back();
    listaL.insert(listaL.end(),listaU.begin(),listaU.end());
    return listaL;
}

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

bool Aresta::operator==(const Aresta& other) const {
    return (a.x == other.a.x && a.y == other.a.y && b.x == other.b.x && b.y == other.b.y) ||
           (a.x == other.b.x && a.y == other.b.y && b.x == other.a.x && b.y == other.a.y);
}

std::vector<Triangulo> delaunay(std::vector<Vector2> &pontos){
    std::vector<Triangulo> triangulos;
    if (pontos.empty()) return triangulos;
    float minX = 0, minY = 0, maxX = 800, maxY = 800;
    Vector2 p1 = { minX - 1000, minY - 1000 };
    Vector2 p2 = { maxX + 1000, minY - 1000 };
    Vector2 p3 = { (minX + maxX) / 2, maxY + 1000 };
    triangulos.push_back({ p1, p2, p3, -1, -1, -1 }); 
    std::vector<int> indices(pontos.size());
    for(size_t i = 0; i < pontos.size(); ++i) indices[i] = i;
    for (int point_idx : indices) {
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
            int idx_a = -1, idx_b = -1;
            for(size_t i = 0; i < pontos.size(); ++i) {
                if (pontos[i].x == e.a.x && pontos[i].y == e.a.y) idx_a = i;
                if (pontos[i].x == e.b.x && pontos[i].y == e.b.y) idx_b = i;
            }
            novos.push_back({ e.a, e.b, p, idx_a, idx_b, point_idx });
        }
        triangulos = novos;
    }
    triangulos.erase(std::remove_if(triangulos.begin(), triangulos.end(),
        [&](const Triangulo& t) {
            bool hasP1 = (t.a.x == p1.x && t.a.y == p1.y) || (t.b.x == p1.x && t.b.y == p1.y) || (t.c.x == p1.x && t.c.y == p1.y);
            bool hasP2 = (t.a.x == p2.x && t.a.y == p2.y) || (t.b.x == p2.x && t.b.y == p2.y) || (t.c.x == p2.x && t.c.y == p2.y);
            bool hasP3 = (t.a.x == p3.x && t.a.y == p3.y) || (t.b.x == p3.x && t.b.y == p3.y) || (t.c.x == p3.x && t.c.y == p3.y);
            return hasP1 || hasP2 || hasP3;
        }),
        triangulos.end());
    return triangulos;
}

void exportEnvoltoriaData(const std::vector<EnvoltoriaPerformanceData>& log) {
    std::ofstream csvFile("envoltoria_performance.csv");
    if (!csvFile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo envoltoria_performance.csv." << std::endl;
        return;
    }
    csvFile << "SnapshotID,TotalPoints,PointsOnHull,PointsInsideHull,ExecutionTimeMicroseconds\n";
    for (const auto& data : log) {
        csvFile << data.snapshot_id << ","
                << data.total_points << ","
                << data.points_on_hull << ","
                << data.points_inside_hull << ","
                << data.execution_time_microseconds << "\n";
    }
    csvFile.close();
    std::cout << "Dados de desempenho da envoltória exportados para envoltoria_performance.csv" << std::endl;
}

void exportPontosLog(const std::vector<PontoLog>& log) {
    std::ofstream csvFile("pontos_log.csv");
    if (!csvFile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo pontos_log.csv." << std::endl;
        return;
    }
    csvFile << "SnapshotID,X,Y\n";
    for (const auto& data : log) {
        csvFile << data.snapshot_id << "," << data.point.x << "," << data.point.y << "\n";
    }
    csvFile.close();
    std::cout << "Log de pontos exportado para pontos_log.csv" << std::endl;
}