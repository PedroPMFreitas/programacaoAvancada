#ifndef IAPP_FACTORY_H
#define IAPP_FACTORY_H

#include "src/Interfaces/IGridFactory.h"
#include "src/Interfaces/IAlgorithmFactory.h"
#include <memory>

class IAppFactory {
public:
    virtual ~IAppFactory() = default;
    virtual std::unique_ptr<IGridFactory> CreateGridFactory() = 0;
    virtual std::unique_ptr<IAlgorithmFactory> CreateAlgorithmFactory() = 0;
};

#endif // IAPP_FACTORY_H
