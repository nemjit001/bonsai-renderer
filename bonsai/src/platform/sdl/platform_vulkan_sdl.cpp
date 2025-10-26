#include "platform/platform_vulkan.hpp"
#if BONSAI_PLATFORM_SDL
#if BONSAI_USE_VULKAN

#include <SDL3/SDL_vulkan.h>
#include "platform_sdl.hpp"

char const** platform_enumerate_vulkan_instance_extensions(uint32_t* out_count)
{
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
    {
        *out_count = 0;
        return nullptr;
    }

    char const* const* extension_names = SDL_Vulkan_GetInstanceExtensions(out_count);
    return const_cast<char const**>(extension_names); // This is kinda gross...
}

bool platform_create_vulkan_surface(Surface const* platform_surface, VkInstance instance, VkAllocationCallbacks const* allocator, VkSurfaceKHR* out_surface)
{
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
    {
        return false;
    }

    SurfaceImpl const* surface_impl = platform_surface->raw_surface();
    return SDL_Vulkan_CreateSurface(surface_impl->window, instance, allocator, out_surface);
}

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_PLATFORM_SDL