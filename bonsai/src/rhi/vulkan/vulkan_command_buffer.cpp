#include "vulkan_command_buffer.hpp"

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer command_buffer)
    :
    ICommandBuffer(),
    m_command_buffer(command_buffer)
{
    //
}
