#pragma once
#ifndef BONSAI_RENDERER_VULKAN_RENDER_BACKEND_HPP
#define BONSAI_RENDERER_VULKAN_RENDER_BACKEND_HPP

#include <vector>
#include <volk.h>
#include <vk_mem_alloc.h>
#include "bonsai/render_backend/render_backend.hpp"
#include "render_backend/vulkan/spirv_reflector.hpp"
#include "render_backend/vulkan/vulkan_render_commands.hpp"
#include "render_backend/shader_compiler.hpp"

static constexpr uint32_t BONSAI_VULKAN_VERSION = VK_API_VERSION_1_3;

/// @brief This struct contains all Vulkan device features that are queried during physical device selection.
/// By passing the features2 field to vkDeviceCreateInfo::pNext, the queried features will be enabled on the logical device.
/// See @ref VulkanRenderBackend::find_physical_device for queried features.
struct VulkanDeviceFeatures
{
    VkPhysicalDeviceFeatures2 features2;
    VkPhysicalDeviceVulkan11Features vulkan11_features;
    VkPhysicalDeviceVulkan12Features vulkan12_features;
    VkPhysicalDeviceVulkan13Features vulkan13_features;
};

/// @brief Vulkan queue family indices for a physical device.
struct VulkanQueueFamilies
{
    /// @brief Get the unique queue family indices for this queue setup.
    /// @return A vector of unique queue families.
    [[nodiscard]]
    std::vector<uint32_t> get_unique() const;

    uint32_t graphics_family; /// @brief The graphics queue family is also guaranteed to support presenting to surfaces.
};

struct VulkanPhysicalDeviceProperties
{
    VkPhysicalDeviceProperties2 properties2;
};

/// @brief Queried swap chain capabilities for a surface & physical device.
struct VulkanSwapchainCapabilities
{
    uint32_t min_image_count;
    uint32_t image_count;
    RenderFormat render_format;
    VkSurfaceFormatKHR preferred_format;
    std::vector<VkPresentModeKHR> present_modes;
};

/// @brief Reified swap chain configuration for a surface.
struct VulkanSwapchainConfiguration
{
    VkExtent2D image_extent;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swap_images = {};
    std::vector<VkImageView> swap_image_views = {};
    std::vector<VkSemaphore> swap_released_semaphores = {};
    std::vector<RenderTexture*> swap_render_textures = {};
};

/// @brief Vulkan implementation for the render backend.
class VulkanRenderBackend : public RenderBackend
{
public:
    /// @brief Create a new Vulkan render backend.
    /// @param platform_surface Main application surface, used for setting up initial state for device selection, swap chain, etc.
    /// @param imgui_context ImGui context created by engine.
    VulkanRenderBackend(PlatformSurface* platform_surface, ImGuiContext* imgui_context);
    ~VulkanRenderBackend() override;

    VulkanRenderBackend(VulkanRenderBackend const&) = delete;
    VulkanRenderBackend& operator=(VulkanRenderBackend const&) = delete;

    void wait_idle() const override;

    void reconfigure_swap_chain(uint32_t width, uint32_t height) override;

    RenderExtent2D get_swap_extent() const override;

    RenderFormat get_swap_format() const override;

    bool is_swap_srgb() const override;

    RenderBackendFrameResult new_frame() override;

    RenderBackendFrameResult end_frame() override;

    RenderCommands* get_frame_commands() override;

    RenderTexture* get_current_swap_texture() override;

    RenderBuffer* create_buffer(
        size_t size,
        RenderBufferUsageFlags buffer_usage,
        bool can_map
    ) override;

    RenderTexture* create_texture(
        RenderTextureType texture_type,
        RenderFormat format,
        uint32_t width,
        uint32_t height,
        uint32_t depth_or_layers,
        uint32_t mip_levels,
        SampleCount sample_count,
        RenderTextureUsageFlags texture_usage,
        RenderTextureTilingMode tiling_mode
    ) override;

    ShaderPipeline* create_graphics_pipeline(GraphicsPipelineDescriptor pipeline_descriptor) override;

    ShaderPipeline* create_compute_pipeline(ComputePipelineDescriptor pipeline_descriptor) override;

