#ifndef IINIT_HANDLER_H
#define IINIT_HANDLER_H

#include <memory>
#include <string>

// Chain of Responsibility - Interface para handlers de inicialização
class IInitHandler {
public:
    virtual ~IInitHandler() = default;
    
    // Define o próximo handler na cadeia
    virtual void setNext(std::shared_ptr<IInitHandler> next) = 0;
    
    // Processa a inicialização e passa para o próximo handler
    virtual bool handle() = 0;
    
    // Retorna o nome do handler para logging
    virtual std::string getName() const = 0;
};

#endif // IINIT_HANDLER_H
