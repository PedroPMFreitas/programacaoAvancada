#ifndef IOBSERVER_H
#define IOBSERVER_H

#include <string>

// Forward declarations
class Agent;

// Observer Pattern - Interface para observadores
class IObserver {
public:
    virtual ~IObserver() = default;
    
    // Chamado quando um evento ocorre
    virtual void onNotify(const std::string& event, void* data) = 0;
};

// Subject/Observable interface
class ISubject {
public:
    virtual ~ISubject() = default;
    
    virtual void addObserver(IObserver* observer) = 0;
    virtual void removeObserver(IObserver* observer) = 0;
    virtual void notifyObservers(const std::string& event, void* data = nullptr) = 0;
};

#endif // IOBSERVER_H
