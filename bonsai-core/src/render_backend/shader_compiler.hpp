#pragma once
#ifndef BONSAI_RENDERER_SHADER_COMPILER_HPP
#define BONSAI_RENDERER_SHADER_COMPILER_HPP

#include <array>
#include <dxc/dxcapi.h>

static constexpr LPCWSTR BONSAI_TARGET_PROFILE_VS   = L"vs_6_1";
static constexpr LPCWSTR BONSAI_TARGET_PROFILE_PS   = L"ps_6_1";
static constexpr LPCWSTR BONSAI_TARGET_PROFILE_CS   = L"cs_6_1";
static constexpr LPCWSTR BONSAI_TARGET_PROFILE_LIB  = L"lib_6_1";

/// @brief Shader compiler for render backends. Provides a common interface that is backend agnostic.
class ShaderCompiler
{
public:
    ShaderCompiler();
    ~ShaderCompiler() = default;

    ShaderCompiler(ShaderCompiler const& other) = default;
    ShaderCompiler& operator=(ShaderCompiler const& other) = default;

    /// @brief Compile a shader source buffer to the backend IL.
    /// @param name Shader source name for use in debug messages.
    /// @param entrypoint Shader entrypoint name.
    /// @param target_profile Target profile for the shader, specifies shader capabilities.
    /// @param source Shader HLSL source.
    /// @param base_include_dir Base include dir to use for #include directives, may be nullptr.
    /// @param compile_into_spirv Compile the shader code into SPIR-V bytecode.
    /// @param compiled_shader Output compiled shader byte code.
    /// @return A boolean indicating successful compilation.
    bool compile_source(char const* name, char const* entrypoint, LPCWSTR target_profile, DxcBuffer source, char const* base_include_dir, bool compile_into_spirv, IDxcBlob** compiled_shader) const;

    bool compile_file(char const* file_path, char const* entrypoint, LPCWSTR target_profile, bool compile_into_spirv, IDxcBlob** compiled_shader) const;

private:
    /// @brief Default shader arguments to pass to the shader compiler.
    static constexpr std::array DEFAULT_ARGUMENTS = {
        DXC_ARG_DEBUG,                  // Generate Debug info
        DXC_ARG_OPTIMIZATION_LEVEL3,    // Set the optimization level to the max
        L"-Wno-ignored-attributes",      // Disables ignored attributes (Vulkan specific attrs fall under this category...)
    };

    /// @brief SPIR-V shader arguments to pass to the shader compiler.
    static constexpr std::array SPIRV_ARGUMENTS = {
        L"-spirv",                      // Generate SPIR-V bytecode
        L"-fspv-reflect",               // Emit as much reflection data as possible
        L"-fspv-target-env=vulkan1.3"   // Use the Vulkan 1.3 target environment
    };

    CComPtr<IDxcUtils> m_utils = nullptr;
    CComPtr<IDxcCompiler3> m_compiler = nullptr;
    CComPtr<IDxcIncludeHandler> m_include_handler = nullptr;
};

#endif //BONSAI_RENDERER_SHADER_COMPILER_HPP