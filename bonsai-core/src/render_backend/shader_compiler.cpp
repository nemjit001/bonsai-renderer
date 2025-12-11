#include "shader_compiler.hpp"

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

bool ShaderCompiler::compile(char const* entrypoint, LPCWSTR target_profile, DxcBuffer source, bool compile_into_spirv, IDxcBlob** compiled_shader) const
{
    std::wstring const entrypoint_name(entrypoint, entrypoint + std::strlen(entrypoint) + 1);
    CComPtr<IDxcCompilerArgs> compiler_args{};
    HRESULT const arg_result = m_utils->BuildArguments(
        L"shader",
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

    if (compile_into_spirv
        && FAILED(compiler_args->AddArguments(const_cast<LPCWSTR*>(SPIRV_ARGUMENTS.data()), SPIRV_ARGUMENTS.size())))
    {
        BONSAI_ENGINE_LOG_ERROR("Failed to add shader SPIR-V compiler arguments");
        return false;
    }

    CComPtr<IDxcResult> result = nullptr;
    if (FAILED(m_compiler->Compile(&source, compiler_args->GetArguments(), compiler_args->GetCount(), m_include_handler, IID_PPV_ARGS(&result)))
        || FAILED(result->GetResult(compiled_shader)))
    {
        BONSAI_ENGINE_LOG_ERROR("Failed to compile shader");
        return false;
    }

    CComPtr<IDxcBlob> errors = nullptr;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    if (errors && errors->GetBufferSize() > 0)
    {
        BONSAI_ENGINE_LOG_WARN("Encountered shader compilation errors:\n{}", static_cast<char const*>(errors->GetBufferPointer()));
        return false;
    }

    BONSAI_ENGINE_LOG_TRACE("Compiled shader blob");
    return true;
}
