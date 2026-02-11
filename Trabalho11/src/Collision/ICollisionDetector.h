#ifndef ICOLLISION_DETECTOR_H
#define ICOLLISION_DETECTOR_H

#include "raylib.h"
#include <vector>
#include <string>

// Forward declaration
class GameAgent;

// Eventos de colisão
namespace CollisionEvents {
    const std::string COLLISION_WARNING = "collision_warning";     // Colisão iminente (raio maior)
    const std::string COLLISION_DETECTED = "collision_detected";   // Colisão real (raio exato)
    const std::string COLLISION_RESOLVED = "collision_resolved";   // Colisão resolvida
}

// Dados de colisão para notificação
struct CollisionData {
    GameAgent* agent1;
    GameAgent* agent2;
    float distance;
    Vector2 collisionPoint;
    bool isWarning;  // true = warning (raio maior), false = colisão real
};

// Interface Strategy para detecção de colisão
class ICollisionDetector {
public:
    virtual ~ICollisionDetector() = default;
    
    // Detecta colisões entre agentes
    virtual std::vector<CollisionData> detectCollisions(
        const std::vector<GameAgent*>& agents, 
        float warningRadius, 
        float collisionRadius) = 0;
    
    // Verifica colisão entre dois pontos
    virtual bool checkCollision(Vector2 pos1, Vector2 pos2, float radius1, float radius2) = 0;
    
    // Calcula distância entre dois pontos
    virtual float getDistance(Vector2 pos1, Vector2 pos2) = 0;
};

#endif // ICOLLISION_DETECTOR_H
