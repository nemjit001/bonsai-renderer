#include "platform_sdl.hpp"

#if BONSAI_PLATFORM_SDL

#include "core/die.hpp"
#include "core/logger.hpp"

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

Surface::Surface(SurfaceImpl* impl)
    :
    m_impl(impl)
{
    //
}

Surface::~Surface()
{
    SDL_DestroyWindow(m_impl->window);
    delete m_impl;
}

void Surface::get_size(uint32_t& width, uint32_t& height) const
{
    int _width = 0, _height = 0;
    SDL_GetWindowSizeInPixels(m_impl->window, &_width, &_height);
    width = static_cast<uint32_t>(_width);
    height = static_cast<uint32_t>(_height);
}

void Surface::set_user_data(void* user_data)
{
    m_impl->user_data = user_data;
}

Platform::Platform()
    :
    m_impl(new Impl{})
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        bonsai::die("Failed to initialize SDL: {}", SDL_GetError());
    }
}

Platform::~Platform()
{
    SDL_Quit();
    delete m_impl;
}

void Platform::pump_messages()
{
    SDL_Event event{};
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        // Handle quit event
        case SDL_EVENT_QUIT:
            if (m_impl->quit_callback) m_impl->quit_callback(m_impl->user_data);
            break;
        // Handle window events
        case SDL_EVENT_WINDOW_RESIZED:
            if (m_impl->surface_resize_callback)
            {
                Surface const* surface = m_impl->surfaces[event.window.windowID];
                m_impl->surface_resize_callback(surface->m_impl->user_data, event.window.data1, event.window.data2);
            }
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
            if (m_impl->surface_resize_callback)
            {
                Surface const* surface = m_impl->surfaces[event.window.windowID];
                m_impl->surface_resize_callback(surface->m_impl->user_data, 0, 0);
            }
            break;
        case SDL_EVENT_WINDOW_RESTORED:
            if (m_impl->surface_resize_callback)
            {
                Surface const* surface = m_impl->surfaces[event.window.windowID];
                int width = 0, height = 0;
                SDL_GetWindowSize(surface->m_impl->window, &width, &height);
                m_impl->surface_resize_callback(surface->m_impl->user_data, width, height);
            }
            break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (m_impl->surface_closed_callback)
            {
                Surface const* surface = m_impl->surfaces[event.window.windowID];
                m_impl->surface_closed_callback(surface->m_impl->user_data);
            }
            break;
        // Handle key events
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            {
                Surface const* surface = m_impl->surfaces[event.key.windowID];
                m_impl->surface_key_callback(surface->m_impl->user_data, static_cast<int32_t>(event.key.key), static_cast<int32_t>(event.key.scancode), event.key.down);
            }
            break;
        // Handle mouse events
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL:
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
    Surface* surface = new Surface(new SurfaceImpl{ window_id, window, nullptr });

    // Add surface to tracked surfaces map
    m_impl->surfaces[window_id] = surface;
    return surface;
}

void Platform::destroy_surface(Surface* surface)
{
    if (surface == nullptr)
    {
        return;
    }

    // Remove tracked surface from surfaces map
    m_impl->surfaces.erase(surface->m_impl->window_id);
    delete surface;
}

void Platform::set_user_data(void* user_data)
{
    m_impl->user_data = user_data;
}

void Platform::set_platform_quit_callback(PFN_PlatformQuitCallback const& callback)
{
    m_impl->quit_callback = callback;
}

void Platform::set_platform_surface_resize_callback(PFN_PlatformSurfaceResizeCallback const& callback)
{
    m_impl->surface_resize_callback = callback;
}

void Platform::set_platform_surface_closed_callback(PFN_PlatformSurfaceClosedCallback const& callback)
{
    m_impl->surface_closed_callback = callback;
}

void Platform::set_platform_surface_key_callback(PFN_PlatformSurfaceKeyCallback const& callback)
{
    m_impl->surface_key_callback = callback;
}

#endif // BONSAI_PLATFORM_SDL
