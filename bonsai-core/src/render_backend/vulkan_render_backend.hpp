#pragma once
#ifndef BONSAI_RENDERER_VULKAN_RENDER_BACKEND_HPP
#define BONSAI_RENDERER_VULKAN_RENDER_BACKEND_HPP

#include <vector>
#include <volk.h>
#include "bonsai/render_backend/render_backend.hpp"

constexpr uint32_t BONSAI_VULKAN_VERSION = VK_API_VERSION_1_3;

/// @brief This struct contains all Vulkan device features that are queried during physical device selection.
/// By passing the features2 field to vkDeviceCreateInfo::pNext, the queried features will be enabled on the logical device.
/// See @ref VulkanRenderBackend::find_physical_device for queried features.
struct VulkanDeviceFeatures
{
    VkPhysicalDeviceFeatures2 features2;
    VkPhysicalDeviceVulkan11Features vulkan11_features;
    VkPhysicalDeviceVulkan12Features vulkan12_features;
    VkPhysicalDeviceVulkan13Features vulkan13_features;
};

struct VulkanQueueFamilies
{
    /// @brief Get the unique queue family indices for this queue setup.
    /// @return A vector of unique queue families.
    [[nodiscard]]
    std::vector<uint32_t> get_unique() const;

    uint32_t graphics_family;
};

class VulkanRenderBackend : public RenderBackend
{
public:
    explicit VulkanRenderBackend(PlatformSurface* platform_surface);
    ~VulkanRenderBackend() override;

    VulkanRenderBackend(VulkanRenderBackend const&) = delete;
    VulkanRenderBackend& operator=(VulkanRenderBackend const&) = delete;

private:
    static bool has_device_extensions(
        VkPhysicalDevice device,
        std::vector<char const*> const& extension_names
    );

    static VkPhysicalDevice find_physical_device(
        VkInstance instance,
        VkPhysicalDeviceProperties& device_properties,
        VulkanDeviceFeatures& enabled_device_features,
        std::vector<char const*> const& enabled_device_extensions
    );

    static uint32_t find_queue_family(
        VkPhysicalDevice physical_device,
        std::vector<VkQueueFamilyProperties> const& queue_families,
        VkQueueFlags required_flags,
        VkQueueFlags ignored_flags,
        VkSurfaceKHR surface = VK_NULL_HANDLE
    );

private:
    VkInstance m_instance = VK_NULL_HANDLE;
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
#endif //NDEBUG
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VulkanQueueFamilies m_queue_families = {};
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
};


#endif //BONSAI_RENDERER_VULKAN_RENDER_BACKEND_HPP