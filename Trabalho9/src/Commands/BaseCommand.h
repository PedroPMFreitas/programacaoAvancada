#ifndef BASE_COMMAND_H
#define BASE_COMMAND_H

#include "src/Interfaces/ICommand.h"

// Command Pattern - Comando base abstrato
class BaseCommand : public ICommand {
protected:
    std::chrono::steady_clock::time_point scheduledTime;

public:
    BaseCommand() : scheduledTime(std::chrono::steady_clock::now()) {}
    
    std::chrono::steady_clock::time_point getScheduledTime() const override {
        return scheduledTime;
    }
    
    void setScheduledTime(std::chrono::steady_clock::time_point time) override {
        scheduledTime = time;
    }
};

#endif // BASE_COMMAND_H
