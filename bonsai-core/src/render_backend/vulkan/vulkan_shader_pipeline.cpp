#include "vulkan_shader_pipeline.hpp"

VulkanShaderPipeline::VulkanShaderPipeline(VkDevice device, VkPipelineLayout layout, VkPipeline pipeline)
    :
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
