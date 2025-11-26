#ifndef RECTANGULAR_GRID_ADAPTER_H
#define RECTANGULAR_GRID_ADAPTER_H

#include "../../include/Interfaces/IGridAdapter.h"
#include "../../include/Core/Cell.h"
#include "raylib.h"
#include <vector>
#include <list>

class RectangularGridAdapter : public IGridAdapter {
private:
    int width;
    int height;
    float cellSize = 20.0f;
    std::vector<std::vector<bool>> obstacles;
    Grid legacyGrid; // For compatibility with the old system

public:
    RectangularGridAdapter(int width, int height) : width(width), height(height), legacyGrid(width, height, cellSize) {
        obstacles.resize(width, std::vector<bool>(height, false));
    }

    void Draw() override {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                if (obstacles[i][j]) {
                    DrawRectangle(i * cellSize, j * cellSize, cellSize, cellSize, BLACK);
                } else {
                    DrawRectangleLines(i * cellSize, j * cellSize, cellSize, cellSize, LIGHTGRAY);
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
    float GetCellSize() const override { return cellSize; }
    Grid& GetLegacyGrid() override { return legacyGrid; }

    void SetPathMovementStrategy(std::unique_ptr<IPathMovement> strategy) override { /* Dummy implementation */ }
    std::vector<Node*> GetNeighbors(Node* node) override { return {}; /* Dummy implementation */ }

    std::list<Cell> getNeighbors(Cell cell) const override {
        std::list<Cell> neighbors;
        int dx[] = {-1, 1, 0, 0}; // Left, Right
        int dy[] = {0, 0, -1, 1}; // Up, Down

        for (int i = 0; i < 4; ++i) {
            int newX = cell.x + dx[i];
            int newY = cell.y + dy[i];
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
};

#endif // RECTANGULAR_GRID_ADAPTER_H