    uint64_t get_current_frame_index() const override { return m_frame_idx; }

private:
    /// @brief Check if device extensions are available on a physical device.
    /// @param device Device to check support for.
    /// @param extension_names Required extension names.
    /// @return A boolean indicating extension availability.
    static bool has_device_extensions(
        VkPhysicalDevice device,
        std::vector<char const*> const& extension_names
    );

    /// @brief Search for a physical device.
    /// @param instance Instance to use for search.
    /// @param device_properties Device properties output variable, populated on success.
    /// @param enabled_device_features Enabled device features based on internal feature query, populated on success.
    /// @param enabled_device_extensions Enabled device extensions that should be supported on the device.
    /// @return A suitable physical device, or VK_NULL_HANDLE on failure.
    static VkPhysicalDevice find_physical_device(
        VkInstance instance,
        VulkanPhysicalDeviceProperties& device_properties,
        VulkanDeviceFeatures& enabled_device_features,
        std::vector<char const*> const& enabled_device_extensions
    );

    /// @brief Find a queue family index by flags and surface support.
    /// @param physical_device Physical device to query for queue families.
    /// @param queue_families Queue families belonging to the passed physical device.
    /// @param required_flags Required queue flags.
    /// @param ignored_flags Ignored queue flags.
    /// @param surface Optional surface that should have present support for a queue with the given filter flags.
    /// @return A queue family index, or VK_QUEUE_FAMILY_IGNORED on failure.
    static uint32_t find_queue_family(
        VkPhysicalDevice physical_device,
        std::vector<VkQueueFamilyProperties> const& queue_families,
        VkQueueFlags required_flags,
        VkQueueFlags ignored_flags,
        VkSurfaceKHR surface = VK_NULL_HANDLE
    );

    /// @brief Query the swap chain capabilities for a surface & physical device.
    /// @param physical_device
    /// @param surface
    /// @return The swap chain capabilities.
    static VulkanSwapchainCapabilities get_swapchain_capabilities(
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface
    );

    /// @brief Configure the swap chain for a given surface & physical device.
    /// @param width Platform render surface width in pixels.
    /// @param height Platform render surface height in pixels.
    /// @param physical_device Vulkan physical device.
    /// @param surface Vulkan surface.
    /// @param device Vulkan logical device.
    /// @param swap_capabilities Swap chain capabilities for the surface & physical device.
    /// @param swapchain_config Output swap chain configuration.
    static bool configure_swapchain(
        uint32_t width,
        uint32_t height,
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface,
        VkDevice device,
        VulkanSwapchainCapabilities const& swap_capabilities,
        VulkanSwapchainConfiguration& swapchain_config
    );

    /// @brief Compile shader source code using the shader compiler.
    /// @param source Shader source structure.
    /// @param target_profile Shader target profile.
    /// @param compiled_shader Output compiled shader blob.
    /// @return A boolean indicating successful compilation.
    bool compile_shader_source(ShaderSource const& source, LPCWSTR target_profile, IDxcBlob** compiled_shader) const;

    /// @brief Generate a pipeline layout based on reflection data for shaders.
    /// @param reflector Reflection data for one or more shaders.
    /// @param descriptor_set_layouts Output descriptor set layouts associated with the pipeline layout.
    /// @return A generated pipeline layout.
    VkPipelineLayout generate_pipeline_layout(SPIRVReflector const& reflector, std::vector<VkDescriptorSetLayout>& descriptor_set_layouts);

private:
    PlatformSurface* m_main_surface = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
#endif //NDEBUG
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VulkanPhysicalDeviceProperties m_device_properties = {};
    VulkanQueueFamilies m_queue_families = {};
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VmaAllocator m_allocator = nullptr;

    VulkanSwapchainCapabilities m_swapchain_capabilities = {};
    VulkanSwapchainConfiguration m_swapchain_config = {};

    VkFence m_frame_ready = VK_NULL_HANDLE;
    VkSemaphore m_swap_available = VK_NULL_HANDLE;
    uint32_t m_active_swap_idx = 0;

    VkCommandPool m_graphics_cmd_pool = VK_NULL_HANDLE;
    VkCommandBuffer m_frame_cmd_buffer = VK_NULL_HANDLE;
    VulkanRenderCommands m_frame_commands = {};

    ShaderCompiler m_shader_compiler = {};
    uint64_t m_frame_idx = 0;
};


#endif //BONSAI_RENDERER_VULKAN_RENDER_BACKEND_HPP