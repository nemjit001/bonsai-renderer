#include "renderer.hpp"

#include "platform/logger.hpp"
#include "core/die.hpp"

Renderer::Renderer(Surface* surface)
{
    m_rhi_instance = rhi::create_instance();
    if (!m_rhi_instance)
    {
        bonsai::die("Failed to create RHI instance");
    }

    RenderDeviceDesc render_device_desc{};
    render_device_desc.compatible_surface = surface;
    render_device_desc.frames_in_flight = 1;
    m_render_device = m_rhi_instance->create_render_device(render_device_desc);
    if (!m_render_device)
    {
        bonsai::die("Failed to create Render Device");
    }

    uint32_t width = 0, height = 0;
    surface->get_size(width, height);

    SwapChainDesc swap_chain_desc{};
    swap_chain_desc.surface = surface;
    swap_chain_desc.image_count = 3;
    swap_chain_desc.format = Format::RGBA8_UNORM; // Should be supported on all platforms
    swap_chain_desc.width = width;
    swap_chain_desc.height = height;
    swap_chain_desc.usage = TextureUsageColorAttachment;
    swap_chain_desc.present_mode = SwapPresentMode::FiFo;
    m_swap_chain = m_render_device->create_swap_chain(swap_chain_desc);
    if (!m_swap_chain)
    {
        bonsai::die("Failed to create Swap Chain");
    }

    m_command_allocator = m_render_device->create_command_allocator(CommandQueueType::Direct);
    m_frame_commands = m_command_allocator->create_command_buffer();
}

Renderer::~Renderer()
{
    m_render_device->wait_idle();
}

void Renderer::on_resize(uint32_t width, uint32_t height)
{
    m_render_device->wait_idle();

    SwapChainDesc const swap_chain_desc = m_swap_chain->get_desc();
    m_swap_chain->resize_swap_buffers(width, height, swap_chain_desc.present_mode);
}

void Renderer::render()
{
    m_render_device->wait_for_queue_idle(CommandQueueType::Direct);
    if (!m_swap_chain->acquire_next_image())
    {
        BONSAI_LOG_ERROR("Failed to acquire swap chain image");
        return;
    }

    uint32_t swap_image_idx = m_swap_chain->current_image_idx();
    TextureHandle swap_texture = m_swap_chain->get_swap_image(swap_image_idx); // TODO(nemjit001): Cache swap image handles to avoid allocations
    TextureViewHandle swap_texture_view = swap_texture->create_view(nullptr);

    if (!m_frame_commands->begin())
    {
        BONSAI_LOG_ERROR("Failed to begin frame command buffer");
        return;
    }

    RenderAttachmentDesc color_attachment{};
    color_attachment.view = swap_texture_view;
    color_attachment.layout = TextureLayout::ColorAttachment;
    color_attachment.load_op = AttachmentLoadOp::Clear;
    color_attachment.store_op = AttachmentStoreOp::Store;
    color_attachment.clear_value = ClearValue{{{ 0.0F, 0.0F, 0.0F, 0.0F }}};

    RenderPassDesc render_pass_desc{};
    render_pass_desc.render_area = {{ 0, 0 }, { swap_texture->width(), swap_texture->height() }};
    render_pass_desc.color_attachment_count = 1;
    render_pass_desc.color_attachments = &color_attachment;
    render_pass_desc.depth_attachment = nullptr;
    render_pass_desc.stencil_attachment = nullptr;
    m_frame_commands->begin_render_pass(render_pass_desc);
    m_frame_commands->end_render_pass();

    if (!m_frame_commands->close())
    {
        BONSAI_LOG_ERROR("Failed to close frame command buffer");
        return;
    }

    m_render_device->submit(CommandQueueType::Direct, 1, &m_frame_commands);
    if (!m_swap_chain->present())
    {
        BONSAI_LOG_ERROR("Failed to present swap chain image");
    }
}
