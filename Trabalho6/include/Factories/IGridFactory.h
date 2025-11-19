#ifndef IGRID_FACTORY_H
#define IGRID_FACTORY_H

#include "include/Interfaces/IGrid.h"
#include "include/Interfaces/IPathMovement.h"
#include <memory>

class IGridFactory {
public:
    virtual ~IGridFactory() = default;
    virtual std::unique_ptr<IGrid> CreateGrid(int width, int height, float cellSize) = 0;
    virtual std::unique_ptr<IPathMovement> CreatePathMovement() = 0;
};

#endif // IGRID_FACTORY_H
