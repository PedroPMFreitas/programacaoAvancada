#ifndef GAME_AGENT_H
#define GAME_AGENT_H

#include "src/Interfaces/IObserver.h"
#include "raylib.h"
#include <vector>
#include <algorithm>
#include <string>

// Eventos do agente
namespace AgentEvents {
    const std::string AGENT_SPAWNED = "agent_spawned";
    const std::string AGENT_DIED = "agent_died";
    const std::string AGENT_REACHED_TARGET = "agent_reached_target";
    const std::string AGENT_MOVED = "agent_moved";
    const std::string AGENT_PATH_BLOCKED = "agent_path_blocked";
    const std::string AGENT_HEALTH_CHANGED = "agent_health_changed";
}

// Forward declarations
class Grid;

// Agente com suporte ao padrão Observer (Subject)
class GameAgent : public ISubject {
private:
    std::vector<IObserver*> observers;
    
    Vector2 position;
    Vector2 spawnPosition;
    Vector2 target;
    std::vector<Vector2> path;
    int currentPathIndex;
    bool hasPath;
    bool reachedTarget;
    Color color;
    float speed;
    int health;
    int maxHealth;
    bool alive;

public:
    GameAgent(Vector2 start, Vector2 targetPos, int hp = 100) 
        : position(start), spawnPosition(start), target(targetPos),
          currentPathIndex(0), hasPath(false), reachedTarget(false),
          color(GetRandomAgentColor()), speed(2.0f),
          health(hp), maxHealth(hp), alive(true) {}

    // Implementação de ISubject
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

    // Getters e Setters
    Vector2 getPosition() const { return position; }
    Vector2 getSpawnPosition() const { return spawnPosition; }
    Vector2 getTarget() const { return target; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    bool isAlive() const { return alive; }
    float getSpeed() const { return speed; }
    Color getColor() const { return color; }
    bool getHasPath() const { return hasPath; }
    bool hasReachedTarget() const { return reachedTarget; }
    const std::vector<Vector2>& getPath() const { return path; }
    int getCurrentPathIndex() const { return currentPathIndex; }

    void setPosition(Vector2 pos) {
        position = pos;
    }

    void setTarget(Vector2 t) { target = t; }
    void setPath(const std::vector<Vector2>& p) { 
        path = p; 
        hasPath = !path.empty();
        currentPathIndex = 0;
    }
    void setHasPath(bool hp) { hasPath = hp; }
    void setCurrentPathIndex(int idx) { currentPathIndex = idx; }
    void setSpeed(float s) { speed = s; }

    // Dano e morte
    void takeDamage(int damage) {
        if (!alive) return;
        
        int oldHealth = health;
        health = std::max(0, health - damage);
        notifyObservers(AgentEvents::AGENT_HEALTH_CHANGED, &oldHealth);
        
        if (health <= 0) {
            die();
        }
    }

    void die() {
        if (!alive) return;
        alive = false;
        notifyObservers(AgentEvents::AGENT_DIED, this);
    }

    // Respawn no ponto de origem
    void respawn() {
        position = spawnPosition;
        health = maxHealth;
        alive = true;
        hasPath = false;
        reachedTarget = false;
        currentPathIndex = 0;
        path.clear();
        notifyObservers(AgentEvents::AGENT_SPAWNED, this);
    }

    // Chegou ao destino
    void reachTarget() {
        if (!reachedTarget) {
            reachedTarget = true;
            notifyObservers(AgentEvents::AGENT_REACHED_TARGET, this);
        }
    }

    void pathBlocked() {
        notifyObservers(AgentEvents::AGENT_PATH_BLOCKED, this);
    }

    // Atualização do agente
    void update(Grid& grid, float deltaTime);
    void draw(float cellSize);

private:
    static Color GetRandomAgentColor() {
        Color colors[] = {BLUE, PURPLE, ORANGE, PINK, DARKBLUE, DARKPURPLE, SKYBLUE, LIME};
        return colors[GetRandomValue(0, 7)];
    }
};

#endif // GAME_AGENT_H
