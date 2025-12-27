#include "spirv_reflector.hpp"

#include <string>
#include <unordered_map>
#include "bonsai/core/assert.hpp"

#define SPV_REFLECT_SUCCEEDED(result)   ((result) == SPV_REFLECT_RESULT_SUCCESS)
#define SPV_REFLECT_FAILED(result)      ((result) != SPV_REFLECT_RESULT_SUCCESS)

SPIRVReflector::SPIRVReflector(IDxcBlob* shader_source)
    :
    SPIRVReflector(&shader_source, 1)
{
    //
}

SPIRVReflector::SPIRVReflector(IDxcBlob** shader_sources, uint32_t source_count)
{
    m_reflect_modules = parse_reflect_modules(shader_sources, source_count);
    m_push_constant_ranges = parse_push_constant_ranges(m_reflect_modules);
    m_descriptor_bindings = parse_descriptor_bindings(m_reflect_modules);
}

SPIRVReflector::~SPIRVReflector()
{
    for (auto& module : m_reflect_modules)
    {
        spvReflectDestroyShaderModule(&module.spv_module);
    }
}

void SPIRVReflector::get_workgroup_size(uint32_t& x, uint32_t& y, uint32_t& z) const
{
    BONSAI_ASSERT(m_reflect_modules.size() == 1 && "Querying the workgroup size is only applicable when reflecting a single shader!");
    ReflectModule const& module = m_reflect_modules[0];
    BONSAI_ASSERT(module.spv_entrypoint.shader_stage == SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT && "Querying the workgroup size is only applicable to compute shaders!");
    x = module.spv_entrypoint.local_size.x;
    y = module.spv_entrypoint.local_size.y;
    z = module.spv_entrypoint.local_size.z;
}

uint32_t SPIRVReflector::get_vertex_attribute_count() const
{
    return 0;
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

std::vector<ReflectModule> SPIRVReflector::parse_reflect_modules(IDxcBlob** shader_sources, uint32_t source_count)
{
    std::vector<ReflectModule> reflect_modules{};
    reflect_modules.reserve(source_count);
    for (uint32_t i = 0; i < source_count; i++)
    {
        IDxcBlob* shader_source = shader_sources[i];

        SpvReflectShaderModule spv_module{};
        spvReflectCreateShaderModule(shader_source->GetBufferSize(), shader_source->GetBufferPointer(), &spv_module);
        BONSAI_ASSERT(spv_module.entry_point_count == 1); // We expect to reflect for a single entrypoint only

        ReflectModule reflect_module{};
        reflect_module.spv_module = spv_module;
        reflect_module.spv_entrypoint = reflect_module.spv_module.entry_points[0];
        reflect_modules.push_back(reflect_module);
    }

    return reflect_modules;
}

std::vector<VkPushConstantRange> SPIRVReflector::parse_push_constant_ranges(std::vector<ReflectModule> const& reflect_modules)
{
    uint32_t absolute_module_offset = 0;
    std::unordered_map<std::string, VkPushConstantRange> named_pc_ranges{};
    for (auto const& module : reflect_modules)
    {
        VkShaderStageFlags const stage_flags = module.spv_module.shader_stage;
        for (uint32_t i = 0; i < module.spv_module.push_constant_block_count; i++)
        {
            SpvReflectBlockVariable const& pc_block = module.spv_module.push_constant_blocks[i];
            auto const iter = named_pc_ranges.find(pc_block.name);
            if (iter != named_pc_ranges.end())
            {
                VkPushConstantRange& pc_range = iter->second;
                pc_range.stageFlags |= stage_flags;
                BONSAI_ASSERT(pc_range.size == pc_block.size && "Named push constant block sizes do not match!");
            }
            else
            {
                absolute_module_offset += pc_block.offset; // Update global offset of pc range

                VkPushConstantRange pc_range{};
                pc_range.stageFlags = stage_flags;
                pc_range.offset = absolute_module_offset;
                pc_range.size = pc_block.size;

                named_pc_ranges[pc_block.name] = pc_range;
            }
        }
    }

    std::vector<VkPushConstantRange> pc_ranges{};
    pc_ranges.reserve(named_pc_ranges.size());
    for (auto const& [name, range] : named_pc_ranges)
    {
        pc_ranges.push_back(range);
    }

    return pc_ranges;
}

std::vector<DescriptorBinding> SPIRVReflector::parse_descriptor_bindings(std::vector<ReflectModule> const& reflect_modules)
{
    std::unordered_map<std::string, DescriptorBinding> named_descriptor_bindings{};
    for (auto const& module : reflect_modules)
    {
        VkPipelineStageFlags const stage_flags = module.spv_module.shader_stage;
        for (uint32_t i = 0; i < module.spv_module.descriptor_binding_count; i++)
        {
            SpvReflectDescriptorBinding const& spv_binding = module.spv_module.descriptor_bindings[i];
            auto const iter = named_descriptor_bindings.find(spv_binding.name);

            VkDescriptorSetLayoutBinding layout_binding{};
            layout_binding.binding = spv_binding.binding;
            layout_binding.descriptorType = static_cast<VkDescriptorType>(spv_binding.descriptor_type);
            layout_binding.descriptorCount = spv_binding.count;
            layout_binding.stageFlags = stage_flags;
            layout_binding.pImmutableSamplers = nullptr; // FIXME(nemjit001): Immutable samplers for pipeline *should* be generated...

            if (iter != named_descriptor_bindings.end())
            {
                DescriptorBinding& descriptor_binding = iter->second;
                descriptor_binding.layout_binding.stageFlags |= stage_flags;
                BONSAI_ASSERT(descriptor_binding.set == spv_binding.set && "Named descriptor set index does not match!");
                BONSAI_ASSERT(descriptor_binding.binding == spv_binding.binding && "Named descriptor binding index does not match!");
                BONSAI_ASSERT(descriptor_binding.layout_binding.descriptorType == layout_binding.descriptorType && "Named descriptor type does not match!");
                BONSAI_ASSERT(descriptor_binding.layout_binding.descriptorCount == layout_binding.descriptorCount && "Named descriptor count does not match!");
            }
            else
            {
                DescriptorBinding descriptor_binding{};
                descriptor_binding.binding = spv_binding.binding;
                descriptor_binding.set = spv_binding.set;
                descriptor_binding.layout_binding = layout_binding;

                named_descriptor_bindings[spv_binding.name] = descriptor_binding;
            }
        }
    }

    std::vector<DescriptorBinding> descriptor_bindings{};
    descriptor_bindings.reserve(named_descriptor_bindings.size());
    for (auto const& [name, binding] : named_descriptor_bindings)
    {
        descriptor_bindings.push_back(binding);
    }

    return descriptor_bindings;
}
