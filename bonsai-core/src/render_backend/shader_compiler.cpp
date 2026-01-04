#include "shader_compiler.hpp"

#include <filesystem>
#include <string>
#include <memory>
#include "bonsai/core/fatal_exit.hpp"
#include "bonsai/core/logger.hpp"

ShaderCompiler::ShaderCompiler()
{
    if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)))
        || FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler))))
    {
        BONSAI_FATAL_EXIT("Failed to initialize DXC shader compiler\n");
    }

    if (FAILED(m_utils->CreateDefaultIncludeHandler(&m_include_handler)))
    {
        BONSAI_FATAL_EXIT("Failed to create DXC include handler\n");
    }
}

bool ShaderCompiler::compile_source(
    char const* name,
    char const* entrypoint,
    LPCWSTR target_profile,
    DxcBuffer source,
    char const* base_include_dir,
    bool compile_into_spirv,
    IDxcBlob** compiled_shader
) const
{
    std::wstring const shader_name(name, name + std::strlen(name) + 1);
    std::wstring const entrypoint_name(entrypoint, entrypoint + std::strlen(entrypoint) + 1);
    BONSAI_ENGINE_LOG_TRACE("Compiling shader blob ({}::{})", name, entrypoint);

    CComPtr<IDxcCompilerArgs> compiler_args{};
    HRESULT const arg_result = m_utils->BuildArguments(
        shader_name.c_str(),
        entrypoint_name.c_str(),
        target_profile,
        const_cast<LPCWSTR*>(DEFAULT_ARGUMENTS.data()), DEFAULT_ARGUMENTS.size(), // Const cast is necessary, pointer *should* not be modified through calling this function.
        nullptr, 0,
        &compiler_args
    );
    if (FAILED(arg_result))
    {
        BONSAI_ENGINE_LOG_ERROR("Failed to create shader compiler arguments");
        return false;
    }

    if (base_include_dir != nullptr)
    {
        std::wstring const include_dir(base_include_dir, base_include_dir + std::strlen(base_include_dir) + 1);
        LPCWSTR include_dir_args[2] = { L"I", include_dir.c_str(), };
        if (FAILED(compiler_args->AddArguments(include_dir_args, std::size(include_dir_args))))
        {
            BONSAI_ENGINE_LOG_ERROR("Failed to add shader base include dir argument");
            return false;
        }
    }

    if (compile_into_spirv)
    {
        HRESULT const spirv_args_result = compiler_args->AddArguments(const_cast<LPCWSTR*>(SPIRV_ARGUMENTS.data()), SPIRV_ARGUMENTS.size());
        if (FAILED(spirv_args_result))
        {
            BONSAI_ENGINE_LOG_ERROR("Failed to add shader SPIR-V compiler arguments");
            return false;
        }
    }

    CComPtr<IDxcResult> result{};
    if (FAILED(m_compiler->Compile(&source, compiler_args->GetArguments(), compiler_args->GetCount(), m_include_handler, IID_PPV_ARGS(&result)))
        || FAILED(result->GetResult(compiled_shader)))
    {
        BONSAI_ENGINE_LOG_ERROR("Failed to compile shader");
        return false;
    }

    CComPtr<IDxcBlob> errors{};
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    if (errors && errors->GetBufferSize() > 0)
    {
        BONSAI_ENGINE_LOG_WARN("Encountered shader compilation errors:\n{}", static_cast<char const*>(errors->GetBufferPointer()));
        return false;
    }

    return true;
}

bool ShaderCompiler::compile_file(char const* file_path, char const* entrypoint, LPCWSTR target_profile, bool compile_into_spirv, IDxcBlob** compiled_shader) const
{
    std::wstring const wide_file_path(file_path, file_path + std::strlen(file_path) + 1);
    CComPtr<IDxcBlobEncoding> shader_source{};
    if (FAILED(m_utils->LoadFile(wide_file_path.c_str(), nullptr, &shader_source)))
    {
        BONSAI_ENGINE_LOG_ERROR("Failed to load shader source from file: {}", file_path);
        return false;
    }

    std::filesystem::path base_include_dir(file_path);
    base_include_dir = base_include_dir.parent_path();

    DxcBuffer source_buffer{};
    source_buffer.Ptr = shader_source->GetBufferPointer();
    source_buffer.Size = shader_source->GetBufferSize();
    source_buffer.Encoding = 0;
    return compile_source(file_path, entrypoint, target_profile, source_buffer, base_include_dir.string().c_str(), compile_into_spirv, compiled_shader);
}
