#ifndef SET_OBSTACLE_COMMAND_H
#define SET_OBSTACLE_COMMAND_H

#include "BaseCommand.h"
#include "src/Interfaces/IGridAdapter.h"

// Command para definir/remover obstáculos no grid
class SetObstacleCommand : public BaseCommand {
private:
    IGridAdapter* grid;
    int x, y;
    bool newState;
    bool previousState;
    bool executed;

public:
    SetObstacleCommand(IGridAdapter* g, int px, int py, bool state) 
        : grid(g), x(px), y(py), newState(state), previousState(false), executed(false) {}
    
    void execute() override {
        if (grid && grid->isValidCoordinate(x, y)) {
            previousState = !grid->IsWalkable(x, y);
            grid->SetObstacle(x, y, newState);
            executed = true;
        }
    }
    
    void undo() override {
        if (executed && grid && grid->isValidCoordinate(x, y)) {
            grid->SetObstacle(x, y, previousState);
        }
    }
};

#endif // SET_OBSTACLE_COMMAND_H
