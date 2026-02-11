#ifndef GRID_INIT_HANDLER_H
#define GRID_INIT_HANDLER_H

#include "BaseInitHandler.h"
#include "src/GridManager.h"
#include "src/Interfaces/IAppFactory.h"
#include "Core/GridType.h"

// Handler para inicialização do grid
class GridInitHandler : public BaseInitHandler {
private:
    std::unique_ptr<IAppFactory> factory;
    GridType gridType;
    int width;
    int height;

public:
    GridInitHandler(std::unique_ptr<IAppFactory> f, GridType type, int w, int h) 
        : factory(std::move(f)), gridType(type), width(w), height(h) {}
    
    std::string getName() const override {
        return "GridInitHandler";
    }

protected:
    bool doHandle() override {
        try {
            GridManager::getInstance()->init(std::move(factory), gridType, width, height);
            return GridManager::getInstance()->getGrid() != nullptr;
        } catch (...) {
            return false;
        }
    }
};

#endif // GRID_INIT_HANDLER_H
