#include "rendering/renderer.hpp"
#if BONSAI_USE_VULKAN

#include <volk.h>
#include "core/die.hpp"
#include "render_backend_vulkan.hpp"

Renderer::Renderer(RenderBackend* render_backend)
    :
    m_render_backend(render_backend)
{
    //
}

void Renderer::on_resize(uint32_t width, uint32_t height)
{
    m_render_backend->on_resize(width, height);
}

void Renderer::render(World const& render_world, double delta)
{
    RenderBackendImpl const* backend_impl = m_render_backend->get_raw();
    FrameState const* frame_state = m_render_backend->start_frame();
    if (frame_state == nullptr)
    {   // Failed to acquire frame, but recoverable
        return;
    }
    vkResetFences(backend_impl->device, 1, &frame_state->frame_ready);

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = nullptr;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    vkResetCommandBuffer(frame_state->frame_commands, 0);
    if (vkBeginCommandBuffer(frame_state->frame_commands, &command_buffer_begin_info) != VK_SUCCESS)
    {
        bonsai::die("Failed to start command recording for frame ({})", backend_impl->frame_index);
    }

    VkImageMemoryBarrier2 swap_render_barrier{};
    swap_render_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    swap_render_barrier.pNext = nullptr;
    swap_render_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    swap_render_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    swap_render_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    swap_render_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    swap_render_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    swap_render_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    swap_render_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    swap_render_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    swap_render_barrier.image = backend_impl->swap_images[frame_state->swap_image_idx];
    swap_render_barrier.subresourceRange = VkImageSubresourceRange {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, 1,
        0, 1,
    };

    VkDependencyInfo swap_render_dependencies{};
    swap_render_dependencies.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    swap_render_dependencies.pNext = nullptr;
    swap_render_dependencies.imageMemoryBarrierCount = 1;
    swap_render_dependencies.pImageMemoryBarriers = &swap_render_barrier;
    vkCmdPipelineBarrier2(frame_state->frame_commands, &swap_render_dependencies);

    VkRenderingAttachmentInfo swap_color_target{};
    swap_color_target.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    swap_color_target.pNext = nullptr;
    swap_color_target.imageView = backend_impl->swap_image_views[frame_state->swap_image_idx];
    swap_color_target.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    swap_color_target.resolveMode = VK_RESOLVE_MODE_NONE;
    swap_color_target.resolveImageView = VK_NULL_HANDLE;
    swap_color_target.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    swap_color_target.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    swap_color_target.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    swap_color_target.clearValue = VkClearValue{{{ 0.0F, 0.0F, 0.0F, 0.0F }}};

    VkRenderingInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.pNext = nullptr;
    rendering_info.flags = 0;
    rendering_info.renderArea = VkRect2D { VkOffset2D{ 0, 0 }, backend_impl->swap_config.extent };
    rendering_info.layerCount = 1;
    rendering_info.viewMask = 0;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &swap_color_target;
    rendering_info.pDepthAttachment = nullptr;
    rendering_info.pStencilAttachment = nullptr;
    vkCmdBeginRendering(frame_state->frame_commands, &rendering_info);
    vkCmdEndRendering(frame_state->frame_commands);

    VkImageMemoryBarrier2 swap_present_barrier{};
    swap_present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    swap_present_barrier.pNext = nullptr;
    swap_present_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    swap_present_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    swap_present_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    swap_present_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    swap_present_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    swap_present_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    swap_present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    swap_present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    swap_present_barrier.image = backend_impl->swap_images[frame_state->swap_image_idx];
    swap_present_barrier.subresourceRange = VkImageSubresourceRange {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, 1,
        0, 1,
    };

    VkDependencyInfo swap_present_dependencies{};
    swap_present_dependencies.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    swap_present_dependencies.pNext = nullptr;
    swap_present_dependencies.dependencyFlags = 0;
    swap_present_dependencies.imageMemoryBarrierCount = 1;
    swap_present_dependencies.pImageMemoryBarriers = &swap_present_barrier;
    vkCmdPipelineBarrier2(frame_state->frame_commands, &swap_present_dependencies);

    if (vkEndCommandBuffer(frame_state->frame_commands) != VK_SUCCESS)
    {
        bonsai::die("Failed to end command recording for frame ({})", backend_impl->frame_index);
    }

    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.pWaitSemaphores = &frame_state->swap_available;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame_state->frame_commands;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &frame_state->rendering_finished;

    if (vkQueueSubmit(backend_impl->graphics_queue, 1, &submit_info, frame_state->frame_ready) != VK_SUCCESS)
    {
        bonsai::die("Failed to submit rendered frame ({})", backend_impl->frame_index);
    }

    m_render_backend->present(frame_state);
}

#endif //BONSAI_USE_VULKAN