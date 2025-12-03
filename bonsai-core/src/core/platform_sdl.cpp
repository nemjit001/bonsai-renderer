#include "bonsai/core/platform.hpp"

#include <unordered_map>
#include <backends/imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include "bonsai/core/fatal_exit.hpp"

#if     BONSAI_USE_VULKAN
    #include <SDL3/SDL_vulkan.h>
#endif //BONSAI_USE_VULKAN

static int get_sdl_window_flags(PlatformSurfaceConfig const& config)
{
    int flags = 0;
    if (config.resizable)
        flags |= SDL_WINDOW_RESIZABLE;
    if (config.high_dpi)
        flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

    flags |= SDL_WINDOW_VULKAN;
    return flags;
}

struct RawPlatformSurface
{
    SDL_Window* window;
    void* user_ptr;
};

struct Platform::Impl
{
    bool imgui_initialized = false;
    PFN_PlatformSurfaceResized surface_resized_callback;
    std::unordered_map<SDL_WindowID, PlatformSurface*> tracked_surfaces;
};

PlatformSurface::PlatformSurface(RawPlatformSurface* raw_surface)
    :
    m_raw_surface(raw_surface)
{
    //
}

void PlatformSurface::get_size(uint32_t& width, uint32_t& height) const
{
    int w = 0, h = 0;
    SDL_GetWindowSize(m_raw_surface->window, &w, &h);
    width = static_cast<uint32_t>(w);
    height = static_cast<uint32_t>(h);
}

void PlatformSurface::get_size_in_pixels(uint32_t& width, uint32_t& height) const
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(m_raw_surface->window, &w, &h);
    width = static_cast<uint32_t>(w);
    height = static_cast<uint32_t>(h);
}

void PlatformSurface::set_user_data(void* user_data)
{
    m_raw_surface->user_ptr = user_data;
}

void* PlatformSurface::get_user_data()
{
    return m_raw_surface->user_ptr;
}

#if     BONSAI_USE_VULKAN
bool PlatformSurface::create_vulkan_surface(VkInstance instance, VkAllocationCallbacks const* allocator, VkSurfaceKHR* surface) const
{
    return SDL_Vulkan_CreateSurface(m_raw_surface->window, instance, allocator, surface);
}
#endif //BONSAI_USE_VULKAN

Platform::Platform()
    :
    m_impl(new Impl{})
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        BONSAI_FATAL_EXIT("Failed to initialize SDL3\n");
    }
}

Platform::~Platform()
{
    if (m_impl->imgui_initialized)
        ImGui_ImplSDL3_Shutdown();

    for (auto const& [ window_id, surface ] : m_impl->tracked_surfaces)
        delete surface;

    SDL_Quit();
    delete m_impl;
}

bool Platform::pump_messages()
{
    ImGuiIO const& io = ImGui::GetIO();
    SDL_Event event{};
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            return false;
        case SDL_EVENT_WINDOW_RESIZED:
            if (m_impl->surface_resized_callback) {
                PlatformSurface* surface = m_impl->tracked_surfaces[event.window.windowID];
                m_impl->surface_resized_callback(surface, event.window.data1, event.window.data2);
            }
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
            if (m_impl->surface_resized_callback) {
                PlatformSurface* surface = m_impl->tracked_surfaces[event.window.windowID];
                m_impl->surface_resized_callback(surface, 0, 0);
            }
            break;
        case SDL_EVENT_WINDOW_MAXIMIZED:
        case SDL_EVENT_WINDOW_RESTORED:
            if (m_impl->surface_resized_callback) {
                PlatformSurface* surface = m_impl->tracked_surfaces[event.window.windowID];
                uint32_t width = 0, height = 0;
                surface->get_size_in_pixels(width, height);
                m_impl->surface_resized_callback(surface, width, height);
            }
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            // TODO(nemjit001): Handle platform inputs & translate to usable format for applications
            break;
        default:
            break;
        }
    }

    ImGui_ImplSDL3_NewFrame();
    return true;
}

void Platform::set_surface_resized_callback(PFN_PlatformSurfaceResized callback)
{
    m_impl->surface_resized_callback = callback;
}

PlatformSurface* Platform::create_surface(char const* title, uint32_t width, uint32_t height, PlatformSurfaceConfig const& config)
{
    int const window_flags = get_sdl_window_flags(config);
    SDL_Window* window = SDL_CreateWindow(title, static_cast<int>(width), static_cast<int>(height), window_flags);
    if (window == nullptr)
    {
        return nullptr;
    }

    // First created surface gets to be the main ImGui context :)
    if (!m_impl->imgui_initialized)
    {
#if     BONSAI_USE_VULKAN
        // FIXME(nemjit001): This is Vulkan specific, should be hidden behind comptime constant when multiple backends are supported.
        if (!ImGui_ImplSDL3_InitForVulkan(window))
        {
            BONSAI_FATAL_EXIT("Failed to initialize the SDL3 ImGui backend\n");
        }
#else
        if (!ImGui_ImplSDL3_InitForOther(window))
        {
            BONSAI_FATAL_EXIT("Failed to initialize the SDL3 ImGui backend\n");
        }
#endif

        m_impl->imgui_initialized = true;
    }

    // Create platform surface data
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

#if     BONSAI_USE_VULKAN
char const** Platform::get_vulkan_instance_extensions(uint32_t* count)
{
    return const_cast<char const**>(SDL_Vulkan_GetInstanceExtensions(count));
}
#endif //BONSAI_USE_VULKAN
