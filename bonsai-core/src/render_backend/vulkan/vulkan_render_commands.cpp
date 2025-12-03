#include "vulkan_render_commands.hpp"

#include <vector>
#include <backends/imgui_impl_vulkan.h>
#include "bonsai/core/assert.hpp"
#include "vk_check.hpp"
#include "vulkan_texture.hpp"

static VkImageMemoryBarrier2 get_image_memory_barrier(
    VulkanTexture* texture,
    VkPipelineStageFlags2 srcStageMask,
    VkAccessFlags2 srcAccessMask,
    VkPipelineStageFlags2 dstStageMask,
    VkAccessFlags2 dstAccessMask,
    VkImageLayout new_layout,
    VkImageSubresourceRange subresource_range
)
{
    VkImageMemoryBarrier2 image_barrier{};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    image_barrier.pNext = nullptr;
    image_barrier.srcStageMask = srcStageMask;
    image_barrier.srcAccessMask = srcAccessMask;
    image_barrier.dstStageMask = dstStageMask;
    image_barrier.dstAccessMask = dstAccessMask;
    image_barrier.oldLayout = texture->set_next_layout(new_layout);
    image_barrier.newLayout = texture->get_current_layout();
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.image = texture->get_image();
    image_barrier.subresourceRange = subresource_range;

    return image_barrier;
}

