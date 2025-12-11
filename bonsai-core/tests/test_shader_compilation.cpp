#include <gtest/gtest.h>
#include "../src/render_backend/shader_compiler.hpp"

static constexpr char const* COMPUTE_SHADER = R"(
[[vk::binding(0, 0)]] cbuffer test_cbuffer : register(b0, space0) {
    uint foo;
};

[[vk::binding(1, 0)]] RWStructuredBuffer<uint> out_buffer : register(u0, space0);

[shader("compute")]
[numthreads(1, 1, 1)]
void AlternateEntrypoint()
{
    //
}

[shader("compute")]
[numthreads(1, 2, 3)]
void CSMain()
{
    out_buffer[0] = foo;
}
)";

TEST(shader_compilation_tests, compile_compute_shader)
{
    ShaderCompiler shader_compiler{};
    CComPtr<IDxcBlob> dxil_shader;
    EXPECT_TRUE(shader_compiler.compile("CSMain", BONSAI_TARGET_PROFILE_CS, { COMPUTE_SHADER, std::strlen(COMPUTE_SHADER), 0 }, false, &dxil_shader));
    EXPECT_TRUE(dxil_shader && dxil_shader->GetBufferSize() > 0 && dxil_shader->GetBufferPointer() != nullptr);

    CComPtr<IDxcBlob> spirv_shader{};
    EXPECT_TRUE(shader_compiler.compile("CSMain", BONSAI_TARGET_PROFILE_CS, { COMPUTE_SHADER, std::strlen(COMPUTE_SHADER), 0 }, true, &spirv_shader));
    EXPECT_TRUE(spirv_shader && spirv_shader->GetBufferSize() > 0 && spirv_shader->GetBufferPointer() != nullptr);
}

/**
 * These are Vulkan only unit tests for the SPIRV reflector. This is separate from the Vulkan backend, but required Vulkan
 * to be available before use.
 *
 * This tests the reflection capabilities of the backend only.
 */
#if BONSAI_USE_VULKAN
#include "../src/render_backend/vulkan/spirv_reflector.hpp"

TEST(shader_compilation_tests, reflect_compute_shader_local_size_spirv)
{
    ShaderCompiler shader_compiler{};
    CComPtr<IDxcBlob> spirv_shader{};
    EXPECT_TRUE(shader_compiler.compile("CSMain", BONSAI_TARGET_PROFILE_CS, { COMPUTE_SHADER, std::strlen(COMPUTE_SHADER), 0 }, true, &spirv_shader));

    SPIRVReflector reflector(spirv_shader);
    uint32_t x = 0, y = 0, z = 0;
    reflector.get_workgroup_size(x, y, z);
    EXPECT_EQ(x, 1);
    EXPECT_EQ(y, 2);
    EXPECT_EQ(z, 3);
}

TEST(shader_compilation_tests, reflect_compute_shader_pipeline_layout_spirv)
{
    ShaderCompiler shader_compiler{};
    CComPtr<IDxcBlob> spirv_shader{};
    EXPECT_TRUE(shader_compiler.compile("CSMain", BONSAI_TARGET_PROFILE_CS, { COMPUTE_SHADER, std::strlen(COMPUTE_SHADER), 0 }, true, &spirv_shader));

    SPIRVReflector reflector(spirv_shader);
    uint32_t const push_constant_count = reflector.get_push_constant_range_count();
    uint32_t const descriptor_set_count = reflector.get_descriptor_set_count();
    EXPECT_EQ(push_constant_count, 0);
    EXPECT_EQ(descriptor_set_count, 1);
}
#endif //BONSAI_USE_VULKAN
