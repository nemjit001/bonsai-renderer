#pragma once
#ifndef BONSAI_RENDERER_RHI_VULKAN_HPP
#define BONSAI_RENDERER_RHI_VULKAN_HPP
#if BONSAI_USE_VULKAN

#include <vector>
#include <volk.h>
#include <vk_mem_alloc.h>
#include "rhi.hpp"

constexpr uint32_t BONSAI_VULKAN_VERSION = VK_API_VERSION_1_3;

/// @brief Vulkan queue families available in a device.
struct VulkanQueueFamilies
{
    /// @brief Get a list of unique queue family indices from the queue setup.
    [[nodiscard]] std::vector<uint32_t> get_unique() const;

    uint32_t graphicsFamily; // Used as direct queue (graphics + transfer + compute workloads)
    uint32_t transferFamily;
    uint32_t computeFamily;
};

class VulkanRenderDevice : public IRenderDevice
{
public:
    VulkanRenderDevice(
        bool headless,
        VkInstance instance,
        VkPhysicalDevice physical_device,
        VulkanQueueFamilies const& queue_families,
        VkDevice device,
        VmaAllocator allocator
    );
    ~VulkanRenderDevice() override;

    VulkanRenderDevice(VulkanRenderDevice const&) = delete;
    VulkanRenderDevice& operator=(VulkanRenderDevice const&) = delete;

    bool is_headless() const override;

    BufferHandle create_buffer(BufferDesc& desc) override;

    TextureHandle create_texture(TextureDesc& desc) override;

    CommandAllocatorHandle create_command_allocator(CommandQueueType queue) override;

    SwapChainHandle create_swap_chain(SwapChainDesc const& desc) override;

    void submit(CommandQueueType queue, size_t count, CommandBufferHandle* command_buffers) override;

protected:
    void* get_raw_object() const override { return m_device; }

private:
    bool                m_headless          = true;
    VkInstance          m_instance          = VK_NULL_HANDLE;
    VkPhysicalDevice    m_physical_device   = VK_NULL_HANDLE;
    VulkanQueueFamilies m_queue_families    = {};
    VkDevice            m_device            = VK_NULL_HANDLE;
    VkQueue             m_graphics_queue    = VK_NULL_HANDLE;
    VkQueue             m_transfer_queue    = VK_NULL_HANDLE;
    VkQueue             m_compute_queue     = VK_NULL_HANDLE;
    VmaAllocator        m_allocator         = VK_NULL_HANDLE;
};

class VulkanRHIInstance : public IRHIInstance
{
public:
    VulkanRHIInstance();
    ~VulkanRHIInstance() override;

    VulkanRHIInstance(VulkanRHIInstance const&) = delete;
    VulkanRHIInstance& operator=(VulkanRHIInstance const&) = delete;

    RenderDeviceHandle create_render_device(RenderDeviceDesc const& desc) override;

protected:
    void* get_raw_object() const override { return m_instance; }

private:
    /// @brief Get the default Vulkan debug messenger create info for the RHI backend.
    /// @return
    static VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info();

    /// @brief Find a suitable physical device using the Vulkan instance.
    /// @param instance Instance to use for device search.
    /// @param compatible_surface Optional compatible surface to use for device search.
    /// @return A physical device on success or VK_NULL_HANDLE on failure.
    static VkPhysicalDevice find_physical_device(VkInstance instance, VkSurfaceKHR compatible_surface);

    /// @brief Find a device queue family for a physical device.
    /// @param physical_device
    /// @param required_flags Required queue flags.
    /// @param ignored_flags Ignored queue flags.
    /// @param compatible_surface Optional surface which the queue should test present support for.
    /// @return The found queue family on success or VK_QUEUE_FAMILY_IGNORED on failure.
    static uint32_t find_queue_family(VkPhysicalDevice physical_device, VkQueueFlags required_flags, VkQueueFlags ignored_flags, VkSurfaceKHR compatible_surface);

private:
    VkInstance                  m_instance          = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT    m_debug_messenger   = VK_NULL_HANDLE;
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_RHI_VULKAN_HPP