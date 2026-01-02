#include "vulkan_render_commands.hpp"

#include <vector>
#include <backends/imgui_impl_vulkan.h>
#include "bonsai/core/assert.hpp"
#include "enum_conversion.hpp"
#include "vk_check.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_shader_pipeline.hpp"
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
    VulkanTexture* vulkan_texture = dynamic_cast<VulkanTexture*>(texture);
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
        vulkan_texture->get_image_aspect(),
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
        VulkanTexture* vk_render_target = dynamic_cast<VulkanTexture*>(color_targets->render_target);
        VulkanTexture* vk_resolve_target = dynamic_cast<VulkanTexture*>(color_targets->resolve_target);
        BONSAI_ASSERT(vk_render_target != nullptr && "Color target was NULL!");
        pass_image_barriers.push_back(get_image_memory_barrier(
            vk_render_target,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            {
                vk_render_target->get_image_aspect(),
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
                    vk_resolve_target->get_image_aspect(),
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
        rendering_attachment_info.loadOp = get_vulkan_load_op(color_targets[i].load_op);
        rendering_attachment_info.storeOp = get_vulkan_store_op(color_targets[i].store_op);
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
        VulkanTexture* vk_render_target = dynamic_cast<VulkanTexture*>(depth_target->render_target);
        VulkanTexture* vk_resolve_target = dynamic_cast<VulkanTexture*>(depth_target->resolve_target);
        BONSAI_ASSERT(vk_render_target != nullptr && "Depth target was NULL!");
        pass_image_barriers.push_back(get_image_memory_barrier(
            vk_render_target,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            {
                vk_render_target->get_image_aspect(),
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
                    vk_resolve_target->get_image_aspect(),
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
        depth_attachment.loadOp = get_vulkan_load_op(depth_target->load_op);
        depth_attachment.storeOp = get_vulkan_store_op(depth_target->store_op);
        depth_attachment.clearValue.depthStencil = {
            depth_target->clear_value.depth_stencil.depth,
            depth_target->clear_value.depth_stencil.stencil,
        };
    }

    // Set and transition stencil target if it exists
    VkRenderingAttachmentInfo stencil_attachment{};
    if (stencil_target != nullptr)
    {
        VulkanTexture* vk_render_target = dynamic_cast<VulkanTexture*>(stencil_target->render_target);
        VulkanTexture* vk_resolve_target = dynamic_cast<VulkanTexture*>(stencil_target->resolve_target);
        BONSAI_ASSERT(vk_render_target != nullptr && "Stencil target was NULL!");
        pass_image_barriers.push_back(get_image_memory_barrier(
            vk_render_target,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            {
                vk_render_target->get_image_aspect(),
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
                    vk_resolve_target->get_image_aspect(),
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
        stencil_attachment.loadOp = get_vulkan_load_op(stencil_target->load_op);
        stencil_attachment.storeOp = get_vulkan_store_op(stencil_target->store_op);
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
    VulkanShaderPipeline const* vk_pipeline = dynamic_cast<VulkanShaderPipeline*>(pipeline);
    vkCmdBindPipeline(m_command_buffer, vk_pipeline->get_bind_point(), vk_pipeline->get_pipeline());
}

void VulkanRenderCommands::set_primitive_topology(PrimitiveTopologyType primitive_topology)
{
    vkCmdSetPrimitiveTopology(m_command_buffer, get_vulkan_topology(primitive_topology));
}

void VulkanRenderCommands::set_viewports(size_t count, RenderViewport* viewports)
{
    BONSAI_ASSERT(count == 1 && "Viewport count must be 1");
    VkViewport const vk_viewport{
        viewports[0].x,
        viewports[0].y,
        viewports[0].width,
        viewports[0].height,
        viewports[0].min_depth,
        viewports[0].max_depth,
    };

    vkCmdSetViewport(m_command_buffer, 0, 1, &vk_viewport);
}

void VulkanRenderCommands::set_scissor_rects(size_t count, RenderRect2D* scissor_rects)
{
    BONSAI_ASSERT(count == 1 && "Scissor rect count must be 1");
    VkRect2D const vk_scissor_rect{
        { scissor_rects[0].offset.x, scissor_rects[0].offset.y },
        { scissor_rects[0].extent.width, scissor_rects[0].extent.height },
    };

    vkCmdSetScissor(m_command_buffer, 0, 1, &vk_scissor_rect);
}

void VulkanRenderCommands::bind_vertex_buffers(size_t base_binding, size_t count, RenderBuffer** buffers, size_t* offsets)
{
    std::vector<VkBuffer> vertex_buffers;
    vertex_buffers.resize(count);
    for (size_t i = 0; i < count; i++)
    {
        VulkanBuffer const* vk_buffer = dynamic_cast<VulkanBuffer*>(buffers[i]);
        vertex_buffers[i] = vk_buffer->get_buffer();
    }

    vkCmdBindVertexBuffers(
        m_command_buffer,
        static_cast<uint32_t>(base_binding),
        static_cast<uint32_t>(count),
        vertex_buffers.data(),
        offsets
    );
}

void VulkanRenderCommands::bind_index_buffer(RenderBuffer* buffer, size_t offset, IndexType index_type)
{
    VulkanBuffer const* vk_buffer = dynamic_cast<VulkanBuffer*>(buffer);
    vkCmdBindIndexBuffer(m_command_buffer, vk_buffer->get_buffer(), offset, get_vulkan_index_type(index_type));
}

void VulkanRenderCommands::draw_instanced(size_t vertex_count, size_t instance_count, size_t first_vertex, size_t first_instance)
{
    vkCmdDraw(
        m_command_buffer,
        static_cast<uint32_t>(vertex_count),
        static_cast<uint32_t>(instance_count),
        static_cast<uint32_t>(first_vertex),
        static_cast<uint32_t>(first_instance)
    );
}

void VulkanRenderCommands::dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    vkCmdDispatch(m_command_buffer, x, y, z);
}

void VulkanRenderCommands::imgui_render_draw_data(ImDrawData* draw_data)
{
    ImGui_ImplVulkan_RenderDrawData(draw_data, m_command_buffer);
}
