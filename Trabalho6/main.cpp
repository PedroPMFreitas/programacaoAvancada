#include "include/Core/Application.h"
#include "src/Factories/AppFactory.h"
#include <memory>

int main() {
    auto appFactory = std::make_unique<StandardAStarAppFactory>();
    auto app = std::make_unique<Application>(std::move(appFactory));
    app->Run();
    return 0;
}
