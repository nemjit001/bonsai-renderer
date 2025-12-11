#pragma once
#ifndef BONSAI_RENDERER_SPIRV_REFLECTOR_HPP
#define BONSAI_RENDERER_SPIRV_REFLECTOR_HPP

#include <cstdint>
#include <dxc/dxcapi.h>
#include <spirv_reflect.h>

/// @brief Lightweight SPIR-V reflector that allows reflecting shader IO data from SPIR-V bytecode blobs.
class SPIRVReflector
{
public:
    explicit SPIRVReflector(CComPtr<IDxcBlob> shader_source);
    ~SPIRVReflector();

    SPIRVReflector(SPIRVReflector const&) = delete;
    SPIRVReflector& operator=(SPIRVReflector const&) = delete;

    /// @brief Get the shader workgroup size. Only applicable to compute shaders.
    /// @param x Local size in the x dimension.
    /// @param y Local size in the y dimension.
    /// @param z Local size in the z dimension.
    void get_workgroup_size(uint32_t& x, uint32_t& y, uint32_t& z) const;

    [[nodiscard]]
    uint32_t get_push_constant_range_count() const;

    [[nodiscard]]
    uint32_t get_descriptor_set_count() const;

private:
    SpvReflectShaderModule m_reflect_module;
    SpvReflectEntryPoint m_entrypoint;
};

#endif //BONSAI_RENDERER_SPIRV_REFLECTOR_HPP