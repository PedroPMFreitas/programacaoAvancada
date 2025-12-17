#include "GameAgent.h"
#include "Trabalho9_Legacy.h"
#include <cmath>

void GameAgent::update(Grid& grid, float deltaTime) {
    if (!alive) return;
    
    if (!hasPath) {
        // Tenta encontrar caminho
        Vector2 gridPos = {position.x / grid.GetCellSize(), position.y / grid.GetCellSize()};
        auto newPath = Pathfinder::FindPath(grid, gridPos, target, "random");
        
        if (newPath.empty()) {
            pathBlocked();
            return;
        }
        
        setPath(newPath);
        return;
    }
    
    if (currentPathIndex < (int)path.size()) {
        Vector2 nextCell = path[currentPathIndex];
        Vector2 targetWorldPos = {
            nextCell.x * grid.GetCellSize() + grid.GetCellSize() / 2,
            nextCell.y * grid.GetCellSize() + grid.GetCellSize() / 2
        };
        
        Vector2 direction = {targetWorldPos.x - position.x, targetWorldPos.y - position.y};
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        
        if (distance < 5.0f) {
            currentPathIndex++;
        } else {
            direction.x /= distance;
            direction.y /= distance;
            
            Vector2 oldPos = position;
            position.x += direction.x * speed * deltaTime * 60.0f;
            position.y += direction.y * speed * deltaTime * 60.0f;
            notifyObservers(AgentEvents::AGENT_MOVED, &oldPos);
        }
    } else {
        hasPath = false;
        reachTarget();
    }
}

void GameAgent::draw(float cellSize) {
    if (!alive) return;
    
    // Desenha o agente
    DrawCircle(position.x, position.y, cellSize / 3, color);
    
    // Desenha barra de vida
    float healthBarWidth = cellSize;
    float healthBarHeight = 4;
    float healthPercent = (float)health / maxHealth;
    
    DrawRectangle(
        position.x - healthBarWidth / 2, 
        position.y - cellSize / 2 - 8, 
        healthBarWidth, 
        healthBarHeight, 
        DARKGRAY
    );
    DrawRectangle(
        position.x - healthBarWidth / 2, 
        position.y - cellSize / 2 - 8, 
        healthBarWidth * healthPercent, 
        healthBarHeight, 
        (healthPercent > 0.5f) ? GREEN : (healthPercent > 0.25f) ? YELLOW : RED
    );
}
