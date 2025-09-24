#include "platform.hpp"

#include <unordered_map>
#include <SDL3/SDL.h>
#include "logger.hpp"

struct Surface
{
    SDL_WindowID window_id;
    SDL_Window* window;
};

struct Platform::Impl
{
    std::unordered_map<SDL_WindowID, Surface*> surfaces;
};

static int get_sdl_window_flags(SurfaceConfig const& config)
{
    int window_flags = 0;
    if (config.resizable)
    {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    if (config.allow_high_dpi)
    {
        window_flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    }
    return window_flags;
}

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
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            BONSAI_LOG_TRACE("Quit event received");
            break;
        case SDL_EVENT_WINDOW_SHOWN:
            BONSAI_LOG_TRACE("Window shown ({})", event.window.windowID);
            break;
        case SDL_EVENT_WINDOW_HIDDEN:
            BONSAI_LOG_TRACE("Window hidden ({})", event.window.windowID);
            break;
        case SDL_EVENT_WINDOW_EXPOSED:
            BONSAI_LOG_TRACE("Window exposed ({})", event.window.windowID);
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            BONSAI_LOG_TRACE("Window resized to {}x{} ({})", event.window.windowID, event.window.data1, event.window.data2);
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            BONSAI_LOG_TRACE("Window close requested ({})", event.window.windowID);
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
            BONSAI_LOG_TRACE("Window minimized ({})", event.window.windowID);
            break;
        case SDL_EVENT_WINDOW_MAXIMIZED:
            BONSAI_LOG_TRACE("Window maximized ({})", event.window.windowID);
            break;
        case SDL_EVENT_WINDOW_RESTORED:
            BONSAI_LOG_TRACE("Window restored ({})", event.window.windowID);
            break;
        default:
            break;
        }
    }
}

Surface* Platform::create_surface(char const* title, uint32_t width, uint32_t height, SurfaceConfig const& config)
{
    // Parse surface config into window flags
    int const window_flags = get_sdl_window_flags(config);

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow(title, static_cast<int>(width), static_cast<int>(height), window_flags);
    if (window == nullptr)
    {
        return nullptr;
    }

    // Create new surface
    SDL_WindowID window_id = SDL_GetWindowID(window);
    Surface* surface = new Surface{ window_id, window };

    // Add surface to tracked surfaces map
    m_pImpl->surfaces[window_id] = surface;
    return surface;
}

void Platform::destroy_surface(Surface* surface)
{
    if (surface == nullptr)
    {
        return;
    }

    // Remove tracked surface from surfaces map
    m_pImpl->surfaces.erase(surface->window_id);

    // Destroy surface
    SDL_DestroyWindow(surface->window);
    delete surface;
}

