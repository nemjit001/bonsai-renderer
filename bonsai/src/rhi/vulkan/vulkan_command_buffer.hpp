#pragma once
#ifndef BONSAI_RENDERER_VULKAN_COMMAND_BUFFER_HPP
#define BONSAI_RENDERER_VULKAN_COMMAND_BUFFER_HPP

#include <volk.h>
#include "rhi/rhi.hpp"

class VulkanCommandBuffer : public ICommandBuffer
{
public:
    explicit VulkanCommandBuffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer);
    ~VulkanCommandBuffer() override;

    VulkanCommandBuffer(VulkanCommandBuffer const&) = delete;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer const&) = delete;

    bool begin() override;

    bool close() override;

protected:
    void* get_raw_object() const override { return m_command_buffer; }

private:
    VkDevice        m_device            = VK_NULL_HANDLE;
    VkCommandPool   m_command_pool      = VK_NULL_HANDLE;
    VkCommandBuffer m_command_buffer    = VK_NULL_HANDLE;
};

#endif //BONSAI_RENDERER_VULKAN_COMMAND_BUFFER_HPP