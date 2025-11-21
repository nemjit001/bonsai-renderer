#pragma once
#ifndef BONSAI_RENDERER_ENGINE_HPP
#define BONSAI_RENDERER_ENGINE_HPP

class Engine
{
public:
    Engine();
    ~Engine();

    Engine(Engine const&) = delete;
    Engine& operator=(Engine const&) = delete;

    void run();
};

#endif //BONSAI_RENDERER_ENGINE_HPP