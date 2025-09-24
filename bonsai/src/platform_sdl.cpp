#include "platform.hpp"

#include <SDL3/SDL.h>
#include "logger.hpp"

/// @brief SDL platform implementation.
struct Platform::Impl
{
    //
};

Platform::Platform()
    :
    m_pImpl(new Impl{})
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        BONSAI_LOG_CRITICAL("Failed to initialize SDL: {}", SDL_GetError());
        std::exit(1); // FIXME(nemjit001): Should this throw?
    }
}

Platform::~Platform()
{
    SDL_Quit();
    delete m_pImpl;
}

Platform& Platform::get()
{
    static Platform instance;
    return instance;
}

void Platform::pump_messages()
{
    SDL_Event event{};
    while (SDL_PollEvent(&event))
    {
        // TODO(nemjit001): Handle SDL events
    }
}
