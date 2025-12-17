#ifndef WINDOW_INIT_HANDLER_H
#define WINDOW_INIT_HANDLER_H

#include "BaseInitHandler.h"
#include "raylib.h"

// Handler para inicialização da janela
class WindowInitHandler : public BaseInitHandler {
private:
    int width;
    int height;
    const char* title;

public:
    WindowInitHandler(int w, int h, const char* t) 
        : width(w), height(h), title(t) {}
    
    std::string getName() const override {
        return "WindowInitHandler";
    }

protected:
    bool doHandle() override {
        InitWindow(width, height, title);
        if (!IsWindowReady()) {
            return false;
        }
        SetTargetFPS(60);
        return true;
    }
};

#endif // WINDOW_INIT_HANDLER_H
