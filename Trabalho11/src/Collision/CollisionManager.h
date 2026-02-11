#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H

#include "ICollisionDetector.h"
#include "CircleCollisionDetector.h"
#include "CollisionObserver.h"
#include "src/Interfaces/IObserver.h"
#include "src/Observer/GameAgent.h"
#include <memory>
#include <vector>
#include <algorithm>

// Singleton - Gerenciador de colisões
// Usa Strategy para o detector e Observer para notificar eventos
class CollisionManager : public ISubject {
private:
    static CollisionManager* instance;
    
    std::unique_ptr<ICollisionDetector> detector;
    std::vector<IObserver*> observers;
    
    float warningRadius = 25.0f;    // Raio para detecção de colisão iminente
    float collisionRadius = 10.0f;  // Raio para colisão real
    
    bool enabled = true;
    bool showWarningZone = true;
    bool showCollisionZone = true;
    
    CollisionManager() {
        // Usa CircleCollisionDetector por padrão (Strategy)
        detector = std::make_unique<CircleCollisionDetector>();
    }

public:
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;
    
    static CollisionManager* getInstance() {
        if (!instance) {
            instance = new CollisionManager();
        }
        return instance;
    }
    
    // Permite trocar o algoritmo de detecção (Strategy pattern)
    void setDetector(std::unique_ptr<ICollisionDetector> newDetector) {
        detector = std::move(newDetector);
    }
    
    // Observer pattern
    void addObserver(IObserver* observer) override {
        observers.push_back(observer);
    }
    
    void removeObserver(IObserver* observer) override {
        observers.erase(
            std::remove(observers.begin(), observers.end(), observer),
            observers.end()
        );
    }
    
    void notifyObservers(const std::string& event, void* data = nullptr) override {
        for (auto* observer : observers) {
            observer->onNotify(event, data);
        }
    }
    
    // Configuração dos raios
    void setWarningRadius(float radius) { warningRadius = radius; }
    void setCollisionRadius(float radius) { collisionRadius = radius; }
    float getWarningRadius() const { return warningRadius; }
    float getCollisionRadius() const { return collisionRadius; }
    
    // Liga/desliga sistema
    void setEnabled(bool e) { enabled = e; }
    bool isEnabled() const { return enabled; }
    
    // Visualização
    void setShowWarningZone(bool show) { showWarningZone = show; }
    void setShowCollisionZone(bool show) { showCollisionZone = show; }
    bool getShowWarningZone() const { return showWarningZone; }
    bool getShowCollisionZone() const { return showCollisionZone; }
    
    // Processa colisões para uma lista de agentes
    void processCollisions(const std::vector<GameAgent*>& agents) {
        if (!enabled || !detector) return;
        
        auto collisions = detector->detectCollisions(agents, warningRadius, collisionRadius);
        
        for (auto& collision : collisions) {
            if (collision.isWarning) {
                notifyObservers(CollisionEvents::COLLISION_WARNING, &collision);
            } else {
                notifyObservers(CollisionEvents::COLLISION_DETECTED, &collision);
            }
        }
    }
    
    // Desenha as zonas de colisão para debug
    void drawCollisionZones(const std::vector<GameAgent*>& agents) {
        if (!enabled) return;
        
        for (auto* agent : agents) {
            if (!agent->isAlive()) continue;
            
            Vector2 pos = agent->getPosition();
            
            // Zona de aviso (amarelo transparente)
            if (showWarningZone) {
                DrawCircleLines(pos.x, pos.y, warningRadius, Fade(YELLOW, 0.5f));
            }
            
            // Zona de colisão (vermelho transparente)
            if (showCollisionZone) {
                DrawCircleLines(pos.x, pos.y, collisionRadius, Fade(RED, 0.7f));
            }
        }
    }
};

inline CollisionManager* CollisionManager::instance = nullptr;

#endif // COLLISION_MANAGER_H
