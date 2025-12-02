#include "bonsai/render_backend/render_backend.hpp"

#if BONSAI_USE_VULKAN
#include "vulkan_render_backend.hpp"
#endif

RenderBackend* RenderBackend::create(PlatformSurface* platform_surface, ImGuiContext* imgui_context)
{
#if BONSAI_USE_VULKAN
    return new VulkanRenderBackend(platform_surface, imgui_context);
#else
    return nullptr;
#endif
}
