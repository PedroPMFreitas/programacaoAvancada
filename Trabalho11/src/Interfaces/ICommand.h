#ifndef ICOMMAND_H
#define ICOMMAND_H

#include <chrono>

// Command Pattern - Interface para comandos
class ICommand {
public:
    virtual ~ICommand() = default;
    
    // Executa o comando
    virtual void execute() = 0;
    
    // Desfaz o comando
    virtual void undo() = 0;
    
    // Retorna o timestamp de quando o comando deve ser executado
    virtual std::chrono::steady_clock::time_point getScheduledTime() const = 0;
    
    // Define o timestamp de execução
    virtual void setScheduledTime(std::chrono::steady_clock::time_point time) = 0;
};

#endif // ICOMMAND_H
