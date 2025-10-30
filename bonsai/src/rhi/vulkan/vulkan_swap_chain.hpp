#pragma once
#ifndef BONSAI_RENDERER_VULKAN_SWAP_CHAIN_HPP
#define BONSAI_RENDERER_VULKAN_SWAP_CHAIN_HPP

#include <vector>
#include <volk.h>
#include "rhi/rhi.hpp"

/// @brief Vulkan surface capabilities structure queried during swap chain creation.
struct VulkanSurfaceCapabilities
{
    /// @brief Check if a format is supported by the surface capabilities.
    /// @param format
    /// @return
    bool is_format_supported(Format format) const;

    /// @brief Check if a present mode is supported by the surface capabilities.
    /// @param present_mode
    /// @return
    bool is_present_mode_supported(SwapPresentMode present_mode) const;

    uint32_t preferred_image_count;
    uint32_t width;
    uint32_t height;
    std::vector<VkSurfaceFormatKHR> supported_formats;
    std::vector<VkPresentModeKHR> supported_present_modes;
    VkSurfaceTransformFlagBitsKHR current_transform;
    VkCompositeAlphaFlagBitsKHR composite_alpha;
};

class VulkanSwapChain : public ISwapChain
{
public:
    VulkanSwapChain(
        VkInstance instance,
        VkPhysicalDevice physical_device,
        VkDevice device,
        VkQueue present_queue,
        VkSurfaceKHR surface,
        VkSwapchainKHR swap_chain,
        SwapChainDesc const& desc
    );
    ~VulkanSwapChain() override;

    VulkanSwapChain(VulkanSwapChain const&) = delete;
    VulkanSwapChain& operator=(VulkanSwapChain const&) = delete;

    /// @brief Get the Vulkan surface capabilities based on a swap chain descriptor.
    /// @param physical_device Physical device to use for queries.
    /// @param surface Surface to use for queries.
    /// @param desc Swap chain descriptor to use for capabilities check.
    /// @return A surface capabilities structure that may be used to create a swap chain.
    static VulkanSurfaceCapabilities get_surface_capabilities(VkPhysicalDevice physical_device, VkSurfaceKHR surface, SwapChainDesc const& desc);

    /// @brief Convert a swap present mode to a Vulkan present mode.
    /// @param present_mode
    /// @return
    static VkPresentModeKHR get_vulkan_present_mode(SwapPresentMode present_mode);

    bool resize_swap_buffers(uint32_t width, uint32_t height, SwapPresentMode present_mode) override;

    bool acquire_next_image() override;

    bool present() override;

    uint32_t current_image_idx() override;

    uint32_t swap_image_count() override;

    TextureHandle get_swap_image(uint32_t idx) override;

    SwapChainDesc get_desc() const override { return m_desc; }

protected:
    void* get_raw_object() const override { return m_swap_chain; }

private:
    VkInstance              m_instance          = VK_NULL_HANDLE;
    VkPhysicalDevice        m_physical_device   = VK_NULL_HANDLE;
    VkDevice                m_device            = VK_NULL_HANDLE;
    VkQueue                 m_present_queue     = VK_NULL_HANDLE;
    VkSurfaceKHR            m_surface           = VK_NULL_HANDLE;
    VkSwapchainKHR          m_swap_chain        = VK_NULL_HANDLE;
    SwapChainDesc           m_desc              = {};
    std::vector<VkImage>    m_swap_images;
    uint32_t                m_active_image_idx  = 0;
};

#endif //BONSAI_RENDERER_VULKAN_SWAP_CHAIN_HPP