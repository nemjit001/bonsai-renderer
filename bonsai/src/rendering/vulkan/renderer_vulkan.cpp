#include "rendering/renderer.hpp"
#if BONSAI_USE_VULKAN

#include <volk.h>
#include "core/die.hpp"
#include "render_backend_vulkan.hpp"

Renderer::Renderer(Surface const* surface)
    :
    m_render_backend(new RenderBackend(surface))
{
    //
}

Renderer::~Renderer()
{
    delete m_render_backend;
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

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = nullptr;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    vkResetCommandBuffer(frame_state->frame_commands, 0);
    if (vkBeginCommandBuffer(frame_state->frame_commands, &command_buffer_begin_info) != VK_SUCCESS)
    {
        bonsai::die("Failed to start command recording for frame ({})", frame_state->frame_idx);
    }

    VkImageMemoryBarrier2 swap_present_barrier{};
    swap_present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    swap_present_barrier.pNext = nullptr;
    swap_present_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    swap_present_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    swap_present_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    swap_present_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    swap_present_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    swap_present_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    swap_present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    swap_present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    swap_present_barrier.image = backend_impl->swap_images[frame_state->swap_image_idx];
    swap_present_barrier.subresourceRange = VkImageSubresourceRange{
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, 1,
        0, 1,
    };

    VkDependencyInfo swap_dependencies{};
    swap_dependencies.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    swap_dependencies.pNext = nullptr;
    swap_dependencies.dependencyFlags = 0;
    swap_dependencies.imageMemoryBarrierCount = 1;
    swap_dependencies.pImageMemoryBarriers = &swap_present_barrier;

    vkCmdPipelineBarrier2(frame_state->frame_commands, &swap_dependencies);

    if (vkEndCommandBuffer(frame_state->frame_commands) != VK_SUCCESS)
    {
        bonsai::die("Failed to end command recording for frame ({})", frame_state->frame_idx);
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
        bonsai::die("Failed to submit rendered frame ({})", frame_state->frame_idx);
    }

    m_render_backend->present(frame_state);
}

#endif //BONSAI_USE_VULKAN