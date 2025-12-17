#ifndef HEXAGONAL_GRID_ADAPTER_H
#define HEXAGONAL_GRID_ADAPTER_H

#include "../../src/Interfaces/IGridAdapter.h"
#include "../../src/Core/Cell.h"
#include "raylib.h"
#include <vector>
#include <list>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <algorithm>

// Grid Hexagonal usando coordenadas offset (odd-q)
// Layout "pointy-top" com colunas ímpares deslocadas para baixo
class HexagonalGridAdapter : public IGridAdapter {
private:
    int width;   // número de colunas
    int height;  // número de linhas
    float hexRadius = 12.0f;  // raio do hexágono (distância do centro ao vértice)
    std::vector<std::vector<bool>> obstacles;
    Grid legacyGrid; // For compatibility with the old system

    // Dimensões derivadas do hexágono (pointy-top)
    float hexWidth() const { return std::sqrt(3.0f) * hexRadius; }
    float hexHeight() const { return hexRadius * 2.0f; }
    float horizSpacing() const { return hexWidth(); }
    float vertSpacing() const { return hexHeight() * 0.75f; }

public:
    HexagonalGridAdapter(int w, int h) 
        : width(w), height(h), 
          legacyGrid(w, h, hexRadius * 2) {
        obstacles.resize(width, std::vector<bool>(height, false));
    }

    // Converte coordenadas do grid para posição em pixels (centro do hexágono)
    Vector2 hexToPixel(int col, int row) const {
        float x = col * horizSpacing() + hexWidth() / 2.0f;
        float y = row * vertSpacing() + hexRadius;
        
        // Offset para colunas ímpares (odd-q layout)
        if (col % 2 == 1) {
            y += vertSpacing() / 2.0f;
        }
        
        return {x, y};
    }

    // Converte posição em pixels para coordenadas do grid
    Cell pixelToHex(float px, float py) const {
        // Estimativa inicial baseada no espaçamento
        int col = (int)(px / horizSpacing());
        col = std::max(0, std::min(col, width - 1));
        
        // Ajuste para offset de coluna ímpar
        float adjustedY = py;
        if (col % 2 == 1) {
            adjustedY -= vertSpacing() / 2.0f;
        }
        
        int row = (int)(adjustedY / vertSpacing());
        row = std::max(0, std::min(row, height - 1));
        
        // Refinamento: verifica hexágonos vizinhos para encontrar o mais próximo
        float minDist = 999999.0f;
        int bestCol = col, bestRow = row;
        
        for (int dc = -1; dc <= 1; dc++) {
            for (int dr = -1; dr <= 1; dr++) {
                int testCol = col + dc;
                int testRow = row + dr;
                if (isValidCoordinate(testCol, testRow)) {
                    Vector2 center = hexToPixel(testCol, testRow);
                    float dist = (px - center.x) * (px - center.x) + (py - center.y) * (py - center.y);
                    if (dist < minDist) {
                        minDist = dist;
                        bestCol = testCol;
                        bestRow = testRow;
                    }
                }
            }
        }
        
        return {bestCol, bestRow};
    }

    // Desenha um hexágono individual (pointy-top: rotação de 0 graus)
    void DrawHexagon(Vector2 center, Color fillColor, Color lineColor, bool filled) const {
        if (filled) {
            DrawPoly(center, 6, hexRadius - 1, 0.0f, fillColor);
        }
        DrawPolyLines(center, 6, hexRadius, 0.0f, lineColor);
    }

    void Draw() override {
        for (int col = 0; col < width; col++) {
            for (int row = 0; row < height; row++) {
                Vector2 center = hexToPixel(col, row);
                
                if (obstacles[col][row]) {
                    DrawHexagon(center, BLACK, DARKGRAY, true);
                } else {
                    DrawHexagon(center, RAYWHITE, LIGHTGRAY, false);
                }
            }
        }
    }
    
    void SetObstacle(int x, int y, bool isObstacle) override {
        if (isValidCoordinate(x, y)) {
            obstacles[x][y] = isObstacle;
            legacyGrid.SetOccupied(x, y, isObstacle);
        }
    }
    
    bool IsWalkable(int x, int y) const override {
        return isValidCoordinate(x, y) && !obstacles[x][y];
    }
    
    int GetWidth() const override { return width; }
    int GetHeight() const override { return height; }
    float GetCellSize() const override { return hexRadius * 2; }
    float GetHexRadius() const { return hexRadius; }
    Grid& GetLegacyGrid() override { return legacyGrid; }
    
    void SetPathMovementStrategy(std::unique_ptr<IPathMovement> strategy) override {}
    
    std::vector<Node*> GetNeighbors(Node* node) override {
        std::vector<Node*> neighbors;
        if (!node) return neighbors;
        
        auto cellNeighbors = getNeighbors({node->x, node->y});
        for (const auto& cell : cellNeighbors) {
            Node* n = legacyGrid.GetNode(cell.x, cell.y);
            if (n && IsWalkable(cell.x, cell.y)) {
                neighbors.push_back(n);
            }
        }
        return neighbors;
    }

