#include "platform/platform_win32.hpp"
#if BONSAI_PLATFORM_SDL
#if _WIN32

#include "platform_sdl.hpp"

HWND platform_get_surface_hwnd(Surface* surface)
{
    SurfaceImpl const* surface_impl = surface->raw_surface();
    SDL_PropertiesID const window_properties = SDL_GetWindowProperties(surface_impl->window);
    return static_cast<HWND>(SDL_GetPointerProperty(window_properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
}

#endif //_WIN32
#endif //BONSAI_PLATFORM_SDL