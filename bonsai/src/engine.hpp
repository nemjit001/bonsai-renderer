#pragma once
#ifndef BONSAI_RENDERER_ENGINE_HPP
#define BONSAI_RENDERER_ENGINE_HPP

#include "core/timer.hpp"
#include "platform/platform.hpp"
#include "world/world_manager.hpp"
#include "rendering/renderer.hpp"

/// @brief Main engine class, handles subsystem management and lifecycle.
class Engine
{
public:
    Engine();
    ~Engine();

    Engine(Engine const&) = delete;
    Engine& operator=(Engine const&) = delete;

    /// @brief Run the engine main loop.
    void run();

private:
    bool            m_running       = false;
    Timer           m_timer         = Timer();
    Platform*       m_platform      = nullptr;
    Surface*        m_surface       = nullptr;
    WorldManager*   m_world_manager = nullptr;
    Renderer*       m_renderer      = nullptr;
};

#endif //BONSAI_RENDERER_ENGINE_HPP