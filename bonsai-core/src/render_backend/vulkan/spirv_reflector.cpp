#include "spirv_reflector.hpp"

#include "bonsai/core/assert.hpp"

#define SPV_REFLECT_SUCCEEDED(result)   ((result) == SPV_REFLECT_RESULT_SUCCESS)
#define SPV_REFLECT_FAILED(result)      ((result) != SPV_REFLECT_RESULT_SUCCESS)

SPIRVReflector::SPIRVReflector(CComPtr<IDxcBlob> shader_source)
{
    spvReflectCreateShaderModule(shader_source->GetBufferSize(), shader_source->GetBufferPointer(), &m_reflect_module);
    BONSAI_ASSERT(m_reflect_module.entry_point_count == 1); // We expect to reflect for a single entrypoint only
    m_entrypoint = m_reflect_module.entry_points[0];
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
    return m_entrypoint.used_push_constant_count;
}

uint32_t SPIRVReflector::get_descriptor_set_count() const
{
    return m_entrypoint.descriptor_set_count;
}
