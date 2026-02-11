#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include "src/Interfaces/ICommand.h"
#include <vector>
#include <memory>
#include <deque>
#include <chrono>
#include <algorithm>

// Gerenciador de comandos - mantém histórico para undo
class CommandManager {
private:
    static CommandManager* instance;
    std::deque<std::shared_ptr<ICommand>> executedCommands;
    std::deque<std::shared_ptr<ICommand>> undoneCommands;
    std::vector<std::shared_ptr<ICommand>> pendingCommands; // Comandos agendados
    size_t maxHistorySize;

    CommandManager() : maxHistorySize(100) {}

public:
    CommandManager(const CommandManager&) = delete;
    CommandManager& operator=(const CommandManager&) = delete;

    static CommandManager* getInstance() {
        if (!instance) {
            instance = new CommandManager();
        }
        return instance;
    }

    // Executa um comando imediatamente
    void executeCommand(std::shared_ptr<ICommand> command) {
        command->execute();
        executedCommands.push_back(command);
        undoneCommands.clear(); // Limpa histórico de redo ao executar novo comando
        
        // Mantém o tamanho do histórico limitado
        while (executedCommands.size() > maxHistorySize) {
            executedCommands.pop_front();
        }
    }

    // Agenda um comando para execução futura
    void scheduleCommand(std::shared_ptr<ICommand> command, 
                         std::chrono::steady_clock::time_point time) {
        command->setScheduledTime(time);
        pendingCommands.push_back(command);
        
        // Ordena por tempo de execução
        std::sort(pendingCommands.begin(), pendingCommands.end(),
            [](const auto& a, const auto& b) {
                return a->getScheduledTime() < b->getScheduledTime();
            });
    }

    // Processa comandos pendentes que já podem ser executados
    void processPendingCommands() {
        auto now = std::chrono::steady_clock::now();
        
        while (!pendingCommands.empty() && 
               pendingCommands.front()->getScheduledTime() <= now) {
            executeCommand(pendingCommands.front());
            pendingCommands.erase(pendingCommands.begin());
        }
    }

    // Desfaz o último comando
    bool undo() {
        if (executedCommands.empty()) {
            return false;
        }
        
        auto command = executedCommands.back();
        executedCommands.pop_back();
        command->undo();
        undoneCommands.push_back(command);
        return true;
    }

    // Refaz o último comando desfeito
    bool redo() {
        if (undoneCommands.empty()) {
            return false;
        }
        
        auto command = undoneCommands.back();
        undoneCommands.pop_back();
        command->execute();
        executedCommands.push_back(command);
        return true;
    }

    // Limpa todo o histórico
    void clearHistory() {
        executedCommands.clear();
        undoneCommands.clear();
        pendingCommands.clear();
    }

    // Retorna o número de comandos no histórico
    size_t getHistorySize() const { return executedCommands.size(); }
    size_t getUndoStackSize() const { return undoneCommands.size(); }
    size_t getPendingCount() const { return pendingCommands.size(); }
    
    void setMaxHistorySize(size_t size) { maxHistorySize = size; }
};

inline CommandManager* CommandManager::instance = nullptr;

#endif // COMMAND_MANAGER_H
