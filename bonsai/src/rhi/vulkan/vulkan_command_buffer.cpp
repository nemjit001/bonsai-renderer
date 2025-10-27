#include "vulkan_command_buffer.hpp"
#if BONSAI_USE_VULKAN

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer command_buffer)
    :
    ICommandBuffer(),
    m_command_buffer(command_buffer)
{
    //
}

#endif //BONSAI_USE_VULKAN