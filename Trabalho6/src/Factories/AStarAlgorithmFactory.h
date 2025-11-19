#ifndef CONCRETE_ASTAR_ALGORITHM_FACTORY_H
#define CONCRETE_ASTAR_ALGORITHM_FACTORY_H

#include "include/Factories/IAlgorithmFactory.h"
#include "src/Adapters/AStarAdapter.h"
#include "src/Adapters/LocationProvider.h"

class AStarAlgorithmFactory : public IAlgorithmFactory {
public:
    std::unique_ptr<IAlgorithm> CreateAlgorithm() override {
        return std::make_unique<AStarAdapter>();
    }

    std::unique_ptr<ILocationProvider> CreateLocationProvider() override {
        return std::make_unique<ManualLocationProvider>();
    }
};

#endif // CONCRETE_ASTAR_ALGORITHM_FACTORY_H
