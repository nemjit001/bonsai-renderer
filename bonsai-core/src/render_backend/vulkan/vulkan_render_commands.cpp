#include "vulkan_render_commands.hpp"

#include <vector>
#include <backends/imgui_impl_vulkan.h>
#include "vk_check.hpp"
#include "vulkan_texture.hpp"

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

void VulkanRenderCommands::mark_for_present(RenderTexture* texture)
{
    VulkanTexture* vulkan_texture = static_cast<VulkanTexture*>(texture);

    VkImageMemoryBarrier2 present_image_barrier{};
    present_image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    present_image_barrier.pNext = nullptr;
    present_image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    present_image_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    present_image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    present_image_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    present_image_barrier.oldLayout = vulkan_texture->set_next_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    present_image_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    present_image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    present_image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    present_image_barrier.image = vulkan_texture->get_image();
    // present_image_barrier.subresourceRange = vulkan_texture->get_subresource_range(); // TODO(nemjit001): Fetch this from vulkan texture
    present_image_barrier.subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, 1,
        0, 1,
    };

    VkDependencyInfo present_dependency{};
    present_dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    present_dependency.pNext = nullptr;
    present_dependency.imageMemoryBarrierCount = 1;
    present_dependency.pImageMemoryBarriers = &present_image_barrier;

    vkCmdPipelineBarrier2(m_command_buffer, &present_dependency);
}

void VulkanRenderCommands::begin_render_pass(
    RenderRect2D render_area,
    RenderAttachmentInfo* color_targets,
    size_t color_target_count,
    RenderAttachmentInfo* depth_attachment,
    RenderAttachmentInfo* stencil_attachment
)
{
    // TODO(nemjit001):
    // - [ ] Fill out rendering info structure, tracking images in the wrong layout
    // - [ ] Transition all targets to appropriate layouts, updating the image layout for VulkanTextures

    VkRenderingInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.pNext = nullptr;
    rendering_info.flags = 0;
    rendering_info.renderArea = {
        {render_area.offset.x, render_area.offset.y },
        { render_area.extent.width, render_area.extent.height }
    };
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
