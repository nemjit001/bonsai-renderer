#include "bonsai/render_backend/render_backend.hpp"

#if BONSAI_USE_VULKAN
#include "vulkan_render_backend.hpp"
#endif

RenderBackend* RenderBackend::create(PlatformSurface* platform_surface)
{
#if BONSAI_USE_VULKAN
    return new VulkanRenderBackend(platform_surface);
#else
    #error "No render backend selected for Bonsai"
#endif
}
