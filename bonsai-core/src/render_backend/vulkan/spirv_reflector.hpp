#pragma once
#ifndef BONSAI_RENDERER_SPIRV_REFLECTOR_HPP
#define BONSAI_RENDERER_SPIRV_REFLECTOR_HPP

#define VK_NO_PROTOTYPES
#include <cstdint>
#include <vector>
#include <dxc/dxcapi.h>
#include <spirv_reflect.h>
#include <vulkan/vulkan.h>

struct DescriptorBinding
{
    uint32_t binding;
    uint32_t set;
    char const* name;
    VkDescriptorSetLayoutBinding layout_binding;
};

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
    VkPushConstantRange const* get_push_constant_ranges() const;

    [[nodiscard]]
    uint32_t get_descriptor_binding_count() const;

    [[nodiscard]]
    DescriptorBinding const* get_descriptor_bindings() const;

private:
    static std::vector<VkPushConstantRange> parse_push_constant_ranges(SpvReflectShaderModule const& reflect_module);

    static std::vector<DescriptorBinding> parse_descriptor_bindings(SpvReflectShaderModule const& reflect_module);

private:
    SpvReflectShaderModule m_reflect_module;
    SpvReflectEntryPoint m_entrypoint;
    std::vector<VkPushConstantRange> m_push_constant_ranges;
    std::vector<DescriptorBinding> m_descriptor_bindings;
};

#endif //BONSAI_RENDERER_SPIRV_REFLECTOR_HPP