#pragma once
#ifndef BONSAI_RENDERER_VULKAN_COMMAND_ALLOCATOR_HPP
#define BONSAI_RENDERER_VULKAN_COMMAND_ALLOCATOR_HPP
#if BONSAI_USE_VULKAN

#include <volk.h>
#include "rhi/rhi.hpp"

class VulkanCommandAllocator : public ICommandAllocator
{
public:
    VulkanCommandAllocator(VkDevice device, VkCommandPool command_pool);
    ~VulkanCommandAllocator() override;

    VulkanCommandAllocator(VulkanCommandAllocator const&) = delete;
    VulkanCommandAllocator& operator=(VulkanCommandAllocator const&) = delete;

    bool reset() override;

    CommandBufferHandle create_command_buffer() override;

private:
    VkDevice        m_device        = VK_NULL_HANDLE;
    VkCommandPool   m_command_pool  = VK_NULL_HANDLE;
};

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_VULKAN_COMMAND_ALLOCATOR_HPP