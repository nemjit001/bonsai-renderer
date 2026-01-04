#pragma once
#ifndef BONSAI_RENDERER_SPIRV_REFLECTOR_HPP
#define BONSAI_RENDERER_SPIRV_REFLECTOR_HPP

#define VK_NO_PROTOTYPES
#include <cstdint>
#include <vector>
#include <spirv_reflect.h>
#include <vulkan/vulkan.h>
#include "render_backend/shader_compiler.hpp"

/// @brief A shader reflect module that consists of the SPIR-V module and the associated entrypoint.
struct ReflectModule
{
    SpvReflectShaderModule spv_module;
    SpvReflectEntryPoint spv_entrypoint;
};

/// @brief SPIR-V descriptor binding.
struct DescriptorBinding
{
    uint32_t binding;
    uint32_t set;
    VkDescriptorSetLayoutBinding layout_binding;
};

/// @brief Lightweight SPIR-V reflector that allows reflecting shader IO data from SPIR-V bytecode blobs.
class SPIRVReflector
{
public:
    /// @brief Create a SPIR-V Reflector for a single shader module (useful for compute shaders)
    explicit SPIRVReflector(IDxcBlob* shader_source);

    /// @brief Create a SPIR-V Reflector for multiple shaders, this will combine push constants and bindings into
    /// a unified shader interface.
    SPIRVReflector(IDxcBlob** shader_sources, uint32_t source_count);
    ~SPIRVReflector();

    SPIRVReflector(SPIRVReflector const&) = default;
    SPIRVReflector& operator=(SPIRVReflector const&) = default;

    /// @brief Get the shader workgroup size. Only applicable to compute shaders.
    /// @param x Local size in the x dimension.
    /// @param y Local size in the y dimension.
    /// @param z Local size in the z dimension.
    void get_workgroup_size(uint32_t& x, uint32_t& y, uint32_t& z) const;

    /// @brief Get the shader pipeline push constant range count.
    /// @return The number of unique push constant ranges for the shaders in the reflector.
    [[nodiscard]]
    uint32_t get_push_constant_range_count() const;

    /// @brief Get the shader pipeline push constants.
    /// @return The unique push constant ranges for the shaders in the reflector.
    [[nodiscard]]
    VkPushConstantRange const* get_push_constant_ranges() const;

    /// @brief Get the number of descriptor bindings for the shaders in the reflector.
    /// @return The number of deduplicated descriptor bindings for the shaders in the reflector.
    [[nodiscard]]
    uint32_t get_descriptor_binding_count() const;

    /// @brief Get the descriptor bindings for the shaders in the reflector.
    /// @return the deduplicated descriptor bindings for the shaders in the reflector.
    [[nodiscard]]
    DescriptorBinding const* get_descriptor_bindings() const;

private:
    /// @brief Parse shader source blobs into a list of reflect modules.
    /// @param shader_sources SPIR-V source blob array.
    /// @param source_count Number of shader sources in the source blob array.
    /// @return A list of reflect modules.
    static std::vector<ReflectModule> parse_reflect_modules(IDxcBlob** shader_sources, uint32_t source_count);

    /// @brief Parse push constant ranges from a list of shader reflect modules.
    /// @param reflect_modules The list of shader reflect modules to parse.
    /// @return A list of used push constant ranges.
    static std::vector<VkPushConstantRange> parse_push_constant_ranges(std::vector<ReflectModule> const& reflect_modules);

    /// @brief Parse descriptor bindings from a list of shader reflect modules.
    /// @param reflect_modules The list of shader reflect modules to parse.
    /// @return A list of used descriptor bindings.
    static std::vector<DescriptorBinding> parse_descriptor_bindings(std::vector<ReflectModule> const& reflect_modules);

private:
    std::vector<ReflectModule> m_reflect_modules;
    std::vector<VkPushConstantRange> m_push_constant_ranges;
    std::vector<DescriptorBinding> m_descriptor_bindings;
};

#endif //BONSAI_RENDERER_SPIRV_REFLECTOR_HPP