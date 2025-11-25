#pragma once
#ifndef BONSAI_RENDERER_VULKAN_RENDER_BACKEND_HPP
#define BONSAI_RENDERER_VULKAN_RENDER_BACKEND_HPP

#include <volk.h>
#include "bonsai/core/platform.hpp"

constexpr uint32_t BONSAI_VULKAN_VERSION = VK_API_VERSION_1_3;

class VulkanRenderBackend
{
public:
    explicit VulkanRenderBackend(PlatformSurface* platform_surface);
    ~VulkanRenderBackend();

    VulkanRenderBackend(VulkanRenderBackend const&) = delete;
    VulkanRenderBackend& operator=(VulkanRenderBackend const&) = delete;

private:
    VkInstance m_instance = VK_NULL_HANDLE;
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
#endif //NDEBUG
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
};


#endif //BONSAI_RENDERER_VULKAN_RENDER_BACKEND_HPP