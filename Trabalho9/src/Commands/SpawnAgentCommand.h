#ifndef SPAWN_AGENT_COMMAND_H
#define SPAWN_AGENT_COMMAND_H

#include "BaseCommand.h"
#include "raylib.h"

// Forward declarations
class GameAgentManager;
class GameAgent;

// Command para criar um agente
class SpawnAgentCommand : public BaseCommand {
private:
    GameAgentManager* manager;
    Vector2 spawnPosition;
    Vector2 targetPosition;
    GameAgent* createdAgent;
    bool executed;

public:
    SpawnAgentCommand(GameAgentManager* mgr, Vector2 spawn, Vector2 target) 
        : manager(mgr), spawnPosition(spawn), targetPosition(target), 
          createdAgent(nullptr), executed(false) {}
    
    void execute() override;
    void undo() override;
    
    GameAgent* getCreatedAgent() const { return createdAgent; }
};

#endif // SPAWN_AGENT_COMMAND_H
