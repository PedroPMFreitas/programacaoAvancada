#ifndef ILOCATION_H
#define ILOCATION_H

#include "Trabalho9_Legacy.h"
#include "src/Interfaces/IGrid.h"

class ILocationProvider {
public:
    virtual ~ILocationProvider() = default;
    virtual Vector2 GetStartPosition(IGrid* grid) = 0;
    virtual Vector2 GetEndPosition(IGrid* grid) = 0;
};

#endif // ILOCATION_H
