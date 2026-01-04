#include <gtest/gtest.h>
#include "../src/render_backend/shader_compiler.hpp"

static constexpr char const* COMPUTE_SHADER = R"(
struct Constants {
    uint write_idx;
};

struct TestBuffer {
    uint foo;
};

[[vk::push_constant]] ConstantBuffer<Constants> constants : register(b0, space0);
[[vk::binding(0, 0)]] ConstantBuffer<TestBuffer> test_buffer : register(b1, space0);
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
    out_buffer[constants.write_idx] = test_buffer.foo;
}
)";

static constexpr char const* VERTEX_SHADER = R"(
struct VSInput
{
    [[vk::location(0)]] float3 position     : POSITION0;
    [[vk::location(1)]] float3 normal       : NORMAL0;
    [[vk::location(2)]] float2 tex_coord    : TEXCOORD0;
};

struct PSInput
{
    float4 position     : SV_Position;
    float3 normal       : NORMAL0;
    float2 tex_coord    : TEXCOORD0;
};

[shader("vertex")]
PSInput VSMain(VSInput input)
{
    PSInput result;
    result.position = float4(input.position, 1.0);
    result.normal = input.normal;
    result.tex_coord = input.tex_coord;
    return result;
}
)";

TEST(shader_compilation_tests, compile_compute_shader)
{
    ShaderCompiler const shader_compiler{};
    DxcBuffer const shader_source{ COMPUTE_SHADER, std::strlen(COMPUTE_SHADER), 0 };
    CComPtr<IDxcBlob> dxil_shader{};
    EXPECT_TRUE(shader_compiler.compile_source("dxil_shader", "CSMain", BONSAI_TARGET_PROFILE_CS, shader_source, nullptr, false, &dxil_shader));
    EXPECT_TRUE(dxil_shader && dxil_shader->GetBufferSize() > 0 && dxil_shader->GetBufferPointer() != nullptr);

    CComPtr<IDxcBlob> spirv_shader{};
    EXPECT_TRUE(shader_compiler.compile_source("spirv_shader", "CSMain", BONSAI_TARGET_PROFILE_CS, shader_source, nullptr, true, &spirv_shader));
    EXPECT_TRUE(spirv_shader && spirv_shader->GetBufferSize() > 0 && spirv_shader->GetBufferPointer() != nullptr);
}

/*
 * These are Vulkan only unit tests for the SPIRV reflector. This is separate from the Vulkan backend, but requires Vulkan
 * to be available before use.
 *
 * This tests the reflection capabilities of the backend only.
 */
#if BONSAI_USE_VULKAN
#include "../src/render_backend/vulkan/spirv_reflector.hpp"

TEST(shader_compilation_tests, reflect_compute_shader_local_size_spirv)
{
    ShaderCompiler shader_compiler{};
    DxcBuffer const shader_source{ COMPUTE_SHADER, std::strlen(COMPUTE_SHADER), 0 };
    CComPtr<IDxcBlob> spirv_shader{};
    EXPECT_TRUE(shader_compiler.compile_source("reflect_shader", "CSMain", BONSAI_TARGET_PROFILE_CS, shader_source, nullptr, true, &spirv_shader));

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
    EXPECT_TRUE(shader_compiler.compile_source("reflect_shader", "CSMain", BONSAI_TARGET_PROFILE_CS, { COMPUTE_SHADER, std::strlen(COMPUTE_SHADER), 0 }, nullptr, true, &spirv_shader));

    SPIRVReflector reflector(spirv_shader);
    uint32_t const push_constant_count = reflector.get_push_constant_range_count();
    uint32_t const descriptor_binding_count = reflector.get_descriptor_binding_count();
    EXPECT_EQ(push_constant_count, 1);
    EXPECT_EQ(descriptor_binding_count, 2);

    VkPushConstantRange const* pc_ranges = reflector.get_push_constant_ranges();
    EXPECT_NE(pc_ranges, nullptr);
    EXPECT_EQ(pc_ranges[0].stageFlags, VK_SHADER_STAGE_COMPUTE_BIT);
    EXPECT_EQ(pc_ranges[0].offset, 0);
    EXPECT_EQ(pc_ranges[0].size, 4);

    DescriptorBinding const* descriptor_bindings = reflector.get_descriptor_bindings();
    for (uint32_t i = 0; i < descriptor_binding_count; i++)
    {
        DescriptorBinding const& descriptor_binding = descriptor_bindings[i];
        if (descriptor_binding.binding == 0 && descriptor_binding.set == 0)
        {
            EXPECT_EQ(descriptor_binding.layout_binding.binding, descriptor_binding.binding);
            EXPECT_EQ(descriptor_binding.layout_binding.stageFlags, VK_SHADER_STAGE_COMPUTE_BIT);
            EXPECT_EQ(descriptor_binding.layout_binding.descriptorType, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            EXPECT_EQ(descriptor_binding.layout_binding.descriptorCount, 1);
        }
        else if (descriptor_binding.binding == 1 && descriptor_binding.set == 0)
        {
            EXPECT_EQ(descriptor_binding.layout_binding.binding, descriptor_binding.binding);
            EXPECT_EQ(descriptor_binding.layout_binding.stageFlags, VK_SHADER_STAGE_COMPUTE_BIT);
            EXPECT_EQ(descriptor_binding.layout_binding.descriptorType, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            EXPECT_EQ(descriptor_binding.layout_binding.descriptorCount, 1);
        }
    }
}

#endif //BONSAI_USE_VULKAN
