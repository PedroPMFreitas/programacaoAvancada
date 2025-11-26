#ifndef IALGORITHM_FACTORY_H
#define IALGORITHM_FACTORY_H

#include "include/Interfaces/IAlgorithm.h"
#include "include/Interfaces/ILocation.h"
#include <memory>

class IAlgorithmFactory {
public:
    virtual ~IAlgorithmFactory() = default;
    virtual std::unique_ptr<IAlgorithm> CreateAlgorithm() = 0;
    virtual std::unique_ptr<ILocationProvider> CreateLocationProvider() = 0;
};

#endif // IALGORITHM_FACTORY_H
