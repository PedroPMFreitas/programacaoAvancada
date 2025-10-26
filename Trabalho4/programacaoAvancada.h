#pragma once

#include "/home/pedro/raylib/build/raylib/include/raylib.h"
#include <vector>

// Estruturas de dados
struct Poly {
    std::vector<Vector2> arestas;
    Color color;
    int lados;
    float raio = 50;
    float rotat = 0;
};

struct Triangulo {
    Vector2 a, b, c;
    int index_a, index_b, index_c;
};

struct Aresta {
    Vector2 a, b;
    bool operator==(const Aresta& other) const;
};

struct Linha {
    Color color;
    Vector2 inicio;
    Vector2 fim;
};

struct Circulo {
    Vector2 centro;
    Color color;
    float raio;
};


// --- Estruturas de Dados para Logging ---

struct EnvoltoriaPerformanceData {
    long long snapshot_id;
    int total_points;
    int points_on_hull;
    int points_inside_hull;
    long long execution_time_microseconds;
};

struct PontoLog {
    long long snapshot_id;
    Vector2 point;
};

struct MinkowskiPerformanceData {
    long long robot_vertex_count;
    long long obstacle_vertex_count;
    long long result_vertex_count;
    long long execution_time_microseconds;
};



// --- Declarações de Funções de Algoritmos ---

// Calcula a triangulação de Delaunay para um conjunto de pontos.
std::vector<Triangulo> delaunay(std::vector<Vector2> &pontos);

// Calcula a envoltória convexa usando o algoritmo da corrente monótona.
std::vector<Vector2> correnteMonotona(std::vector<Vector2> n);

// Calcula o circuncentro de um triângulo (usado para o diagrama de Voronoi).
Vector2 calcularCircuncentro(const Triangulo& t);

// Calcula a Soma de Minkowski de dois polígonos convexos.
std::vector<Poly> minkowskiSum(const Poly& A, const Poly& B);

Poly refletirPoligono(const Poly& p);
float distanciaOrigemPoligono(const Poly& p);


// --- Declarações de Funções de Exportação para CSV ---
void exportEnvoltoriaData(const std::vector<EnvoltoriaPerformanceData>& log);
void exportPontosLog(const std::vector<PontoLog>& log);
void exportMinkowskiData(const std::vector<MinkowskiPerformanceData>& log);
void exportDistanceMatrix(const std::vector<Poly>& poligonos);