    // Vizinhos em grid hexagonal (offset odd-q, pointy-top)
    std::list<Cell> getNeighbors(Cell cell) const override {
        std::list<Cell> neighbors;
        
        // Direções para odd-q offset (pointy-top)
        // Vizinhos para coluna par
        static const int evenColDirs[6][2] = {
            {0, -1},   // Norte
            {1, -1},   // Nordeste
            {1, 0},    // Sudeste
            {0, 1},    // Sul
            {-1, 0},   // Sudoeste
            {-1, -1}   // Noroeste
        };
        
        // Vizinhos para coluna ímpar
        static const int oddColDirs[6][2] = {
            {0, -1},   // Norte
            {1, 0},    // Nordeste
            {1, 1},    // Sudeste
            {0, 1},    // Sul
            {-1, 1},   // Sudoeste
            {-1, 0}    // Noroeste
        };
        
        const int (*dirs)[2] = (cell.x % 2 == 0) ? evenColDirs : oddColDirs;

        for (int i = 0; i < 6; ++i) {
            int newX = cell.x + dirs[i][0];
            int newY = cell.y + dirs[i][1];
            if (isValidCoordinate(newX, newY)) {
                neighbors.push_back({newX, newY});
            }
        }
        return neighbors;
    }

    bool isValidCoordinate(int x, int y) const override {
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    Cell getCell(int x, int y) const override {
        return {x, y};
    }
    
    Cell getClickedCell(Vector2 mousePos) const {
        return pixelToHex(mousePos.x, mousePos.y);
    }

    // A* Pathfinding específico para grid hexagonal
    std::vector<Vector2> FindPathHex(Vector2 start, Vector2 end) {
        Cell startCell = {(int)start.x, (int)start.y};
        Cell endCell = {(int)end.x, (int)end.y};
        
        if (!IsWalkable(startCell.x, startCell.y) || !IsWalkable(endCell.x, endCell.y)) {
            return {};
        }
        
        // Estrutura para nós do A*
        struct AStarNode {
            Cell cell;
            float gCost;
            float hCost;
            float fCost() const { return gCost + hCost; }
            Cell parent;
            bool hasParent;
        };
        
        auto cellHash = [](const Cell& c) { return std::hash<int>()(c.x) ^ (std::hash<int>()(c.y) << 16); };
        auto cellEqual = [](const Cell& a, const Cell& b) { return a.x == b.x && a.y == b.y; };
        
        std::unordered_map<Cell, AStarNode, decltype(cellHash), decltype(cellEqual)> 
            allNodes(10, cellHash, cellEqual);
        
        auto heuristic = [](Cell a, Cell b) -> float {
            // Distância de Manhattan adaptada para hex
            int dx = std::abs(a.x - b.x);
            int dy = std::abs(a.y - b.y);
            return (float)(dx + std::max(0, dy - dx / 2));
        };
        
        auto cmp = [&allNodes](const Cell& a, const Cell& b) {
            return allNodes[a].fCost() > allNodes[b].fCost();
        };
        
        std::priority_queue<Cell, std::vector<Cell>, decltype(cmp)> openSet(cmp);
        std::unordered_map<Cell, bool, decltype(cellHash), decltype(cellEqual)> 
            closedSet(10, cellHash, cellEqual);
        std::unordered_map<Cell, bool, decltype(cellHash), decltype(cellEqual)> 
            inOpenSet(10, cellHash, cellEqual);
        
        // Inicializa o nó inicial
        allNodes[startCell] = {startCell, 0, heuristic(startCell, endCell), {0, 0}, false};
        openSet.push(startCell);
        inOpenSet[startCell] = true;
        
        while (!openSet.empty()) {
            Cell current = openSet.top();
            openSet.pop();
            inOpenSet[current] = false;
            
            if (current == endCell) {
                // Reconstrói o caminho
                std::vector<Vector2> path;
                Cell node = endCell;
                while (!(node == startCell)) {
                    path.push_back({(float)node.x, (float)node.y});
                    if (!allNodes[node].hasParent) break;
                    node = allNodes[node].parent;
                }
                path.push_back({(float)startCell.x, (float)startCell.y});
                std::reverse(path.begin(), path.end());
                return path;
            }
            
            closedSet[current] = true;
            
            for (const Cell& neighbor : getNeighbors(current)) {
                if (closedSet[neighbor] || !IsWalkable(neighbor.x, neighbor.y)) {
                    continue;
                }
                
                float tentativeG = allNodes[current].gCost + 1.0f;
                
                if (allNodes.find(neighbor) == allNodes.end()) {
                    allNodes[neighbor] = {neighbor, tentativeG, heuristic(neighbor, endCell), current, true};
                    openSet.push(neighbor);
                    inOpenSet[neighbor] = true;
                } else if (tentativeG < allNodes[neighbor].gCost) {
                    allNodes[neighbor].gCost = tentativeG;
                    allNodes[neighbor].parent = current;
                    allNodes[neighbor].hasParent = true;
                    if (!inOpenSet[neighbor]) {
                        openSet.push(neighbor);
                        inOpenSet[neighbor] = true;
                    }
                }
            }
        }
        
        return {}; // Caminho não encontrado
    }
};

#endif // HEXAGONAL_GRID_ADAPTER_H
