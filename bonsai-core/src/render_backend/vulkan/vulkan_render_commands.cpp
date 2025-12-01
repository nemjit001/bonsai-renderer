#include "vulkan_render_commands.hpp"

#include "vk_check.hpp"

VulkanRenderCommands::VulkanRenderCommands(VkCommandBuffer command_buffer)
    :
    m_command_buffer(command_buffer)
{
    //
}

bool VulkanRenderCommands::begin()
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    return VK_SUCCEEDED(vkBeginCommandBuffer(m_command_buffer, &begin_info));
}

bool VulkanRenderCommands::end()
{
    return VK_SUCCEEDED(vkEndCommandBuffer(m_command_buffer));
}

void VulkanRenderCommands::set_pipeline(ShaderPipeline* pipeline)
{
    // TODO(nemjit001): Set the pipeline state based on the passed shader pipeline -> WILL require downcasting
}

void VulkanRenderCommands::bind_uniform(char const* name, RenderBuffer* buffer, size_t size, size_t offset)
{
    // TODO(nemjit001): Bind a uniform to the descriptor set that uses the specified named binding
}

void VulkanRenderCommands::bind_buffer(char const* name, RenderBuffer* buffer, size_t size, size_t offset)
{
    // TODO(nemjit001): Bind a buffer to the descriptor set that uses the specified named binding
}

void VulkanRenderCommands::bind_texture(char const* name, RenderTexture* texture)
{
    // TODO(nemjit001): Bind a texture to the descriptor set that uses the specified named binding
}

void VulkanRenderCommands::dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    /// TODO(nemjit001): Should we check if last bound pipeline was a compute pipeline?
    vkCmdDispatch(m_command_buffer, x, y, z);
}
