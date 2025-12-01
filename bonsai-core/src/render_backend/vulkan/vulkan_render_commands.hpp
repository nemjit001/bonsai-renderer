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
    void set_pipeline(ShaderPipeline* pipeline) override;
    void bind_uniform(char const* name, RenderBuffer* buffer, size_t size, size_t offset) override;
    void bind_buffer(char const* name, RenderBuffer* buffer, size_t size, size_t offset) override;
    void bind_texture(char const* name, RenderTexture* texture) override;
    void dispatch(uint32_t x, uint32_t y, uint32_t z) override;

private:
    VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
};

#endif //BONSAI_RENDERER_VULKAN_RENDER_COMMANDS_HPP