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

/// @brief Vulkan queue family indices for a physical device.
struct VulkanQueueFamilies
{
    /// @brief Get the unique queue family indices for this queue setup.
    /// @return A vector of unique queue families.
    [[nodiscard]]
    std::vector<uint32_t> get_unique() const;

    uint32_t graphics_family; /// @brief The graphics queue family is also guaranteed to support presenting to surfaces.
};

/// @brief Vulkan implementation for the render backend.
class VulkanRenderBackend : public RenderBackend
{
public:
    /// @brief Create a new Vulkan render backend.
    /// @param platform_surface Main application surface, used for setting up initial state for device selection, swap chain, etc.
    explicit VulkanRenderBackend(PlatformSurface* platform_surface);
    ~VulkanRenderBackend() override;

    VulkanRenderBackend(VulkanRenderBackend const&) = delete;
    VulkanRenderBackend& operator=(VulkanRenderBackend const&) = delete;

private:
    /// @brief Check if device extensions are available on a physical device.
    /// @param device Device to check support for.
    /// @param extension_names Required extension names.
    /// @return A boolean indicating extension availability.
    static bool has_device_extensions(
        VkPhysicalDevice device,
        std::vector<char const*> const& extension_names
    );

    /// @brief Search for a physical device.
    /// @param instance Instance to use for search.
    /// @param device_properties Device properties output variable, populated on success.
    /// @param enabled_device_features Enabled device features based on internal feature query, populated on success.
    /// @param enabled_device_extensions Enabled device extensions that should be supported on the device.
    /// @return A suitable physical device, or VK_NULL_HANDLE on failure.
    static VkPhysicalDevice find_physical_device(
        VkInstance instance,
        VkPhysicalDeviceProperties& device_properties,
        VulkanDeviceFeatures& enabled_device_features,
        std::vector<char const*> const& enabled_device_extensions
    );

    /// @param Find a queue family index by flags and surface support.
    /// @param physical_device Physical device to query for queue families.
    /// @param queue_families Queue families belonging to the passed physical device.
    /// @param required_flags Required queue flags.
    /// @param ignored_flags Ignored queue flags.
    /// @param surface Optional surface that should have present support for a queue with the given filter flags.
    /// @return A queue family index, or VK_QUEUE_FAMILY_IGNORED on failure.
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