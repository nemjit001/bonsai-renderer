#include "bonsai/systems/renderer.hpp"

#include <cstring>
#include "bonsai/core/fatal_exit.hpp"

// TODO(nemjit001): Add shader asset type support w/ loading from disk
static char const* SHADER_CODE = R"(
struct VertexInput
{
    float3 position : POSITION0;
    float2 tex_coord : TEXCOORD0;
}

struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 tex_coord : TEXCOORD0;
}

[[shader("vertex")]]
VertexOutput VSmain(VertexInput input)
{
    VertexOutput result;
    result.position = float4(input.position, 1);
    result.tex_coord = input.tex_coord;
    return result;
}

[[shader("pixel")]]
float4 PSmain(VertexOutput input) : SV_TARGET0
{
    return float4(input.tex_coord, 0, 1);
}
)";

Renderer::Renderer(RenderBackend* render_backend)
    :
    m_render_backend(render_backend)
{
    m_swap_extent = m_render_backend->get_swap_extent();

    ShaderCodeBlob vertex_shader{};
    vertex_shader.entrypoint = "VSmain";
    vertex_shader.code = SHADER_CODE;
    vertex_shader.code_size = std::strlen(SHADER_CODE);

    ShaderCodeBlob fragment_shader{};
    fragment_shader.entrypoint = "PSmain";
    fragment_shader.code = SHADER_CODE;
    fragment_shader.code_size = std::strlen(SHADER_CODE);

    GraphicsPipelineDescriptor pipeline_descriptor{};
    pipeline_descriptor.vertex_shader = &vertex_shader;
    pipeline_descriptor.fragment_shader = &fragment_shader;

    m_shader_pipeline = m_render_backend->create_graphics_pipeline(pipeline_descriptor);
}

Renderer::~Renderer()
{
    delete m_shader_pipeline;
}

void Renderer::on_resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0)
    {
        m_swap_extent = { 0, 0 };
        return;
    }

    m_render_backend->wait_idle();
    m_render_backend->reconfigure_swap_chain(width, height);
    m_swap_extent = { width, height };
}

void Renderer::render()
{
    if (m_swap_extent.width == 0 || m_swap_extent.height == 0)
    {
        return;
    }

    if (m_render_backend->new_frame() == RenderBackendFrameResult::FatalError)
    {
        BONSAI_FATAL_EXIT("Failed to start renderer frame\n");
    }

    ImGui::NewFrame();
    // TODO(nemjit001): render GUI here (using app specific function?)
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
    frame_commands->set_pipeline(m_shader_pipeline);
    // TODO(nemjit001): Record draw commands
    frame_commands->end_render_pass();

    RenderAttachmentInfo imgui_color_attachment{};
    imgui_color_attachment.render_target = swap_texture;
    imgui_color_attachment.load_op = RenderLoadOpLoad;
    imgui_color_attachment.store_op = RenderStoreOpStore;
    imgui_color_attachment.clear_value = RenderClearValue{{{ 0.0F, 0.0F, 0.0F, 0.0F }}};

    frame_commands->begin_render_pass(render_area, &imgui_color_attachment, 1, nullptr, nullptr);
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
