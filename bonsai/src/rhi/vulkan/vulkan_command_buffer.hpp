#pragma once
#ifndef BONSAI_RENDERER_VULKAN_COMMAND_BUFFER_HPP
#define BONSAI_RENDERER_VULKAN_COMMAND_BUFFER_HPP
#if BONSAI_USE_VULKAN

#include <volk.h>
#include "rhi/rhi.hpp"

class VulkanCommandBuffer : public ICommandBuffer
{
public:
    explicit VulkanCommandBuffer(VkCommandBuffer command_buffer);
    ~VulkanCommandBuffer() override = default;

    VulkanCommandBuffer(VulkanCommandBuffer const&) = delete;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer const&) = delete;

protected:
    void* get_raw_object() const override { return m_command_buffer; }

private:
    VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_VULKAN_COMMAND_BUFFER_HPP