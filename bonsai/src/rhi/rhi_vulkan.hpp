#pragma once
#ifndef BONSAI_RENDERER_RHI_VULKAN_HPP
#define BONSAI_RENDERER_RHI_VULKAN_HPP
#if BONSAI_USE_VULKAN

#include <volk.h>
#include <vk_mem_alloc.h>
#include "rhi.hpp"

constexpr uint32_t BONSAI_VULKAN_VERSION = VK_API_VERSION_1_3;

/// @brief Vulkan queue families available in a device.
struct VulkanQueueFamilies
{
    uint32_t graphicsFamily; // Used as direct queue (graphics + transfer + compute workloads)
    uint32_t transferFamily;
    uint32_t computeFamily;
};

class VulkanRenderDevice : public IRenderDevice
{
public:
    VulkanRenderDevice(
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

    CommandBufferHandle create_command_buffer(CommandQueueType queue) override;

private:
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

private:
    VkInstance                  m_instance          = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT    m_debug_messenger   = VK_NULL_HANDLE;
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_RHI_VULKAN_HPP