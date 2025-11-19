#ifndef LOCATION_PROVIDER_H
#define LOCATION_PROVIDER_H

#include "include/Interfaces/ILocation.h"

class ManualLocationProvider : public ILocationProvider {
public:
    Vector2 GetStartPosition(IGrid* grid) override {
        return {-1, -1}; 
    }
    Vector2 GetEndPosition(IGrid* grid) override {
        return {-1, -1};
    }
};

class RandomLocationProvider : public ILocationProvider {
public:
    Vector2 GetStartPosition(IGrid* grid) override {
        Vector2 start;
        do {
            start = {
                (float)GetRandomValue(0, grid->GetWidth() - 1), 
                (float)GetRandomValue(0, grid->GetHeight() - 1)
            };
        } while (!grid->IsWalkable((int)start.x, (int)start.y));
        return start;
    }

    Vector2 GetEndPosition(IGrid* grid) override {
        Vector2 end;
        do {
            end = {
                (float)GetRandomValue(0, grid->GetWidth() - 1),
                (float)GetRandomValue(0, grid->GetHeight() - 1)
            };
        } while (!grid->IsWalkable((int)end.x, (int)end.y));
        return end;
    }
};

#endif // LOCATION_PROVIDER_H
