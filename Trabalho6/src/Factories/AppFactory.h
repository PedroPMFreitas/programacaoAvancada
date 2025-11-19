#ifndef CONCRETE_APP_FACTORY_H
#define CONCRETE_APP_FACTORY_H

#include "include/Factories/IAppFactory.h"
#include "src/Factories/GridFactory.h"
#include "src/Factories/AStarAlgorithmFactory.h"

class StandardAStarAppFactory : public IAppFactory {
public:
    std::unique_ptr<IGridFactory> CreateGridFactory() override {
        return std::make_unique<StandardGridFactory>();
    }

    std::unique_ptr<IAlgorithmFactory> CreateAlgorithmFactory() override {
        return std::make_unique<AStarAlgorithmFactory>();
    }
};

#endif // CONCRETE_APP_FACTORY_H
