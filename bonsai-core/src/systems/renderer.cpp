#include "bonsai/systems/renderer.hpp"

#include <cstring>
#include "bonsai/core/fatal_exit.hpp"

// TODO(nemjit001): Add shader asset type support w/ loading from disk
static char const* SHADER_CODE = R"(
struct VertexInput
{
    float3 position     : POSITION0;
    float3 color        : COLOR0;
    float2 tex_coord    : TEXCOORD0;
};

struct VertexOutput
{
    float4 position     : SV_POSITION;
    float3 color        : COLOR0;
    float2 tex_coord    : TEXCOORD0;
};

[shader("vertex")]
VertexOutput VSmain(VertexInput input)
{
    VertexOutput result;
    result.position = float4(input.position, 1);
    result.color = input.color;
    result.tex_coord = input.tex_coord;
    return result;
}

[shader("pixel")]
float4 PSmain(VertexOutput input) : SV_TARGET0
{
    return float4(input.color, 1);
}
)";

static float VERTEX_DATA[] = {
    /* position */ -0.5F, -0.5F, 0.0F, /* color */ 1.0F, 0.0F, 0.0F , /* tex coord */ 0.0F, 0.0F,
    /* position */ -0.5F,  0.5F, 0.0F, /* color */ 0.0F, 1.0F, 0.0F , /* tex coord */ 0.0F, 1.0F,
    /* position */  0.5F,  0.5F, 0.0F, /* color */ 0.0F, 0.0F, 1.0F , /* tex coord */ 1.0F, 0.0F,
    /* position */  0.5F, -0.5F, 0.0F, /* color */ 1.0F, 1.0F, 1.0F , /* tex coord */ 1.0F, 1.0F,
};

static uint16_t INDEX_DATA[] = {
    0, 1, 2,
    2, 3, 0,
};

Renderer::Renderer(RenderBackend* render_backend)
    :
    m_render_backend(render_backend)
{
    m_swap_extent = m_render_backend->get_swap_extent();

    ShaderSource vertex_shader{};
    vertex_shader.source_kind = ShaderSourceKindInline;
    vertex_shader.entrypoint = "VSmain";
    vertex_shader.shader_source = SHADER_CODE;

    ShaderSource fragment_shader{};
    fragment_shader.source_kind = ShaderSourceKindInline;
    fragment_shader.entrypoint = "PSmain";
    fragment_shader.shader_source = SHADER_CODE;

    GraphicsPipelineDescriptor pipeline_descriptor{};
    pipeline_descriptor.vertex_shader = &vertex_shader;
    pipeline_descriptor.fragment_shader = &fragment_shader;

    pipeline_descriptor.input_assembly_state.primitive_topology = PrimitiveTopologyTypeTriangleList;
    pipeline_descriptor.input_assembly_state.strip_cut_value = IndexBufferStripCutValueDisabled;

    pipeline_descriptor.rasterization_state.polygon_mode = PolygonModeFill;
    pipeline_descriptor.rasterization_state.cull_mode = CullModeNone;
    pipeline_descriptor.rasterization_state.front_face_counter_clockwise = true;
    pipeline_descriptor.rasterization_state.depth_bias = 0.0F;
    pipeline_descriptor.rasterization_state.depth_bias_clamp = 0.0F;
    pipeline_descriptor.rasterization_state.depth_bias_slope_factor = 0.0F;

    pipeline_descriptor.multisample_state.sample_count = SampleCount1Sample;
    pipeline_descriptor.multisample_state.sample_mask = nullptr;

    pipeline_descriptor.depth_stencil_state.depth_test = false;
    pipeline_descriptor.depth_stencil_state.depth_write = false;
    pipeline_descriptor.depth_stencil_state.depth_compare_op = CompareOpLess;
    pipeline_descriptor.depth_stencil_state.depth_bounds_test = false;
    pipeline_descriptor.depth_stencil_state.stencil_test = false;
    pipeline_descriptor.depth_stencil_state.front = {};
    pipeline_descriptor.depth_stencil_state.back = {};

    pipeline_descriptor.color_blend_state.logic_op_enable = false;
    pipeline_descriptor.color_blend_state.logic_op = LogicOpClear;
    pipeline_descriptor.color_blend_state.attachments[0].blend_enable = false;
    pipeline_descriptor.color_blend_state.attachments[0].color_write_mask = ColorComponentAll;

    pipeline_descriptor.color_attachment_count = 1;
    pipeline_descriptor.color_attachment_formats[0] = m_render_backend->get_swap_format();
    pipeline_descriptor.depth_stencil_attachment_format = RenderFormatUndefined;

    m_shader_pipeline = m_render_backend->create_graphics_pipeline(pipeline_descriptor);
    if (!m_shader_pipeline)
    {
        BONSAI_FATAL_EXIT("Failed to compile simple shader pipeline\n");
    }

    m_vertex_buffer = m_render_backend->create_buffer(sizeof(VERTEX_DATA), RenderBufferUsageVertexBuffer, true);
    m_index_buffer = m_render_backend->create_buffer(sizeof(INDEX_DATA), RenderBufferUsageIndexBuffer, true);

    void* vertex_buffer_data = nullptr;
    if (!m_vertex_buffer || !m_vertex_buffer->map(&vertex_buffer_data, m_vertex_buffer->size(), 0))
    {
        BONSAI_FATAL_EXIT("Failed to create or map Vertex buffer\n");
    }
    memcpy(vertex_buffer_data, VERTEX_DATA, sizeof(VERTEX_DATA));
    m_vertex_buffer->unmap();

    void* index_buffer_data = nullptr;
    if (!m_index_buffer || !m_index_buffer->map(&index_buffer_data, m_index_buffer->size(), 0))
    {
        BONSAI_FATAL_EXIT("Failed to create or map Index buffer\n");
    }
    memcpy(index_buffer_data, INDEX_DATA, sizeof(INDEX_DATA));
    m_index_buffer->unmap();
}

