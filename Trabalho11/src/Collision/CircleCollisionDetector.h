#ifndef CIRCLE_COLLISION_DETECTOR_H
#define CIRCLE_COLLISION_DETECTOR_H

#include "ICollisionDetector.h"
#include "src/Observer/GameAgent.h"
#include <cmath>

// Implementação concreta - detecção de colisão circular
class CircleCollisionDetector : public ICollisionDetector {
public:
    std::vector<CollisionData> detectCollisions(
        const std::vector<GameAgent*>& agents, 
        float warningRadius, 
        float collisionRadius) override 
    {
        std::vector<CollisionData> collisions;
        
        for (size_t i = 0; i < agents.size(); i++) {
            if (!agents[i]->isAlive()) continue;
            
            for (size_t j = i + 1; j < agents.size(); j++) {
                if (!agents[j]->isAlive()) continue;
                
                Vector2 pos1 = agents[i]->getPosition();
                Vector2 pos2 = agents[j]->getPosition();
                float dist = getDistance(pos1, pos2);
                
                // Verifica colisão real primeiro
                if (dist < collisionRadius * 2) {
                    CollisionData data;
                    data.agent1 = agents[i];
                    data.agent2 = agents[j];
                    data.distance = dist;
                    data.collisionPoint = {(pos1.x + pos2.x) / 2, (pos1.y + pos2.y) / 2};
                    data.isWarning = false;
                    collisions.push_back(data);
                }
                // Verifica warning (colisão iminente)
                else if (dist < warningRadius * 2) {
                    CollisionData data;
                    data.agent1 = agents[i];
                    data.agent2 = agents[j];
                    data.distance = dist;
                    data.collisionPoint = {(pos1.x + pos2.x) / 2, (pos1.y + pos2.y) / 2};
                    data.isWarning = true;
                    collisions.push_back(data);
                }
            }
        }
        
        return collisions;
    }
    
    bool checkCollision(Vector2 pos1, Vector2 pos2, float radius1, float radius2) override {
        float dist = getDistance(pos1, pos2);
        return dist < (radius1 + radius2);
    }
    
    float getDistance(Vector2 pos1, Vector2 pos2) override {
        float dx = pos2.x - pos1.x;
        float dy = pos2.y - pos1.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

#endif // CIRCLE_COLLISION_DETECTOR_H
