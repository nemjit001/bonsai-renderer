#include "bonsai/engine.hpp"

#include "bonsai/application.hpp"

Engine::Engine() = default;
Engine::~Engine() = default;

void Engine::run()
{
    Application* app = create_application();
    destroy_application(app);
}
