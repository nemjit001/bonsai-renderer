#pragma once
#ifndef BONSAI_RENDERER_VULKAN_RENDER_COMMANDS_HPP
#define BONSAI_RENDERER_VULKAN_RENDER_COMMANDS_HPP

#include <volk.h>
#include "bonsai/render_backend/render_backend.hpp"

class VulkanRenderCommands : public RenderCommands
{
public:
    VulkanRenderCommands() = default;
    explicit VulkanRenderCommands(VkCommandBuffer command_buffer);
    ~VulkanRenderCommands() override = default;

    bool begin() override;

    bool end() override;

    void mark_for_present(RenderTexture* texture) override;

    void begin_render_pass(
        RenderRect2D render_area,
        RenderAttachmentInfo* color_targets,
        size_t color_target_count,
        RenderAttachmentInfo* depth_target,
        RenderAttachmentInfo* stencil_target
    ) override;

    void end_render_pass() override;

    void set_pipeline(ShaderPipeline* pipeline) override;

    void set_primitive_topology(PrimitiveTopologyType primitive_topology) override;

    void set_viewports(size_t count, RenderViewport* viewports) override;

    void set_scissor_rects(size_t count, RenderRect2D* scissor_rects) override;

    void bind_vertex_buffers(uint32_t base_binding, size_t count, RenderBuffer** buffers, size_t* offsets) override;

    void bind_index_buffer(RenderBuffer* buffer, size_t offset, IndexType index_type) override;

    void draw_instanced(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;

    void draw_indexed_instanced(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) override;

    void dispatch(uint32_t x, uint32_t y, uint32_t z) override;

    void imgui_render_draw_data(ImDrawData* draw_data) override;

private:
    VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
};

#endif //BONSAI_RENDERER_VULKAN_RENDER_COMMANDS_HPP