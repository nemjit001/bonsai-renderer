#pragma once
#ifndef BONSAI_RENDERER_RENDER_BACKEND_VULKAN_HPP
#define BONSAI_RENDERER_RENDER_BACKEND_VULKAN_HPP
#if BONSAI_USE_VULKAN

#include <vector>
#include <volk.h>
#include <vk_mem_alloc.h>
#include "rendering/render_backend.hpp"

/// @brief Minimum supported Vulkan API version against which Bonsai is written.
static constexpr uint32_t   BONSAI_MINIMUM_VULKAN_VERSION   = VK_API_VERSION_1_3;
/// @brief Number of frames in flight that Bonsai initializes with.
static constexpr size_t     BONSAI_FRAMES_IN_FLIGHT         = 2;

/// @brief Swap chain capabilities, contains possible config values that can be used to initialize the SwapchainConfig.
struct SwapchainCapabilities
{
    /// @brief Get the swapchain capabilities for a physical device and surface.
    /// @param device
    /// @param surface
    /// @return
    static SwapchainCapabilities get(VkPhysicalDevice device, VkSurfaceKHR surface);

    VkSurfaceCapabilitiesKHR surface_capabilities;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> present_infos;
};

/// @brief Swap chain configuration state, selected from swap capabilities.
struct SwapchainConfig
{
    /// @brief Get a swapchain configuration for a physical device, surface, and requested surface size.
    /// @param device
    /// @param surface
    /// @param width
    /// @param height
    /// @return
    static SwapchainConfig get(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height);

    VkSurfaceKHR surface;
    uint32_t image_count;
    VkExtent2D extent;
    VkFormat preferred_format;
    VkColorSpaceKHR preferred_color_space;
    VkPresentModeKHR preferred_present_mode;
    VkSurfaceTransformFlagBitsKHR current_transform;
};

/// @brief Vulkan frame state, contains state that is better kept separated between frames.
struct FrameState
{
    VkFence frame_ready;
    VkSemaphore swap_available;
    VkSemaphore rendering_finished;
    VkCommandPool graphics_pool;
    VkCommandPool compute_pool;
    VkCommandPool transfer_pool;
    VkCommandBuffer frame_commands;
    uint32_t swap_image_idx;
    uint32_t frame_idx;
};

/// @brief Vulkan Renderer implementation.
struct RenderBackendImpl
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    uint32_t graphics_queue_family;
    uint32_t compute_queue_family;
    uint32_t transfer_queue_family;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue compute_queue;
    VkQueue transfer_queue;
    VmaAllocator allocator;
    SwapchainConfig swap_config;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swap_images;
    std::vector<VkImageView> swap_image_views;
    std::vector<FrameState> frame_states;
    uint64_t frame_index;
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_RENDER_BACKEND_VULKAN_HPP