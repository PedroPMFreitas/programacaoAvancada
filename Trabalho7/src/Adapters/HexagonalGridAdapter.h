#ifndef HEXAGONAL_GRID_ADAPTER_H
#define HEXAGONAL_GRID_ADAPTER_H

#include "../../include/Interfaces/IGridAdapter.h"
#include "../../include/Core/Cell.h"
#include "raylib.h"
#include <vector>
#include <list>
#include <cmath>

class HexagonalGridAdapter : public IGridAdapter {
private:
    int q_max; // Max for q coordinate
    int r_max; // Max for r coordinate
    float hexSize = 20.0f;
    Grid legacyGrid; // For compatibility with the old system

public:
    HexagonalGridAdapter(int q_max, int r_max) : q_max(q_max), r_max(r_max), legacyGrid(q_max*2, r_max*2, hexSize) {}

    // Helper to convert axial coordinates to pixel coordinates
    Vector2 axialToPixel(int q, int r) const {
        float x = hexSize * (3.0/2.0 * q);
        float y = hexSize * (sqrt(3.0)/2.0 * q + sqrt(3.0) * r);
        return {x, y};
    }

    void Draw() override {
        for (int q = -q_max; q <= q_max; q++) {
            for (int r = -r_max; r <= r_max; r++) {
                if (isValidCoordinate(q, r)) {
                    Vector2 center = axialToPixel(q, r);
                    DrawPolyLines(center, 6, hexSize, 0, LIGHTGRAY);
                }
            }
        }
    }
    
    void SetObstacle(int x, int y, bool isObstacle) override { /* Dummy implementation */ }
    bool IsWalkable(int x, int y) const override { return true; /* Dummy implementation */ }
    int GetWidth() const override { return q_max * 2; }
    int GetHeight() const override { return r_max * 2; }
    float GetCellSize() const override { return hexSize; }
    Grid& GetLegacyGrid() override { return legacyGrid; }
    void SetPathMovementStrategy(std::unique_ptr<IPathMovement> strategy) override { /* Dummy implementation */ }
    std::vector<Node*> GetNeighbors(Node* node) override { return {}; /* Dummy implementation */ }

    std::list<Cell> getNeighbors(Cell cell) const override {
        std::list<Cell> neighbors;
        int directions[6][2] = {
            {1, 0}, {0, 1}, {-1, 1},
            {-1, 0}, {0, -1}, {1, -1}
        };

        for (int i = 0; i < 6; ++i) {
            int newQ = cell.x + directions[i][0];
            int newR = cell.y + directions[i][1];
            if (isValidCoordinate(newQ, newR)) {
                neighbors.push_back({newQ, newR});
            }
        }
        return neighbors;
    }

    bool isValidCoordinate(int q, int r) const override {
        // This condition is for a parallelogram-shaped map.
        // A real hexagonal map would have a more complex shape (e.g., a hexagon).
        return q >= -q_max && q <= q_max && r >= -r_max && r <= r_max;
    }

    Cell getCell(int q, int r) const override {
        return {q, r};
    }
};

#endif // HEXAGONAL_GRID_ADAPTER_H
