#pragma once
#ifndef BONSAI_RENDERER_VULKAN_SHADER_PIPELINE_HPP
#define BONSAI_RENDERER_VULKAN_SHADER_PIPELINE_HPP

#include <vector>
#include <volk.h>
#include "bonsai/render_backend/render_backend.hpp"

class VulkanShaderPipeline : public ShaderPipeline
{
public:
    VulkanShaderPipeline(
        PipelineType pipeline_type,
        WorkgroupSize const& workgroup_size,
        VkDevice device,
        std::vector<VkDescriptorSetLayout> const& descriptor_set_layouts,
        VkPipelineLayout layout,
        VkPipeline pipeline
    );
    ~VulkanShaderPipeline() override;

    VulkanShaderPipeline(VulkanShaderPipeline const&) = delete;
    VulkanShaderPipeline &operator=(VulkanShaderPipeline const&) = delete;

private:
    VkDevice m_device = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
};

#endif //BONSAI_RENDERER_VULKAN_SHADER_PIPELINE_HPP