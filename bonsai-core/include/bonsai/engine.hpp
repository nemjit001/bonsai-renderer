#pragma once
#ifndef BONSAI_RENDERER_ENGINE_HPP
#define BONSAI_RENDERER_ENGINE_HPP

/// @brief The Engine class glues all bonsai systems together :)
class Engine
{
public:
    Engine();
    ~Engine();

    Engine(Engine const&) = delete;
    Engine& operator=(Engine const&) = delete;

    /// @brief Run the engine main loop.
    /// @param app_name Application library name to run.
    void run(char const* app_name);
};

#endif //BONSAI_RENDERER_ENGINE_HPP