#include "MoveCommand.h"
#include "src/Observer/GameAgent.h"

void MoveCommand::execute() {
    if (agent && agent->isAlive()) {
        previousPosition = agent->getPosition();
        agent->setPosition(newPosition);
        executed = true;
    }
}

void MoveCommand::undo() {
    if (executed && agent) {
        agent->setPosition(previousPosition);
    }
}
