#include "vulkan_shader_pipeline.hpp"

VulkanShaderPipeline::VulkanShaderPipeline(PipelineType pipeline_type, WorkgroupSize const& workgroup_size, VkDevice device, VkPipelineLayout layout, VkPipeline pipeline)
    :
    ShaderPipeline(pipeline_type, workgroup_size),
    m_device(device),
    m_layout(layout),
    m_pipeline(pipeline)
{
    //
}

VulkanShaderPipeline::~VulkanShaderPipeline()
{
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_layout, nullptr);
}
