#include "SpawnAgentCommand.h"
#include "src/Observer/GameAgentManager.h"

void SpawnAgentCommand::execute() {
    if (manager && !executed) {
        createdAgent = manager->addAgent(spawnPosition, targetPosition);
        executed = true;
    }
}

void SpawnAgentCommand::undo() {
    if (executed && manager && createdAgent) {
        manager->removeAgent(createdAgent);
        createdAgent = nullptr;
        executed = false;
    }
}
