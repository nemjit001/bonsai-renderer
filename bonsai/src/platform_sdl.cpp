#include "platform.hpp"

#include <unordered_map>
#include <SDL3/SDL.h>
#include "logger.hpp"

struct Surface
{
    SDL_WindowID window_id;
    SDL_Window* window;
    void* user_data;
};

struct Platform::Impl
{
    void* user_data;
    std::unordered_map<SDL_WindowID, Surface*> surfaces;
    PFN_PlatformQuitCallback quit_callback;
    PFN_PlatformSurfaceResizeCallback surface_resize_callback;
    PFN_PlatformSurfaceClosedCallback surface_closed_callback;
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

void Platform::pump_messages()
{
    SDL_Event event{};
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            if (m_pImpl->quit_callback) m_pImpl->quit_callback(m_pImpl->user_data);
            break;
        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_EXPOSED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            if (m_pImpl->surface_resize_callback)
            {
                Surface const* surface = m_pImpl->surfaces[event.window.windowID];
                m_pImpl->surface_resize_callback(surface->user_data, event.window.data1, event.window.data2);
            }
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
            if (m_pImpl->surface_resize_callback)
            {
                Surface const* surface = m_pImpl->surfaces[event.window.windowID];
                m_pImpl->surface_resize_callback(surface->user_data, 0, 0);
            }
            break;
        case SDL_EVENT_WINDOW_RESTORED:
            if (m_pImpl->surface_resize_callback)
            {
                Surface const* surface = m_pImpl->surfaces[event.window.windowID];
                int width = 0, height = 0;
                SDL_GetWindowSize(surface->window, &width, &height);
                m_pImpl->surface_resize_callback(surface->user_data, width, height);
            }
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (m_pImpl->surface_closed_callback)
            {
                Surface const* surface = m_pImpl->surfaces[event.window.windowID];
                m_pImpl->surface_closed_callback(surface->user_data);
            }
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
    Surface* surface = new Surface{ window_id, window, nullptr };

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

void Platform::set_surface_user_data(Surface* surface, void* user_data)
{
    surface->user_data = user_data;
}

void Platform::set_platform_user_data(void* user_data)
{
    m_pImpl->user_data = user_data;
}

void Platform::set_platform_quit_callback(PFN_PlatformQuitCallback const& callback)
{
    m_pImpl->quit_callback = callback;
}

void Platform::set_platform_surface_resize_callback(PFN_PlatformSurfaceResizeCallback const& callback)
{
    m_pImpl->surface_resize_callback = callback;
}

void Platform::set_platform_surface_closed_callback(PFN_PlatformSurfaceClosedCallback const& callback)
{
    m_pImpl->surface_closed_callback = callback;
}

