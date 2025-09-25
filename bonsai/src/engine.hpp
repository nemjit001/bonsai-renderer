#pragma once
#ifndef BONSAI_RENDERER_ENGINE_HPP
#define BONSAI_RENDERER_ENGINE_HPP

#include "platform/platform.hpp"

class Engine
{
public:
    Engine() = default;
    ~Engine();

    Engine(Engine const&) = delete;
    Engine& operator=(Engine const&) = delete;

    /// @brief Initialize all engine subsystems.
    void init();

    /// @brief Run the engine main loop.
    void run();

private:
    bool        m_running   = false;
    Platform*   m_platform  = nullptr;
    Surface*    m_surface   = nullptr;
};

#endif //BONSAI_RENDERER_ENGINE_HPP