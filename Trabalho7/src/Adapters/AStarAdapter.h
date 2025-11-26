#ifndef ASTAR_ADAPTER_H
#define ASTAR_ADAPTER_H

#include "Interfaces/IAlgorithm.h"
#include "../../Trabalho6_Legacy.h"

class AStarAdapter : public IAlgorithm {
public:
    std::vector<Vector2> FindPath(IGrid* grid, Vector2 start, Vector2 end) override {
        Grid& legacyGrid = grid->GetLegacyGrid();
        return Pathfinder::FindPath(legacyGrid, start, end, "AStar_Adapter");
    }

    double GetLastExecutionTime() const override {
        return Pathfinder::GetLastExecutionTime();
    }
};

#endif // ASTAR_ADAPTER_H
