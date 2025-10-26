#pragma once
#ifndef BONSAI_RENDERER_RHI_VULKAN_HPP
#define BONSAI_RENDERER_RHI_VULKAN_HPP
#if BONSAI_USE_VULKAN

#include <volk.h>
#include <vk_mem_alloc.h>
#include "rhi.hpp"

constexpr uint32_t BONSAI_VULKAN_VERSION = VK_API_VERSION_1_3;

class VulkanRenderDevice : public IRenderDevice
{
public:
    bool is_headless() const override;

    BufferHandle create_buffer(BufferDesc& desc) override;

    TextureHandle create_texture(TextureDesc& desc) override;

    CommandBufferHandle create_command_buffer(CommandQueueType queue) override;
};

class VulkanRHIInstance : public IRHIInstance
{
public:
    VulkanRHIInstance();
    ~VulkanRHIInstance();

    VulkanRHIInstance(VulkanRHIInstance const&) = delete;
    VulkanRHIInstance& operator=(VulkanRHIInstance const&) = delete;

    RenderDeviceHandle create_render_device(RenderDeviceDesc const& desc) override;

private:
    VkInstance                  m_instance          = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT    m_debug_messenger   = VK_NULL_HANDLE;
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_RHI_VULKAN_HPP