static VkAttachmentLoadOp get_load_op(RenderLoadOp load_op)
{
    if (load_op == RenderLoadOpLoad)
    {
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    }
    if (load_op == RenderLoadOpClear)
    {
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    }
    if (load_op == RenderLoadOpDontCare)
    {
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
    return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
}

static VkAttachmentStoreOp get_store_op(RenderStoreOp store_op)
{
    if (store_op == RenderStoreOpStore)
    {
        return VK_ATTACHMENT_STORE_OP_STORE;
    }
    else if (store_op == RenderStoreOpDontCare)
    {
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
    return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
}

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
    present_image_barrier.newLayout = vulkan_texture->get_current_layout();
    present_image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    present_image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    present_image_barrier.image = vulkan_texture->get_image();
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
    RenderAttachmentInfo* depth_target,
    RenderAttachmentInfo* stencil_target
)
{
    // Set & transition color targets
    std::vector<VkImageMemoryBarrier2> pass_image_barriers{};
    std::vector<VkRenderingAttachmentInfo> color_attachments{};
    pass_image_barriers.reserve(2 * (color_target_count + 2)); // Also used to transition depth + stencil targets and optionally resolve targets
    color_attachments.reserve(color_target_count);
    for (size_t i = 0; i < color_target_count; i++)
    {
        VulkanTexture* vk_render_target = static_cast<VulkanTexture*>(color_targets->render_target);
        VulkanTexture* vk_resolve_target = static_cast<VulkanTexture*>(color_targets->resolve_target);
        BONSAI_ASSERT(vk_render_target != nullptr && "Color target was NULL!");
        pass_image_barriers.push_back(get_image_memory_barrier(
            vk_render_target,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0, 1,
                0, 1,
            }
        ));

        if (vk_resolve_target)
        {
            pass_image_barriers.push_back(get_image_memory_barrier(
                vk_resolve_target,
                VK_PIPELINE_STAGE_2_RESOLVE_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_RESOLVE_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0, 1,
                    0, 1,
                }
            ));
        }

        VkRenderingAttachmentInfo rendering_attachment_info{};
        rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        rendering_attachment_info.pNext = nullptr;
        rendering_attachment_info.imageView = vk_render_target->get_image_view();
        rendering_attachment_info.imageLayout = vk_render_target->get_current_layout();
        rendering_attachment_info.resolveMode = vk_resolve_target ? VK_RESOLVE_MODE_AVERAGE_BIT : VK_RESOLVE_MODE_NONE;
        rendering_attachment_info.resolveImageView = vk_resolve_target ? vk_resolve_target->get_image_view() : VK_NULL_HANDLE;
        rendering_attachment_info.resolveImageLayout = vk_resolve_target ? vk_resolve_target->get_current_layout() : VK_IMAGE_LAYOUT_UNDEFINED;
        rendering_attachment_info.loadOp = get_load_op(color_targets[i].load_op);
        rendering_attachment_info.storeOp = get_store_op(color_targets[i].store_op);
        rendering_attachment_info.clearValue = VkClearValue{{{
            color_targets[i].clear_value.color.float32[0],
            color_targets[i].clear_value.color.float32[1],
            color_targets[i].clear_value.color.float32[2],
            color_targets[i].clear_value.color.float32[3],
        }}};
        color_attachments.push_back(rendering_attachment_info);
    }

    // Set and transition depth target if it exists
    VkRenderingAttachmentInfo depth_attachment{};
    if (depth_target != nullptr)
    {
        VulkanTexture* vk_render_target = static_cast<VulkanTexture*>(depth_target->render_target);
        VulkanTexture* vk_resolve_target = static_cast<VulkanTexture*>(depth_target->resolve_target);
        BONSAI_ASSERT(vk_render_target != nullptr && "Depth target was NULL!");
        pass_image_barriers.push_back(get_image_memory_barrier(
            vk_render_target,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            {
                VK_IMAGE_ASPECT_DEPTH_BIT,
                0, 1,
                0, 1,
            }
        ));

        if (vk_resolve_target)
        {
            pass_image_barriers.push_back(get_image_memory_barrier(
                vk_resolve_target,
                VK_PIPELINE_STAGE_2_RESOLVE_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_RESOLVE_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                {
                    VK_IMAGE_ASPECT_DEPTH_BIT,
                    0, 1,
                    0, 1,
                }
            ));
        }

        depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depth_attachment.pNext = nullptr;
        depth_attachment.imageView = vk_render_target->get_image_view();
        depth_attachment.imageLayout = vk_render_target->get_current_layout();
        depth_attachment.resolveMode = vk_resolve_target ? VK_RESOLVE_MODE_AVERAGE_BIT : VK_RESOLVE_MODE_NONE;
        depth_attachment.resolveImageView = vk_resolve_target ? vk_resolve_target->get_image_view() : VK_NULL_HANDLE;
        depth_attachment.resolveImageLayout = vk_resolve_target ? vk_resolve_target->get_current_layout() : VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.loadOp = get_load_op(depth_target->load_op);
        depth_attachment.storeOp = get_store_op(depth_target->store_op);
        depth_attachment.clearValue.depthStencil = {
            depth_target->clear_value.depth_stencil.depth,
            depth_target->clear_value.depth_stencil.stencil,
        };
    }

    // Set and transition stencil target if it exists
    VkRenderingAttachmentInfo stencil_attachment{};
    if (stencil_target != nullptr)
    {
        VulkanTexture* vk_render_target = static_cast<VulkanTexture*>(stencil_target->render_target);
        VulkanTexture* vk_resolve_target = static_cast<VulkanTexture*>(stencil_target->resolve_target);
        BONSAI_ASSERT(vk_render_target != nullptr && "Stencil target was NULL!");
        pass_image_barriers.push_back(get_image_memory_barrier(
            vk_render_target,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            {
                VK_IMAGE_ASPECT_STENCIL_BIT,
                0, 1,
                0, 1,
            }
        ));

        if (vk_resolve_target)
        {
            pass_image_barriers.push_back(get_image_memory_barrier(
                vk_resolve_target,
                VK_PIPELINE_STAGE_2_RESOLVE_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_RESOLVE_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                {
                    VK_IMAGE_ASPECT_STENCIL_BIT,
                    0, 1,
                    0, 1,
                }
            ));
        }

        stencil_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        stencil_attachment.pNext = nullptr;
        stencil_attachment.imageView = vk_render_target->get_image_view();
        stencil_attachment.imageLayout = vk_render_target->get_current_layout();
        stencil_attachment.resolveMode = vk_resolve_target ? VK_RESOLVE_MODE_AVERAGE_BIT : VK_RESOLVE_MODE_NONE;
        stencil_attachment.resolveImageView = vk_resolve_target ? vk_resolve_target->get_image_view() : VK_NULL_HANDLE;
        stencil_attachment.resolveImageLayout = vk_resolve_target ? vk_resolve_target->get_current_layout() : VK_IMAGE_LAYOUT_UNDEFINED;
        stencil_attachment.loadOp = get_load_op(stencil_target->load_op);
        stencil_attachment.storeOp = get_store_op(stencil_target->store_op);
        stencil_attachment.clearValue.depthStencil = {
            depth_target->clear_value.depth_stencil.depth,
            depth_target->clear_value.depth_stencil.stencil,
        };
    }

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
    rendering_info.colorAttachmentCount = static_cast<uint32_t>(color_attachments.size());
    rendering_info.pColorAttachments = color_attachments.data();
    rendering_info.pDepthAttachment = depth_target ? &depth_attachment : nullptr;
    rendering_info.pStencilAttachment = stencil_target ? &stencil_attachment : nullptr;

    VkDependencyInfo pass_dependency_info{};
    pass_dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    pass_dependency_info.pNext = nullptr;
    pass_dependency_info.imageMemoryBarrierCount = static_cast<uint32_t>(pass_image_barriers.size());
    pass_dependency_info.pImageMemoryBarriers = pass_image_barriers.data();

    vkCmdPipelineBarrier2(m_command_buffer, &pass_dependency_info);
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
