#include "bonsai/systems/renderer.hpp"

#include "bonsai/core/fatal_exit.hpp"

Renderer::Renderer(RenderBackend* render_backend)
    :
    m_render_backend(render_backend)
{
    //
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

    // FIXME(nemjit001): set render pass attachments & clear values
    frame_commands->begin_render_pass();
    frame_commands->imgui_render_draw_data(ImGui::GetDrawData());
    frame_commands->end_render_pass();

    if (!frame_commands->end())
    {
        BONSAI_FATAL_EXIT("Failed to end renderer frame command recording\n");
    }

    if (m_render_backend->end_frame() == RenderBackendFrameResult::FatalError)
    {
        BONSAI_FATAL_EXIT("Failed to end renderer frame\n");
    }
}
