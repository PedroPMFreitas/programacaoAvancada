#ifndef BASE_INIT_HANDLER_H
#define BASE_INIT_HANDLER_H

#include "src/Interfaces/IInitHandler.h"
#include <iostream>

// Chain of Responsibility - Handler base abstrato
class BaseInitHandler : public IInitHandler {
protected:
    std::shared_ptr<IInitHandler> nextHandler;

public:
    void setNext(std::shared_ptr<IInitHandler> next) override {
        nextHandler = next;
    }
    
    bool handle() override {
        std::cout << "[Init] " << getName() << " executando..." << std::endl;
        
        bool success = doHandle();
        
        if (success) {
            std::cout << "[Init] " << getName() << " concluído com sucesso." << std::endl;
            if (nextHandler) {
                return nextHandler->handle();
            }
            return true;
        } else {
            std::cout << "[Init] " << getName() << " FALHOU!" << std::endl;
            return false;
        }
    }
    
protected:
    // Template method para implementação específica
    virtual bool doHandle() = 0;
};

#endif // BASE_INIT_HANDLER_H
