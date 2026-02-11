#ifndef IALGORITHM_H
#define IALGORITHM_H

#include "Trabalho9_Legacy.h"
#include "src/Interfaces/IGrid.h"
#include <vector>

class IAlgorithm {
public:
    virtual ~IAlgorithm() = default;
    virtual std::vector<Vector2> FindPath(IGrid* grid, Vector2 start, Vector2 end) = 0;
    virtual double GetLastExecutionTime() const = 0;
};

#endif // IALGORITHM_H
