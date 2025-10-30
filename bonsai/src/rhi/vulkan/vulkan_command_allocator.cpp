#include "vulkan_command_allocator.hpp"

#include "vulkan_command_buffer.hpp"

VulkanCommandAllocator::VulkanCommandAllocator(VkDevice device, VkCommandPool command_pool)
    :
    m_device(device),
    m_command_pool(command_pool)
{
    //
}

VulkanCommandAllocator::~VulkanCommandAllocator()
{
    vkDestroyCommandPool(m_device, m_command_pool, nullptr);
}

bool VulkanCommandAllocator::reset()
{
    return vkResetCommandPool(m_device, m_command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) == VK_SUCCESS;
}

CommandBufferHandle VulkanCommandAllocator::create_command_buffer()
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.pNext = nullptr;
    command_buffer_allocate_info.commandPool = m_command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(m_device, &command_buffer_allocate_info, &command_buffer) != VK_SUCCESS)
    {
        return {};
    }

    return CommandBufferHandle(new VulkanCommandBuffer(m_device, m_command_pool, command_buffer));
}
