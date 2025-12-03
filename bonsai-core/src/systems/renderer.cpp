#include "bonsai/systems/renderer.hpp"

#include "bonsai/core/fatal_exit.hpp"

Renderer::Renderer(RenderBackend* render_backend)
    :
    m_render_backend(render_backend)
{
    //
}

void Renderer::on_resize(uint32_t width, uint32_t height)
{
    m_render_backend->wait_idle();
    m_render_backend->reconfigure_swap_chain(width, height);
}

void Renderer::render()
{
    if (m_render_backend->new_frame() == RenderBackendFrameResult::FatalError)
    {
        BONSAI_FATAL_EXIT("Failed to start renderer frame\n");
    }
    ImGui::NewFrame();
    // TODO(nemjit001): render GUI here (using app specific function?)
    ImGui::ShowDemoWindow();
    ImGui::Render();

    RenderCommands* frame_commands = m_render_backend->get_frame_commands();
    RenderTexture* swap_texture = m_render_backend->get_current_swap_texture();
    if (!frame_commands->begin())
    {
        BONSAI_FATAL_EXIT("Failed to start renderer frame command recording\n");
    }

    RenderExtent3D const swap_extent = swap_texture->extent();
    RenderRect2D render_area{};
    render_area.offset = { 0, 0 };
    render_area.extent = { swap_extent.width, swap_extent.height  };

    RenderAttachmentInfo color_attachment{};
    color_attachment.render_target = swap_texture;
    color_attachment.load_op = RenderLoadOpClear;
    color_attachment.store_op = RenderStoreOpStore;
    color_attachment.clear_value = RenderClearValue{{{ 0.0F, 0.0F, 0.0F, 0.0F }}};

    frame_commands->begin_render_pass(render_area, &color_attachment, 1, nullptr, nullptr);
    frame_commands->imgui_render_draw_data(ImGui::GetDrawData());
    frame_commands->end_render_pass();
    frame_commands->mark_for_present(swap_texture);

    if (!frame_commands->end())
    {
        BONSAI_FATAL_EXIT("Failed to end renderer frame command recording\n");
    }

    if (m_render_backend->end_frame() == RenderBackendFrameResult::FatalError)
    {
        BONSAI_FATAL_EXIT("Failed to end renderer frame\n");
    }
}
