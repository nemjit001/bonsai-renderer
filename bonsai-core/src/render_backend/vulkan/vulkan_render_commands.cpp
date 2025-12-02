#include "vulkan_render_commands.hpp"

#include <backends/imgui_impl_vulkan.h>
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

void VulkanRenderCommands::begin_render_pass()
{
    VkRenderingInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.pNext = nullptr;
    rendering_info.flags = 0;
    rendering_info.renderArea = {};
    rendering_info.layerCount = 1;
    rendering_info.viewMask = 0;
    rendering_info.colorAttachmentCount = 0;
    rendering_info.pColorAttachments = nullptr;
    rendering_info.pDepthAttachment = nullptr;
    rendering_info.pStencilAttachment = nullptr;

    vkCmdBeginRendering(m_command_buffer, &rendering_info);
}

void VulkanRenderCommands::end_render_pass()
{
    vkCmdEndRendering(m_command_buffer);
}

void VulkanRenderCommands::set_pipeline(ShaderPipeline* pipeline)
{
    // TODO(nemjit001): Set the pipeline state based on the passed shader pipeline
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
    vkCmdDispatch(m_command_buffer, x, y, z);
}

void VulkanRenderCommands::imgui_render_draw_data(ImDrawData* draw_data)
{
    ImGui_ImplVulkan_RenderDrawData(draw_data, m_command_buffer);
}
