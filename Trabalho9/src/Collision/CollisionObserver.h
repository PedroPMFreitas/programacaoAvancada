#ifndef COLLISION_OBSERVER_H
#define COLLISION_OBSERVER_H

#include "src/Interfaces/IObserver.h"
#include "ICollisionDetector.h"
#include <iostream>

// Observer que reage a eventos de colisão
class CollisionObserver : public IObserver {
private:
    bool logToConsole;
    
public:
    CollisionObserver(bool log = false) : logToConsole(log) {}
    
    void setLogToConsole(bool log) { logToConsole = log; }
    
    void onNotify(const std::string& event, void* data) override {
        if (!data) return;
        
        CollisionData* collision = static_cast<CollisionData*>(data);
        
        if (event == CollisionEvents::COLLISION_WARNING) {
            if (logToConsole) {
                std::cout << "[Colisão] AVISO: Agentes se aproximando! Distância: " 
                          << collision->distance << std::endl;
            }
            // Pode-se adicionar lógica para desvio preventivo aqui
        }
        else if (event == CollisionEvents::COLLISION_DETECTED) {
            if (logToConsole) {
                std::cout << "[Colisão] DETECTADA! Distância: " 
                          << collision->distance << std::endl;
            }
            // Lógica de resolução de colisão
            handleCollision(collision);
        }
        else if (event == CollisionEvents::COLLISION_RESOLVED) {
            if (logToConsole) {
                std::cout << "[Colisão] Resolvida." << std::endl;
            }
        }
    }
    
private:
    void handleCollision(CollisionData* collision) {
        // Empurra os agentes para direções opostas
        if (collision->agent1 && collision->agent2) {
            Vector2 pos1 = collision->agent1->getPosition();
            Vector2 pos2 = collision->agent2->getPosition();
            
            // Direção de separação
            float dx = pos2.x - pos1.x;
            float dy = pos2.y - pos1.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist > 0.001f) {
                // Normaliza
                dx /= dist;
                dy /= dist;
                
                // Empurra cada agente 5 pixels na direção oposta
                float pushForce = 5.0f;
                collision->agent1->setPosition({pos1.x - dx * pushForce, pos1.y - dy * pushForce});
                collision->agent2->setPosition({pos2.x + dx * pushForce, pos2.y + dy * pushForce});
            }
        }
    }
};

#endif // COLLISION_OBSERVER_H
