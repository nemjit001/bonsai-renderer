#include "spirv_reflector.hpp"

#include "bonsai/core/assert.hpp"

#define SPV_REFLECT_SUCCEEDED(result)   ((result) == SPV_REFLECT_RESULT_SUCCESS)
#define SPV_REFLECT_FAILED(result)      ((result) != SPV_REFLECT_RESULT_SUCCESS)

SPIRVReflector::SPIRVReflector(CComPtr<IDxcBlob> shader_source)
{
    spvReflectCreateShaderModule(shader_source->GetBufferSize(), shader_source->GetBufferPointer(), &m_reflect_module);
    BONSAI_ASSERT(m_reflect_module.entry_point_count == 1); // We expect to reflect for a single entrypoint only
    m_entrypoint = m_reflect_module.entry_points[0];
    m_push_constant_ranges = parse_push_constant_ranges(m_reflect_module);
    m_descriptor_bindings = parse_descriptor_bindings(m_reflect_module);
}

SPIRVReflector::~SPIRVReflector()
{
    spvReflectDestroyShaderModule(&m_reflect_module);
}

void SPIRVReflector::get_workgroup_size(uint32_t& x, uint32_t& y, uint32_t& z) const
{
    BONSAI_ASSERT(m_entrypoint.shader_stage == SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT && "Querying the workgroup size is only applicable to compute shaders!");
    x = m_entrypoint.local_size.x;
    y = m_entrypoint.local_size.y;
    z = m_entrypoint.local_size.z;
}

uint32_t SPIRVReflector::get_push_constant_range_count() const
{
    return static_cast<uint32_t>(m_push_constant_ranges.size());
}

VkPushConstantRange const* SPIRVReflector::get_push_constant_ranges() const
{
    return m_push_constant_ranges.data();
}

uint32_t SPIRVReflector::get_descriptor_binding_count() const
{
    return static_cast<uint32_t>(m_descriptor_bindings.size());
}

DescriptorBinding const* SPIRVReflector::get_descriptor_bindings() const
{
    return m_descriptor_bindings.data();
}

std::vector<VkPushConstantRange> SPIRVReflector::parse_push_constant_ranges(SpvReflectShaderModule const& reflect_module)
{
    VkShaderStageFlags const stage_flags = reflect_module.shader_stage;
    std::vector<VkPushConstantRange> pc_ranges{};
    pc_ranges.reserve(reflect_module.push_constant_block_count);
    for (uint32_t i = 0; i < reflect_module.push_constant_block_count; i++)
    {
        SpvReflectBlockVariable const& pc_block = reflect_module.push_constant_blocks[i];
        VkPushConstantRange pc_range{};
        pc_range.stageFlags = stage_flags;
        pc_range.offset = pc_block.absolute_offset;
        pc_range.size = pc_block.size;
        pc_ranges.push_back(pc_range);
    }

    return pc_ranges;
}

std::vector<DescriptorBinding> SPIRVReflector::parse_descriptor_bindings(SpvReflectShaderModule const& reflect_module)
{
    VkPipelineStageFlags const stage_flags = reflect_module.shader_stage;
    std::vector<DescriptorBinding> descriptor_bindings{};
    descriptor_bindings.reserve(reflect_module.descriptor_binding_count);
    for (uint32_t i = 0; i < reflect_module.descriptor_binding_count; i++)
    {
        SpvReflectDescriptorBinding const& spv_binding = reflect_module.descriptor_bindings[i];

        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = spv_binding.binding;
        layout_binding.descriptorType = static_cast<VkDescriptorType>(spv_binding.descriptor_type);
        layout_binding.descriptorCount = spv_binding.count;
        layout_binding.stageFlags = stage_flags;
        layout_binding.pImmutableSamplers = nullptr; // FIXME(nemjit001): Immutable samplers for pipeline *should* be generated...

        DescriptorBinding descriptor_binding{};
        descriptor_binding.binding = spv_binding.binding;
        descriptor_binding.set = spv_binding.set;
        descriptor_binding.name = spv_binding.name;
        descriptor_binding.layout_binding = layout_binding;
        descriptor_bindings.push_back(descriptor_binding);
    }

    return descriptor_bindings;
}
