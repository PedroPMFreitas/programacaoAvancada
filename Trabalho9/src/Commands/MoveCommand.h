#ifndef MOVE_COMMAND_H
#define MOVE_COMMAND_H

#include "BaseCommand.h"
#include "raylib.h"
#include <iostream>

// Forward declaration
class GameAgent;

// Command para movimentação de agente
class MoveCommand : public BaseCommand {
private:
    GameAgent* agent;
    Vector2 previousPosition;
    Vector2 newPosition;
    bool executed;

public:
    MoveCommand(GameAgent* a, Vector2 newPos) 
        : agent(a), newPosition(newPos), executed(false) {}
    
    void execute() override;
    void undo() override;
    
    Vector2 getNewPosition() const { return newPosition; }
    Vector2 getPreviousPosition() const { return previousPosition; }
    bool wasExecuted() const { return executed; }
};

#endif // MOVE_COMMAND_H
