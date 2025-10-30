#include "vulkan_command_buffer.hpp"

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer command_buffer)
    :
    ICommandBuffer(),
    m_command_buffer(command_buffer)
{
    //
}

bool VulkanCommandBuffer::begin()
{
    if (vkResetCommandBuffer(m_command_buffer, 0 /* no  flags */) != VK_SUCCESS)
    {
        return false;
    }

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    return vkBeginCommandBuffer(m_command_buffer, &begin_info) == VK_SUCCESS;
}

bool VulkanCommandBuffer::close()
{
    return vkEndCommandBuffer(m_command_buffer) == VK_SUCCESS;
}
