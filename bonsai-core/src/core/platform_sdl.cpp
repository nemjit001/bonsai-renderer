#include "bonsai/core/platform.hpp"

#include <unordered_map>
#include <SDL3/SDL.h>

struct RawPlatformSurface
{
    SDL_Window* window;
    void* user_ptr;
};

struct Platform::Impl
{
    PFN_PlatformSurfaceResized surface_resized_callback;
    std::unordered_map<SDL_WindowID, PlatformSurface*> tracked_surfaces;
};

PlatformSurface::PlatformSurface(RawPlatformSurface* raw_surface)
    :
    m_raw_surface(raw_surface)
{
    //
}

void PlatformSurface::set_user_data(void* user_data)
{
    m_raw_surface->user_ptr = user_data;
}

void* PlatformSurface::get_user_data()
{
    return m_raw_surface->user_ptr;
}

Platform::Platform()
    :
    m_impl(new Impl{})
{
    //
}

Platform::~Platform()
{
    delete m_impl;
}

bool Platform::pump_messages()
{
    SDL_Event event{};
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            return false;
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
        case SDL_EVENT_WINDOW_RESTORED:
            if (m_impl->surface_resized_callback) {
                PlatformSurface* surface = m_impl->tracked_surfaces[event.window.windowID];
                RawPlatformSurface const* raw_surface = surface->get_raw();

                int width = 0, height = 0;
                SDL_GetWindowSizeInPixels(raw_surface->window, &width, &height);
                m_impl->surface_resized_callback(surface, width, width);
            }
            break;
        default:
            break;
        }
    }

    return true;
}

void Platform::set_surface_resized_callback(PFN_PlatformSurfaceResized callback)
{
    m_impl->surface_resized_callback = callback;
}

PlatformSurface* Platform::create_surface(char const* title, uint32_t width, uint32_t height, PlatformSurfaceConfig const& config)
{
    SDL_Window* window = SDL_CreateWindow(title, static_cast<int>(width), static_cast<int>(height), 0);
    if (window == nullptr)
    {
        return nullptr;
    }

    RawPlatformSurface* raw_surface = new RawPlatformSurface{};
    raw_surface->window = window;

    PlatformSurface* surface = new PlatformSurface(raw_surface);
    m_impl->tracked_surfaces[SDL_GetWindowID(window)] = surface;
    return surface;
}

void Platform::destroy_surface(PlatformSurface* surface)
{
    RawPlatformSurface const* raw_surface = surface->get_raw();
    SDL_WindowID const window_id = SDL_GetWindowID(raw_surface->window);

    SDL_DestroyWindow(raw_surface->window);
    delete raw_surface;

    m_impl->tracked_surfaces.erase(window_id);
    delete surface;
}