Renderer::~Renderer()
{
    m_render_backend->wait_idle();
    delete m_index_buffer;
    delete m_vertex_buffer;
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
    m_swap_extent = m_render_backend->get_swap_extent();
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
    ImGui::EndFrame();
    ImGui::Render();

    RenderCommands* frame_commands = m_render_backend->get_frame_commands();
    RenderTexture* swap_texture = m_render_backend->get_current_swap_texture();
    if (!frame_commands->begin())
    {
        BONSAI_FATAL_EXIT("Failed to start renderer frame command recording\n");
    }

    RenderRect2D render_area{};
    render_area.offset = { 0, 0 };
    render_area.extent = { m_swap_extent.width, m_swap_extent.height  };

    RenderAttachmentInfo color_attachment{};
    color_attachment.render_target = swap_texture;
    color_attachment.load_op = RenderLoadOpClear;
    color_attachment.store_op = RenderStoreOpStore;
    color_attachment.clear_value = RenderClearValue{{{ 0.0F, 0.0F, 0.0F, 0.0F }}};

    frame_commands->begin_render_pass(render_area, &color_attachment, 1, nullptr, nullptr);
    frame_commands->set_pipeline(m_shader_pipeline);

    RenderViewport viewport{ 0.0F, 0.0F, static_cast<float>(m_swap_extent.width), static_cast<float>(m_swap_extent.height), 0.0F, 1.0F };
    RenderRect2D scissor{ { 0, 0 }, { m_swap_extent.width, m_swap_extent.height } };
    frame_commands->set_viewports(1, &viewport);
    frame_commands->set_scissor_rects(1, &scissor);
    frame_commands->set_primitive_topology(PrimitiveTopologyTypeTriangleList);

    size_t offsets[] = { 0 };
    frame_commands->bind_vertex_buffers(0, 1, &m_vertex_buffer, offsets);
    frame_commands->bind_index_buffer(m_index_buffer, 0, IndexTypeUint16);
    frame_commands->draw_indexed_instanced(6, 1, 0, 0, 0);
    frame_commands->end_render_pass();

    RenderAttachmentInfo imgui_color_attachment{};
    imgui_color_attachment.render_target = swap_texture;
    imgui_color_attachment.load_op = RenderLoadOpLoad;
    imgui_color_attachment.store_op = RenderStoreOpStore;
    imgui_color_attachment.clear_value = {};

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
