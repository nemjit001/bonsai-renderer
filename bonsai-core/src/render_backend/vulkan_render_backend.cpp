#include "vulkan_render_backend.hpp"
#define VMA_IMPLEMENTATION
#define VOLK_IMPLEMENTATION

#include <algorithm>
#include <backends/imgui_impl_vulkan.h>
#include <vk_mem_alloc.h>
#include <volk.h>
#include "bonsai/core/assert.hpp"
#include "bonsai/core/fatal_exit.hpp"
#include "bonsai/core/logger.hpp"
#include "render_backend/vulkan/enum_conversion.hpp"
#include "render_backend/vulkan/vk_check.hpp"
#include "render_backend/vulkan/vulkan_buffer.hpp"
#include "render_backend/vulkan/vulkan_shader_pipeline.hpp"
#include "render_backend/vulkan/vulkan_texture.hpp"
#include "bonsai_config.hpp"

[[maybe_unused]]
static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
    void* user_data
)
{
    (void)(message_type);
    (void)(user_data);

    switch (message_severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        BONSAI_ENGINE_LOG_TRACE("[Vulkan] {}", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        BONSAI_ENGINE_LOG_INFO("[Vulkan] {}", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        BONSAI_ENGINE_LOG_WARN("[Vulkan] {}", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        BONSAI_ENGINE_LOG_ERROR("[Vulkan] {}", callback_data->pMessage);
        break;
    default:
        break;
    }

    return VK_FALSE;
}

[[maybe_unused]]
static VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info()
{
    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    create_info.pfnUserCallback = vulkan_debug_callback;
    create_info.pUserData = nullptr;

    return create_info;
}

std::vector<uint32_t> VulkanQueueFamilies::get_unique() const
{
    std::vector<uint32_t> queue_families{ graphics_family, };
    std::sort(queue_families.begin(), queue_families.end());
    queue_families.erase(std::unique(queue_families.begin(), queue_families.end()), queue_families.end());
    return queue_families;
}

VulkanRenderBackend::VulkanRenderBackend(PlatformSurface* platform_surface, ImGuiContext* imgui_context)
{
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(imgui_context);

    m_main_surface = platform_surface;
    if (VK_FAILED(volkInitialize()))
    {
        BONSAI_FATAL_EXIT("Failed to load Vulkan symbols\n");
    }
    BONSAI_ENGINE_LOG_TRACE("Loaded Vulkan symbols: v{}.{}.{}",
        VK_VERSION_MAJOR(volkGetInstanceVersion()),
        VK_VERSION_MINOR(volkGetInstanceVersion()),
        VK_VERSION_PATCH(volkGetInstanceVersion())
    );

    uint32_t platform_extension_count = 0;
    char const** platform_extensions = Platform::get_vulkan_instance_extensions(&platform_extension_count);

    std::vector<char const*> enabled_layers{};
    std::vector<char const*> enabled_extensions(platform_extensions, platform_extensions + platform_extension_count);
#ifndef NDEBUG
    enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif //NDEBUG

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = "Bonsai Renderer";
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, BONSAI_VERSION_MAJOR, BONSAI_VERSION_MINOR, BONSAI_VERSION_PATCH);
    app_info.pEngineName = "Bonsai Renderer";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, BONSAI_VERSION_MAJOR, BONSAI_VERSION_MINOR, BONSAI_VERSION_PATCH);
    app_info.apiVersion = BONSAI_VULKAN_VERSION;

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = nullptr;
    instance_create_info.flags = 0;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
    instance_create_info.ppEnabledLayerNames = enabled_layers.data();
    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
    instance_create_info.ppEnabledExtensionNames = enabled_extensions.data();

#ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = get_debug_messenger_create_info();
    instance_create_info.pNext = &debug_messenger_create_info;
#endif //NDEBUG

    if (VK_FAILED(vkCreateInstance(&instance_create_info, nullptr, &m_instance)))
    {
        BONSAI_FATAL_EXIT("Failed to create Vulkan instance\n");
    }
    volkLoadInstance(m_instance);

#ifndef NDEBUG
    if (VK_FAILED(vkCreateDebugUtilsMessengerEXT(m_instance, &debug_messenger_create_info, nullptr, &m_debugMessenger)))
    {
        BONSAI_FATAL_EXIT("Failed to create Vulkan debug messenger\n");
    }
#endif //NDEBUG

    if (!m_main_surface->create_vulkan_surface(m_instance, nullptr, &m_surface))
    {
        BONSAI_FATAL_EXIT("Failed to create Vulkan surface\n");
    }

    VkPhysicalDeviceProperties device_properties{};
    VulkanDeviceFeatures enabled_features{};
    std::vector<char const*> enabled_device_extensions{};
    enabled_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    m_physical_device = find_physical_device(m_instance, device_properties, enabled_features, enabled_device_extensions);
    if (m_physical_device == VK_NULL_HANDLE)
    {
        BONSAI_FATAL_EXIT("Failed to find suitable physical device\n");
    }
    BONSAI_ENGINE_LOG_TRACE("Selected Vulkan physical device: \"{} ({})\"",
        device_properties.deviceName,
        device_properties.deviceID
    );

    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_count, queue_families.data());
    m_queue_families.graphics_family = find_queue_family(m_physical_device, queue_families,VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, 0, m_surface);
    if (m_queue_families.graphics_family == VK_QUEUE_FAMILY_IGNORED)
    {
        BONSAI_FATAL_EXIT("Failed to select required device queue families\n");
    }

    std::vector<uint32_t> const unique_queue_families = m_queue_families.get_unique();
    std::vector<float> const queue_priorities(unique_queue_families.size(), 1.0F);
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.reserve(unique_queue_families.size());
    for (size_t i = 0; i < unique_queue_families.size(); i++)
    {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.pNext = nullptr;
        queue_create_info.flags = 0;
        queue_create_info.queueFamilyIndex = unique_queue_families[i];
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priorities[i];

        queue_create_infos.push_back(queue_create_info);
    }

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &enabled_features.features2;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_device_extensions.size());
    device_create_info.ppEnabledExtensionNames = enabled_device_extensions.data();
    device_create_info.pEnabledFeatures = nullptr;

    if (VK_FAILED(vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device)))
    {
        BONSAI_FATAL_EXIT("Failed to create Vulkan device\n");
    }
    vkGetDeviceQueue(m_device, m_queue_families.graphics_family, 0, &m_graphics_queue);

    VmaVulkanFunctions vma_vulkan_functions{};
    VmaAllocatorCreateInfo allocator_create_info{};
    allocator_create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT
        | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocator_create_info.instance = m_instance;
    allocator_create_info.physicalDevice = m_physical_device;
    allocator_create_info.device = m_device;
    allocator_create_info.vulkanApiVersion = BONSAI_VULKAN_VERSION;
    allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;

    if (VK_FAILED(vmaImportVulkanFunctionsFromVolk(&allocator_create_info, &vma_vulkan_functions))
        || VK_FAILED(vmaCreateAllocator(&allocator_create_info, &m_allocator)))
    {
        BONSAI_FATAL_EXIT("Failed to create Vulkan VMA allocator\n");
    }

    uint32_t surface_width = 0, surface_height = 0;
    m_main_surface->get_size_in_pixels(surface_width, surface_height);
    m_swapchain_capabilities = get_swapchain_capabilities(m_physical_device, m_surface);
    if (!configure_swapchain(surface_width, surface_height, m_physical_device, m_surface, m_device, m_swapchain_capabilities, m_swapchain_config))
    {
        BONSAI_FATAL_EXIT("Failed to configure Vulkan swap chain\n");
    }

    VkFenceCreateInfo frame_ready_fence_create_info{};
    frame_ready_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    frame_ready_fence_create_info.pNext = nullptr;
    frame_ready_fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = nullptr;
    semaphore_create_info.flags = 0;

    if (VK_FAILED(vkCreateFence(m_device, &frame_ready_fence_create_info, nullptr, &m_frame_ready))
        || VK_FAILED(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_swap_available)))
    {
        BONSAI_FATAL_EXIT("Failed to create Vulkan frame sync state\n");
    }

    VkCommandPoolCreateInfo graphics_cmd_pool_create_info{};
    graphics_cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    graphics_cmd_pool_create_info.pNext = nullptr;
    graphics_cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    graphics_cmd_pool_create_info.queueFamilyIndex = m_queue_families.graphics_family;

    if (VK_FAILED(vkCreateCommandPool(m_device, &graphics_cmd_pool_create_info, nullptr, &m_graphics_cmd_pool)))
    {
        BONSAI_FATAL_EXIT("Failed to create Vulkan frame command pool(s)\n");
    }

    VkCommandBufferAllocateInfo frame_cmd_buffer_allocate_info{};
    frame_cmd_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    frame_cmd_buffer_allocate_info.pNext = nullptr;
    frame_cmd_buffer_allocate_info.commandPool = m_graphics_cmd_pool;
    frame_cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    frame_cmd_buffer_allocate_info.commandBufferCount = 1;

    if (VK_FAILED(vkAllocateCommandBuffers(m_device, &frame_cmd_buffer_allocate_info, &m_frame_cmd_buffer)))
    {
        BONSAI_FATAL_EXIT("Failed to allocate Vulkan frame command buffer(s)\n");
    }
    m_frame_commands = VulkanRenderCommands(m_frame_cmd_buffer);

    VkPipelineRenderingCreateInfo imgui_pipeline_rendering_info{};
    imgui_pipeline_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    imgui_pipeline_rendering_info.pNext = nullptr;
    imgui_pipeline_rendering_info.viewMask = 0;
    imgui_pipeline_rendering_info.colorAttachmentCount = 1;
    imgui_pipeline_rendering_info.pColorAttachmentFormats = &m_swapchain_capabilities.preferred_format.format;
    imgui_pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    imgui_pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    ImGui_ImplVulkan_InitInfo imgui_init_info{};
    imgui_init_info.ApiVersion = BONSAI_VULKAN_VERSION;
    imgui_init_info.Instance = m_instance;
    imgui_init_info.PhysicalDevice = m_physical_device;
    imgui_init_info.Device = m_device;
    imgui_init_info.QueueFamily = m_queue_families.graphics_family;
    imgui_init_info.Queue = m_graphics_queue;
    imgui_init_info.DescriptorPool = VK_NULL_HANDLE; // Uses internal descriptor pool for ImGui
    imgui_init_info.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
    imgui_init_info.MinImageCount = m_swapchain_capabilities.min_image_count;
    imgui_init_info.ImageCount = m_swapchain_capabilities.image_count;
    imgui_init_info.PipelineCache = VK_NULL_HANDLE;
    imgui_init_info.UseDynamicRendering = true;
    imgui_init_info.PipelineInfoMain.Subpass = 0;
    imgui_init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    imgui_init_info.PipelineInfoMain.PipelineRenderingCreateInfo = imgui_pipeline_rendering_info;

    if (!ImGui_ImplVulkan_Init(&imgui_init_info))
    {
        BONSAI_FATAL_EXIT("Failed to initialize Vulkan ImGui backend\n");
    }

    ImGuiIO& imgui_io = ImGui::GetIO();
    if (m_swapchain_capabilities.preferred_format.format == VK_FORMAT_R8G8B8A8_SRGB
        || m_swapchain_capabilities.preferred_format.format == VK_FORMAT_B8G8R8A8_SRGB)
    {
        imgui_io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
    }
}

VulkanRenderBackend::~VulkanRenderBackend()
{
    VulkanRenderBackend::wait_idle();
    ImGui_ImplVulkan_Shutdown();

    vkDestroyCommandPool(m_device, m_graphics_cmd_pool, nullptr);

    vkDestroySemaphore(m_device, m_swap_available, nullptr);
    vkDestroyFence(m_device, m_frame_ready, nullptr);

    for (size_t i = 0; i < m_swapchain_config.swap_images.size(); i++)
    {
        delete m_swapchain_config.swap_render_textures[i];
        vkDestroySemaphore(m_device, m_swapchain_config.swap_released_semaphores[i], nullptr);
        vkDestroyImageView(m_device, m_swapchain_config.swap_image_views[i], nullptr);
    }
    vkDestroySwapchainKHR(m_device, m_swapchain_config.swapchain, nullptr);

    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
#ifndef NDEBUG
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
#endif //NDEBUG
    vkDestroyInstance(m_instance, nullptr);
    volkFinalize();
}

void VulkanRenderBackend::wait_idle() const
{
    vkDeviceWaitIdle(m_device);
}

void VulkanRenderBackend::reconfigure_swap_chain(uint32_t width, uint32_t height)
{
    configure_swapchain(
        width, height,
        m_physical_device,
        m_surface,
        m_device,
        m_swapchain_capabilities,
        m_swapchain_config
    );
}

RenderExtent2D VulkanRenderBackend::get_swap_extent() const
{
    return {
        m_swapchain_config.image_extent.width,
        m_swapchain_config.image_extent.height,
    };
}

RenderFormat VulkanRenderBackend::get_swap_format() const
{
    return m_swapchain_capabilities.render_format;
}

RenderBackendFrameResult VulkanRenderBackend::new_frame()
{
    vkWaitForFences(m_device, 1, &m_frame_ready, VK_TRUE, UINT64_MAX);
    VkResult const acquire_result = vkAcquireNextImageKHR(m_device, m_swapchain_config.swapchain, UINT64_MAX, m_swap_available, VK_NULL_HANDLE, &m_active_swap_idx);
    if (VK_FAILED(acquire_result)
        && (acquire_result == VK_SUBOPTIMAL_KHR || acquire_result == VK_ERROR_OUT_OF_DATE_KHR))
    {
        return RenderBackendFrameResult::SwapOutOfDate;
    }
    else if (VK_FAILED(acquire_result))
    {
        return RenderBackendFrameResult::FatalError;
    }

    vkResetFences(m_device, 1, &m_frame_ready); // We're committed now to finishing this frame
    ImGui_ImplVulkan_NewFrame();
    return RenderBackendFrameResult::Ok;
}

RenderBackendFrameResult VulkanRenderBackend::end_frame()
{
    VkPipelineStageFlags const wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo frame_submit_info = {};
    frame_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    frame_submit_info.waitSemaphoreCount = 1;
    frame_submit_info.pWaitSemaphores = &m_swap_available;
    frame_submit_info.pWaitDstStageMask = wait_stages;
    frame_submit_info.commandBufferCount = 1;
    frame_submit_info.pCommandBuffers = &m_frame_cmd_buffer;
    frame_submit_info.signalSemaphoreCount = 1;
    frame_submit_info.pSignalSemaphores = &m_swapchain_config.swap_released_semaphores[m_active_swap_idx];

    if (VK_FAILED(vkQueueSubmit(m_graphics_queue, 1, &frame_submit_info, m_frame_ready)))
    {
        return RenderBackendFrameResult::FatalError;
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &m_swapchain_config.swap_released_semaphores[m_active_swap_idx];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swapchain_config.swapchain;
    present_info.pImageIndices = &m_active_swap_idx;
    present_info.pResults = nullptr;

    VkResult const present_result = vkQueuePresentKHR(m_graphics_queue, &present_info);
    if (VK_FAILED(present_result)
        && (present_result == VK_SUBOPTIMAL_KHR || present_result == VK_ERROR_OUT_OF_DATE_KHR))
    {
        m_frame_idx += 1;
        return RenderBackendFrameResult::SwapOutOfDate;
    }
    else if (VK_FAILED(present_result))
    {
        return RenderBackendFrameResult::FatalError;
    }

    m_frame_idx += 1;
    return RenderBackendFrameResult::Ok;
}

RenderCommands* VulkanRenderBackend::get_frame_commands()
{
    return &m_frame_commands;
}

RenderTexture* VulkanRenderBackend::get_current_swap_texture()
{
    return m_swapchain_config.swap_render_textures[m_active_swap_idx];
}

RenderBuffer* VulkanRenderBackend::create_buffer(
    size_t size,
    RenderBufferUsageFlags buffer_usage,
    bool can_map
)
{
    // Set buffer usage flags
    VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    if (buffer_usage & RenderBufferUsageTransferSrc)
        usage_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (buffer_usage & RenderBufferUsageTransferDst)
        usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (buffer_usage & RenderBufferUsageUniformBuffer)
        usage_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (buffer_usage & RenderBufferUsageStorageBuffer)
        usage_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (buffer_usage & RenderBufferUsageIndexBuffer)
        usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (buffer_usage & RenderBufferUsageVertexBuffer)
        usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (buffer_usage & RenderBufferUsageIndirectBuffer)
        usage_flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    // Set memory property flags
    VkMemoryPropertyFlags memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VmaAllocationCreateFlags allocation_create_flags = 0;
    if (can_map)
    {
        memory_property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        allocation_create_flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    }

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = nullptr;
    buffer_create_info.flags = 0;
    buffer_create_info.usage = usage_flags;
    buffer_create_info.size = static_cast<uint32_t>(size);
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create_info.queueFamilyIndexCount = 0;
    buffer_create_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo allocation_create_info{};
    allocation_create_info.flags = allocation_create_flags;
    allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    allocation_create_info.requiredFlags = memory_property_flags;
    allocation_create_info.preferredFlags = 0;
    allocation_create_info.memoryTypeBits = UINT32_MAX;
    allocation_create_info.pool = VK_NULL_HANDLE;
    allocation_create_info.pUserData = nullptr;
    allocation_create_info.priority = 1.0F;

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    if (VK_FAILED(vmaCreateBuffer(m_allocator, &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr)))
    {
        return nullptr;
    }

    VulkanBufferDesc buffer_desc{};
    buffer_desc.size = size;

    return new VulkanBuffer(m_allocator, buffer, allocation, buffer_desc);
}

RenderTexture* VulkanRenderBackend::create_texture(
    RenderTextureType texture_type,
    RenderFormat format,
    uint32_t width,
    uint32_t height,
    uint32_t depth_or_layers,
    uint32_t mip_levels,
    SampleCount sample_count,
    RenderTextureUsageFlags texture_usage,
    RenderTextureTilingMode tiling_mode
)
{
    // Set image usage flags
    VkImageUsageFlags usage_flags = 0;
    if (texture_usage & RenderTextureUsageTransferSrc)
        usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (texture_usage & RenderTextureUsageTransferDst)
        usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (texture_usage & RenderTextureUsageSampled)
        usage_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (texture_usage & RenderTextureUsageStorage)
        usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (texture_usage & RenderTextureUsageRenderTarget)
        usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (texture_usage & RenderTextureUsageDepthStencilTarget)
        usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // Set flags, depth, and array layers based on image type
    VkImageCreateFlags image_flags = 0;
    uint32_t depth = 1;
    uint32_t array_layers = depth_or_layers;
    if (texture_type == RenderTextureType3D)
    {
        depth = depth_or_layers;
        array_layers = 1;
    }
    else if (texture_type == RenderTextureType2D && array_layers == 6)
    {
        image_flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    VkFormat const image_format = get_vulkan_format(format);
    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
    image_create_info.flags = image_flags;
    image_create_info.imageType = get_vulkan_image_type(texture_type);
    image_create_info.format = image_format;
    image_create_info.extent = VkExtent3D{ width, height, depth };
    image_create_info.mipLevels = mip_levels;
    image_create_info.arrayLayers = array_layers;
    image_create_info.samples = static_cast<VkSampleCountFlagBits>(sample_count);
    image_create_info.tiling = get_vulkan_image_tiling(tiling_mode);
    image_create_info.usage = usage_flags;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocation_create_info{};
    allocation_create_info.flags = 0;
    allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocation_create_info.requiredFlags = 0;
    allocation_create_info.preferredFlags = 0;
    allocation_create_info.memoryTypeBits = UINT32_MAX;
    allocation_create_info.pool = VK_NULL_HANDLE;
    allocation_create_info.pUserData = nullptr;
    allocation_create_info.priority = 1.0F;

    VkImage image = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    if (VK_FAILED(vmaCreateImage(m_allocator, &image_create_info, &allocation_create_info, &image, &allocation, nullptr)))
    {
        return nullptr;
    }

    // Set image view type based  on input params
    VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    if (texture_type == RenderTextureType1D && depth_or_layers == 1)
        view_type = VK_IMAGE_VIEW_TYPE_1D;
    else if (texture_type == RenderTextureType1D && depth_or_layers > 1)
        view_type = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    else if (texture_type == RenderTextureType2D && depth_or_layers == 1)
        view_type = VK_IMAGE_VIEW_TYPE_2D;
    else if (texture_type == RenderTextureType2D && depth_or_layers == 6) // Specific case for cubemaps
        view_type = VK_IMAGE_VIEW_TYPE_CUBE;
    else if (texture_type == RenderTextureType2D && depth_or_layers > 1)
        view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    else if (texture_type == RenderTextureType3D)
        view_type = VK_IMAGE_VIEW_TYPE_3D;

    VkImageAspectFlags image_aspect = get_vulkan_aspect_flags(format);
    VkImageViewCreateInfo view_create_info{};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.pNext = nullptr;
    view_create_info.flags = 0;
    view_create_info.image = image;
    view_create_info.viewType = view_type;
    view_create_info.format = image_format;
    view_create_info.components = VkComponentMapping{
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
    };
    view_create_info.subresourceRange = VkImageSubresourceRange{
        image_aspect,
        0, mip_levels,
        0, array_layers,
    };

    VkImageView image_view = VK_NULL_HANDLE;
    if (VK_FAILED(vkCreateImageView(m_device, &view_create_info, nullptr, &image_view)))
    {
        vmaDestroyImage(m_allocator, image, allocation);
        return nullptr;
    }

    VulkanTextureDesc texture_desc{};
    texture_desc.format = format;
    texture_desc.extent = { width, height, depth };

    return new VulkanTexture(m_device, m_allocator, image, image_view, allocation, texture_desc);
}

ShaderPipeline* VulkanRenderBackend::create_graphics_pipeline(GraphicsPipelineDescriptor pipeline_descriptor)
{
    /*
     * This function is quite long, but since Vulkan pipeline setup takes quite a bit of state management
     * it's acceptable.
     */
    BONSAI_ASSERT(pipeline_descriptor.color_attachment_count <= BONSAI_MAX_COLOR_ATTACHMENT_COUNT && "The number of pipeline color attachments must be less than the max number of color attachments");

    // Compiled used shader sources & store in compiled shaders array
    std::vector<IDxcBlob*> compiled_shaders{};
    std::unordered_map<VkShaderStageFlagBits, std::pair<ShaderSource, CComPtr<IDxcBlob>>> shaders{};
    if (pipeline_descriptor.vertex_shader != nullptr)
    {
        CComPtr<IDxcBlob> vertex_shader{};
        if (!compile_shader_source(*pipeline_descriptor.vertex_shader, BONSAI_TARGET_PROFILE_VS, &vertex_shader))
        {
            return nullptr;
        }
        shaders[VK_SHADER_STAGE_VERTEX_BIT] = { *pipeline_descriptor.vertex_shader, vertex_shader };
        compiled_shaders.push_back(vertex_shader);
    }

    if (pipeline_descriptor.fragment_shader != nullptr)
    {
        CComPtr<IDxcBlob> fragment_shader{};
        if (!compile_shader_source(*pipeline_descriptor.fragment_shader, BONSAI_TARGET_PROFILE_PS, &fragment_shader))
        {
            return nullptr;
        }
        shaders[VK_SHADER_STAGE_FRAGMENT_BIT] = { *pipeline_descriptor.fragment_shader, fragment_shader };
        compiled_shaders.push_back(fragment_shader);
    }

    // Reflect shader info & bindings
    SPIRVReflector reflector(compiled_shaders.data(), compiled_shaders.size());
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
    VkPipelineLayout pipeline_layout = generate_pipeline_layout(reflector, descriptor_set_layouts);
    if (pipeline_layout == VK_NULL_HANDLE)
    {
        for (auto const& layout : descriptor_set_layouts)
        {
            vkDestroyDescriptorSetLayout(m_device, layout, nullptr);
        }
        return nullptr;
    }

    // Generate shader modules & shader stages
    std::vector<VkShaderModule> shader_modules{};
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};
    for (auto const& [stage, shader_data ] : shaders)
    {
        auto const& [ shader_source, shader_code ] = shader_data;
        VkShaderModuleCreateInfo shader_module_create_info{};
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.pNext = nullptr;
        shader_module_create_info.flags = 0;
        shader_module_create_info.pCode = static_cast<uint32_t*>(shader_code->GetBufferPointer());
        shader_module_create_info.codeSize = shader_code->GetBufferSize();

        VkShaderModule shader_module = VK_NULL_HANDLE;
        if (VK_FAILED(vkCreateShaderModule(m_device, &shader_module_create_info, nullptr, &shader_module)))
        {
            return nullptr;
        }

        VkPipelineShaderStageCreateInfo shader_stage_create_info{};
        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.pNext = nullptr;
        shader_stage_create_info.flags = 0;
        shader_stage_create_info.stage = stage;
        shader_stage_create_info.module = shader_module;
        shader_stage_create_info.pName = shader_source.entrypoint;
        shader_stage_create_info.pSpecializationInfo = nullptr;

        shader_modules.push_back(shader_module);
        shader_stages.push_back(shader_stage_create_info);
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_state{};
    vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state.pNext = nullptr;
    vertex_input_state.flags = 0;
    vertex_input_state.vertexBindingDescriptionCount = reflector.get_vertex_binding_count();
    vertex_input_state.pVertexBindingDescriptions = reflector.get_vertex_bindings();
    vertex_input_state.vertexAttributeDescriptionCount = reflector.get_vertex_attribute_count();
    vertex_input_state.pVertexAttributeDescriptions = reflector.get_vertex_attributes();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state{};
    input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state.pNext = nullptr;
    input_assembly_state.flags = 0;
    input_assembly_state.topology = get_vulkan_topology(pipeline_descriptor.input_assembly_state.primitive_topology);
    input_assembly_state.primitiveRestartEnable = (pipeline_descriptor.input_assembly_state.strip_cut_value != IndexBufferStripCutValueDisabled);

    VkPipelineTessellationStateCreateInfo tessellation_state{};
    tessellation_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellation_state.pNext = nullptr;
    tessellation_state.flags = 0;
    tessellation_state.patchControlPoints = 0; // TODO(nemjit001): Reflect this from shaders

    // Viewport and scissor will be set as dynamic state during command recording
    VkViewport const viewport{ 0.0F, 0.0F, 1.0F, 1.0F, 0.0F, 1.0F };
    VkRect2D const scissor{ { 0, 0 }, { 1, 1 } };
    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pNext = nullptr;
    viewport_state.flags = 0;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterization_state{};
    rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state.pNext = nullptr;
    rasterization_state.flags = 0;
    rasterization_state.depthClampEnable = (pipeline_descriptor.rasterization_state.depth_bias_clamp > 0.0F);
    rasterization_state.rasterizerDiscardEnable = VK_FALSE; // TODO(nemjit001): Add support for disabling rasterization, find way to disable Output Merger in DX12
    rasterization_state.polygonMode = get_vulkan_polygon_mode(pipeline_descriptor.rasterization_state.polygon_mode);
    rasterization_state.cullMode = get_vulkan_cull_mode(pipeline_descriptor.rasterization_state.cull_mode);
    rasterization_state.frontFace = pipeline_descriptor.rasterization_state.front_face_counter_clockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterization_state.depthBiasEnable = (pipeline_descriptor.rasterization_state.depth_bias > 0.0F);
    rasterization_state.depthBiasConstantFactor = pipeline_descriptor.rasterization_state.depth_bias;
    rasterization_state.depthBiasClamp = pipeline_descriptor.rasterization_state.depth_bias_clamp;
    rasterization_state.depthBiasSlopeFactor = pipeline_descriptor.rasterization_state.depth_bias_slope_factor;
    rasterization_state.lineWidth = 1.0F; // DX12 only supports a line width of 1 pixel

    VkPipelineMultisampleStateCreateInfo multisample_state{};
    multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.pNext = nullptr;
    multisample_state.flags = 0;
    multisample_state.rasterizationSamples = static_cast<VkSampleCountFlagBits>(pipeline_descriptor.multisample_state.sample_count);
    multisample_state.sampleShadingEnable = VK_FALSE; // TODO(nemjit001): Enable this if the physical device supports sample rate shading
    multisample_state.minSampleShading = 0.0F; // TODO(nemjit001): Set this to the max supported sample shading rate for the physical device
    multisample_state.pSampleMask = pipeline_descriptor.multisample_state.sample_mask;
    multisample_state.alphaToCoverageEnable = pipeline_descriptor.multisample_state.alpha_to_coverage;
    multisample_state.alphaToOneEnable = VK_FALSE; // TODO(nemjit001): Check if alpha to one can be added as pipeline state

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state{};
    depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state.pNext = nullptr;
    depth_stencil_state.flags = 0;
    depth_stencil_state.depthTestEnable = pipeline_descriptor.depth_stencil_state.depth_test;
    depth_stencil_state.depthWriteEnable = pipeline_descriptor.depth_stencil_state.depth_write;
    depth_stencil_state.depthCompareOp = get_vulkan_compare_op(pipeline_descriptor.depth_stencil_state.depth_compare_op);
    depth_stencil_state.depthBoundsTestEnable = pipeline_descriptor.depth_stencil_state.depth_bounds_test;
    depth_stencil_state.stencilTestEnable = pipeline_descriptor.depth_stencil_state.stencil_test;
    depth_stencil_state.front = {
        get_vulkan_stencil_op(pipeline_descriptor.depth_stencil_state.front.fail_op),
        get_vulkan_stencil_op(pipeline_descriptor.depth_stencil_state.front.pass_op),
        get_vulkan_stencil_op(pipeline_descriptor.depth_stencil_state.front.depth_fail_op),
        get_vulkan_compare_op(pipeline_descriptor.depth_stencil_state.front.compare_op),
        pipeline_descriptor.depth_stencil_state.stencil_read_mask,
        pipeline_descriptor.depth_stencil_state.stencil_write_mask,
        0
    };
    depth_stencil_state.back = {
        get_vulkan_stencil_op(pipeline_descriptor.depth_stencil_state.back.fail_op),
        get_vulkan_stencil_op(pipeline_descriptor.depth_stencil_state.back.pass_op),
        get_vulkan_stencil_op(pipeline_descriptor.depth_stencil_state.back.depth_fail_op),
        get_vulkan_compare_op(pipeline_descriptor.depth_stencil_state.back.compare_op),
        pipeline_descriptor.depth_stencil_state.stencil_read_mask,
        pipeline_descriptor.depth_stencil_state.stencil_write_mask,
        0
    };
    depth_stencil_state.minDepthBounds = 0.0F;
    depth_stencil_state.maxDepthBounds = 1.0F;

    VkPipelineColorBlendAttachmentState color_blend_attachments[BONSAI_MAX_COLOR_ATTACHMENT_COUNT]{};
    for (uint32_t i = 0; i < BONSAI_MAX_COLOR_ATTACHMENT_COUNT; i++)
    {
        color_blend_attachments[i].blendEnable = pipeline_descriptor.color_blend_state.attachments[i].blend_enable;
        color_blend_attachments[i].srcColorBlendFactor = get_vulkan_blend_factor(pipeline_descriptor.color_blend_state.attachments[i].src_blend_factor);
        color_blend_attachments[i].dstColorBlendFactor = get_vulkan_blend_factor(pipeline_descriptor.color_blend_state.attachments[i].dst_blend_factor);
        color_blend_attachments[i].colorBlendOp = get_vulkan_blend_op(pipeline_descriptor.color_blend_state.attachments[i].blend_op);
        color_blend_attachments[i].srcAlphaBlendFactor = get_vulkan_blend_factor(pipeline_descriptor.color_blend_state.attachments[i].src_blend_alpha_factor);
        color_blend_attachments[i].dstAlphaBlendFactor = get_vulkan_blend_factor(pipeline_descriptor.color_blend_state.attachments[i].dst_blend_alpha_factor);
        color_blend_attachments[i].alphaBlendOp = get_vulkan_blend_op(pipeline_descriptor.color_blend_state.attachments[i].blend_op_alpha);
        color_blend_attachments[i].colorWriteMask = pipeline_descriptor.color_blend_state.attachments[i].color_write_mask;
    }

    VkPipelineColorBlendStateCreateInfo color_blend_state{};
    color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.pNext = nullptr;
    color_blend_state.flags = 0;
    color_blend_state.logicOpEnable = pipeline_descriptor.color_blend_state.logic_op_enable;
    color_blend_state.logicOp = get_vulkan_logic_op(pipeline_descriptor.color_blend_state.logic_op);
    color_blend_state.attachmentCount = pipeline_descriptor.color_attachment_count;
    color_blend_state.pAttachments = color_blend_attachments;
    color_blend_state.blendConstants[0] = 0.0F; // Blend constants are set during command recording
    color_blend_state.blendConstants[1] = 0.0F;
    color_blend_state.blendConstants[2] = 0.0F;
    color_blend_state.blendConstants[3] = 0.0F;

    VkDynamicState dynamic_states[] = {
        // Dynamic states that are required to reach parity with DX12 pipeline state settings during command recording.
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_BLEND_CONSTANTS,
        VK_DYNAMIC_STATE_DEPTH_BOUNDS,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
    };
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.pNext = nullptr;
    dynamic_state.flags = 0;
    dynamic_state.dynamicStateCount = std::size(dynamic_states);
    dynamic_state.pDynamicStates = dynamic_states;

    VkFormat color_attachment_formats[BONSAI_MAX_COLOR_ATTACHMENT_COUNT]{};
    for (uint32_t i = 0; i < BONSAI_MAX_COLOR_ATTACHMENT_COUNT; i++)
    {
        color_attachment_formats[i] = get_vulkan_format(pipeline_descriptor.color_attachment_formats[i]);
    }

    VkPipelineRenderingCreateInfo rendering_create_info{};
    rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rendering_create_info.pNext = nullptr;
    rendering_create_info.viewMask = 0;
    rendering_create_info.colorAttachmentCount = pipeline_descriptor.color_attachment_count;
    rendering_create_info.pColorAttachmentFormats = color_attachment_formats;
    rendering_create_info.depthAttachmentFormat = get_vulkan_format(pipeline_descriptor.depth_stencil_attachment_format);
    rendering_create_info.stencilAttachmentFormat = get_vulkan_format(pipeline_descriptor.depth_stencil_attachment_format);

    VkGraphicsPipelineCreateInfo pipeline_create_info{};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.pNext = &rendering_create_info;
    pipeline_create_info.flags = 0;
    pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
    pipeline_create_info.pStages = shader_stages.data();
    pipeline_create_info.pVertexInputState = &vertex_input_state;
    pipeline_create_info.pInputAssemblyState = &input_assembly_state;
    pipeline_create_info.pTessellationState = &tessellation_state;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pRasterizationState = &rasterization_state;
    pipeline_create_info.pMultisampleState = &multisample_state;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state;
    pipeline_create_info.pColorBlendState = &color_blend_state;
    pipeline_create_info.pDynamicState = &dynamic_state;
    pipeline_create_info.layout = pipeline_layout;
    pipeline_create_info.renderPass = VK_NULL_HANDLE;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = 0;

    VkPipeline pipeline = VK_NULL_HANDLE;
    if (VK_FAILED(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline)))
    {
        for (auto const& module : shader_modules)
        {
            vkDestroyShaderModule(m_device, module, nullptr);
        }
        vkDestroyPipelineLayout(m_device, pipeline_layout, nullptr);
        for (auto const& layout : descriptor_set_layouts)
        {
            vkDestroyDescriptorSetLayout(m_device, layout, nullptr);
        }
        return nullptr;
    }

    for (auto const& module : shader_modules)
    {
        vkDestroyShaderModule(m_device, module, nullptr);
    }
    return new VulkanShaderPipeline(ShaderPipeline::Graphics, ShaderPipeline::WorkgroupSize{}, m_device, {}, pipeline_layout, pipeline);
}

ShaderPipeline* VulkanRenderBackend::create_compute_pipeline(ComputePipelineDescriptor pipeline_descriptor)
{
    // Compile shader
    CComPtr<IDxcBlob> shader_code{};
    if (!compile_shader_source(pipeline_descriptor.compute_shader, BONSAI_TARGET_PROFILE_CS, &shader_code))
    {
        return nullptr;
    }

    // Reflect shader info & bindings
    SPIRVReflector reflector(shader_code);
    ShaderPipeline::WorkgroupSize workgroup_size{};
    reflector.get_workgroup_size(workgroup_size.x, workgroup_size.y, workgroup_size.z);

    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
    VkPipelineLayout pipeline_layout = generate_pipeline_layout(reflector, descriptor_set_layouts);
    if (pipeline_layout == VK_NULL_HANDLE)
    {
        for (auto const& layout : descriptor_set_layouts)
        {
            vkDestroyDescriptorSetLayout(m_device, layout, nullptr);
        }
        return nullptr;
    }

    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.pNext = nullptr;
    shader_module_create_info.flags = 0;
    shader_module_create_info.pCode = static_cast<uint32_t*>(shader_code->GetBufferPointer());
    shader_module_create_info.codeSize = shader_code->GetBufferSize();

    VkShaderModule shader_module = VK_NULL_HANDLE;
    if (VK_FAILED(vkCreateShaderModule(m_device, &shader_module_create_info, nullptr, &shader_module)))
    {
        vkDestroyPipelineLayout(m_device, pipeline_layout, nullptr);
        for (auto const& layout : descriptor_set_layouts)
        {
            vkDestroyDescriptorSetLayout(m_device, layout, nullptr);
        }
        return nullptr;
    }

    VkComputePipelineCreateInfo pipeline_create_info{};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_create_info.pNext = nullptr;
    pipeline_create_info.flags = 0;
    pipeline_create_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipeline_create_info.stage.pNext = nullptr;
    pipeline_create_info.stage.flags = 0;
    pipeline_create_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipeline_create_info.stage.module = shader_module;
    pipeline_create_info.stage.pName = pipeline_descriptor.compute_shader.entrypoint;
    pipeline_create_info.stage.pSpecializationInfo = nullptr;
    pipeline_create_info.layout = pipeline_layout;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = 0;

    VkPipeline pipeline = VK_NULL_HANDLE;
    if (VK_FAILED(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline)))
    {
        vkDestroyShaderModule(m_device, shader_module, nullptr);
        vkDestroyPipelineLayout(m_device, pipeline_layout, nullptr);
        for (auto const& layout : descriptor_set_layouts)
        {
            vkDestroyDescriptorSetLayout(m_device, layout, nullptr);
        }
        return nullptr;
    }

    vkDestroyShaderModule(m_device, shader_module, nullptr);
    return new VulkanShaderPipeline(ShaderPipeline::Compute, workgroup_size, m_device, descriptor_set_layouts, pipeline_layout, pipeline);
}

bool VulkanRenderBackend::has_device_extensions(
    VkPhysicalDevice device,
    std::vector<char const*> const& extension_names
)
{
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    for (auto const& name : extension_names)
    {
        bool found = false;
        for (auto const& available : available_extensions)
        {
            if (std::strcmp(name, available.extensionName) == 0)
            {
                found = true;
            }
        }

        if (!found)
        {
            return false;
        }
    }
    return true;
}

VkPhysicalDevice VulkanRenderBackend::find_physical_device(
    VkInstance instance,
    VkPhysicalDeviceProperties& device_properties,
    VulkanDeviceFeatures& enabled_device_features,
    std::vector<char const*> const& enabled_device_extensions
)
{
    // Set up features struct
    enabled_device_features.features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    enabled_device_features.features2.pNext = &enabled_device_features.vulkan11_features;

    enabled_device_features.vulkan11_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    enabled_device_features.vulkan11_features.pNext = &enabled_device_features.vulkan12_features;

    enabled_device_features.vulkan12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    enabled_device_features.vulkan12_features.pNext = &enabled_device_features.vulkan13_features;

    enabled_device_features.vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    enabled_device_features.vulkan13_features.pNext = nullptr;

    // Enumerate devices
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    for (auto const& device : devices)
    {
        vkGetPhysicalDeviceProperties(device, &device_properties);

        if (device_properties.apiVersion < BONSAI_VULKAN_VERSION)
        {
            continue;
        }

        vkGetPhysicalDeviceFeatures2(device, &enabled_device_features.features2);
        if (enabled_device_features.features2.features.samplerAnisotropy != VK_TRUE
            || enabled_device_features.vulkan12_features.descriptorIndexing != VK_TRUE
            || enabled_device_features.vulkan12_features.descriptorBindingPartiallyBound != VK_TRUE
            || enabled_device_features.vulkan12_features.descriptorBindingSampledImageUpdateAfterBind != VK_TRUE
            || enabled_device_features.vulkan12_features.descriptorBindingStorageImageUpdateAfterBind != VK_TRUE
            || enabled_device_features.vulkan12_features.descriptorBindingUniformBufferUpdateAfterBind != VK_TRUE
            || enabled_device_features.vulkan12_features.descriptorBindingStorageBufferUpdateAfterBind != VK_TRUE
            || enabled_device_features.vulkan12_features.descriptorBindingVariableDescriptorCount != VK_TRUE
            || enabled_device_features.vulkan12_features.bufferDeviceAddress != VK_TRUE
            || enabled_device_features.vulkan13_features.dynamicRendering != VK_TRUE
            || enabled_device_features.vulkan13_features.synchronization2 != VK_TRUE)
        {
            continue;
        }

        if (!has_device_extensions(device, enabled_device_extensions))
        {
            continue;
        }

        return device;
    }
    return VK_NULL_HANDLE;
}

uint32_t VulkanRenderBackend::find_queue_family(
    VkPhysicalDevice physical_device,
    std::vector<VkQueueFamilyProperties> const& queue_families,
    VkQueueFlags required_flags,
    VkQueueFlags ignored_flags,
    VkSurfaceKHR surface
)
{
    for (uint32_t queue_idx = 0; queue_idx < queue_families.size(); queue_idx++)
    {
        VkBool32 surface_support = VK_TRUE;
        if (surface != VK_NULL_HANDLE)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_idx, surface, &surface_support);
        }

        VkQueueFamilyProperties const& queue = queue_families[queue_idx];
        if (surface_support == VK_TRUE
            && (queue.queueFlags & required_flags) == required_flags
            && (queue.queueFlags & ignored_flags) == 0)
        {
            return queue_idx;
        }
    }

    return VK_QUEUE_FAMILY_IGNORED;
}

VulkanSwapchainCapabilities VulkanRenderBackend::get_swapchain_capabilities(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface
)
{
    VkSurfaceCapabilitiesKHR surface_capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    uint32_t image_count = surface_capabilities.minImageCount + 1;
    if (image_count > surface_capabilities.maxImageCount && surface_capabilities.maxImageCount != 0)
    {
        image_count = surface_capabilities.maxImageCount;
    }

    uint32_t surface_format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats.data());

    RenderFormat render_format = RenderFormatUndefined;
    VkSurfaceFormatKHR preferred_format = { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    for (auto const format : surface_formats)
    {
        if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            render_format = RenderFormatRGBA8_SRGB;
            preferred_format = format;
            break;
        }
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            render_format = RenderFormatBGRA8_SRGB;
            preferred_format = format;
            break;
        }
        if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            render_format = RenderFormatRGBA8_UNORM;
            preferred_format = format;
            break;
        }
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            render_format = RenderFormatBGRA8_UNORM;
            preferred_format = format;
            break;
        }
    }

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());

    return {
        surface_capabilities.minImageCount,
        image_count,
        render_format,
        preferred_format,
        present_modes
    };
}

bool VulkanRenderBackend::configure_swapchain(
    uint32_t width,
    uint32_t height,
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    VkDevice device,
    VulkanSwapchainCapabilities const& swap_capabilities,
    VulkanSwapchainConfiguration& swapchain_config
)
{
    for (size_t i = 0; i < swapchain_config.swap_images.size(); i++)
    {
        delete swapchain_config.swap_render_textures[i];
        vkDestroySemaphore(device, swapchain_config.swap_released_semaphores[i], nullptr);
        vkDestroyImageView(device, swapchain_config.swap_image_views[i], nullptr);
    }
    swapchain_config.swap_render_textures.clear();
    swapchain_config.swap_released_semaphores.clear();
    swapchain_config.swap_image_views.clear();

    VkSurfaceCapabilitiesKHR surface_capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    VkExtent2D image_extent = surface_capabilities.currentExtent;
    if (image_extent.width == UINT32_MAX && image_extent.height == UINT32_MAX)
    {
        image_extent.width = width;
        image_extent.height = height;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = nullptr;
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = swap_capabilities.image_count;
    swapchain_create_info.imageFormat = swap_capabilities.preferred_format.format;
    swapchain_create_info.imageColorSpace = swap_capabilities.preferred_format.colorSpace;
    swapchain_create_info.imageExtent = image_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = nullptr;
    swapchain_create_info.preTransform = surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = selected_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = swapchain_config.swapchain;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    if (VK_FAILED(vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain)))
    {
        return false;
    }
    vkDestroySwapchainKHR(device, swapchain_config.swapchain, nullptr);
    swapchain_config.image_extent = image_extent;
    swapchain_config.swapchain = swapchain;

    uint32_t swap_image_count = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &swap_image_count, nullptr);
    swapchain_config.swap_images.resize(swap_image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &swap_image_count, swapchain_config.swap_images.data());

    swapchain_config.swap_image_views.resize(swap_image_count);
    swapchain_config.swap_released_semaphores.resize(swap_image_count);
    swapchain_config.swap_render_textures.resize(swap_image_count);
    for (uint32_t i = 0; i < swap_image_count; i++)
    {
        // Create swap image view
        VkImageViewCreateInfo view_create_info{};
        view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.pNext = nullptr;
        view_create_info.flags = 0;
        view_create_info.image = swapchain_config.swap_images[i];
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_create_info.format = swap_capabilities.preferred_format.format;
        view_create_info.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
        };
        view_create_info.subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0, 1,
            0, 1,
        };

        if (VK_FAILED(vkCreateImageView(device, &view_create_info, nullptr, &swapchain_config.swap_image_views[i])))
        {
            return false;
        }

        // Create swap sync semaphore
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_create_info.pNext = nullptr;
        semaphore_create_info.flags = 0;

        if (VK_FAILED(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &swapchain_config.swap_released_semaphores[i])))
        {
            return false;
        }

        // Create render texture for swap
        VulkanTextureDesc texture_desc{};
        texture_desc.format = swap_capabilities.render_format;
        texture_desc.extent = { image_extent.width, image_extent.height, 1 };
        texture_desc.vk_aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT; // This is always a color format

        swapchain_config.swap_render_textures[i] = new VulkanTexture(
            swapchain_config.swap_images[i],
            swapchain_config.swap_image_views[i],
            texture_desc
        );
    }

    return true;
}

bool VulkanRenderBackend::compile_shader_source(ShaderSource const& source, LPCWSTR target_profile, IDxcBlob** compiled_shader) const
{
    if (source.source_kind == ShaderSourceKindInline)
    {
        DxcBuffer shader_source{};
        shader_source.Ptr = source.shader_source;
        shader_source.Size = std::strlen(source.shader_source);
        shader_source.Encoding = 0; // unknown encoding, just guess...

        char const* entrypoint = source.entrypoint;
        if (m_shader_compiler.compile_source(
            entrypoint /* use the entrypoint as shader name */,
            entrypoint, target_profile,
            shader_source, nullptr,
            true, compiled_shader))
        {
            return true;
        }
    }
    else if (source.source_kind == ShaderSourceKindFile)
    {
        char const* shader_file = source.shader_source;
        char const* entrypoint = source.entrypoint;
        if (m_shader_compiler.compile_file(
            shader_file,
            entrypoint, target_profile,
            true, compiled_shader))
        {
            return true;
        }
    }

    return false;
}

VkPipelineLayout VulkanRenderBackend::generate_pipeline_layout(SPIRVReflector const& reflector, std::vector<VkDescriptorSetLayout>& descriptor_set_layouts)
{
    // Generate descriptor bindings based on reflection data
    uint32_t const descriptor_binding_count = reflector.get_descriptor_binding_count();
    DescriptorBinding const* descriptor_bindings = reflector.get_descriptor_bindings();

    // This is a 2D vector because empty descriptor sets can exist and MUST take up a slot in the pipeline layout...
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptor_set_layout_bindings{};
    for (uint32_t i = 0; i < descriptor_binding_count; i++)
    {
        DescriptorBinding const& descriptor_binding = descriptor_bindings[i];
        if (descriptor_binding.set >= descriptor_set_layout_bindings.size())
        {
            descriptor_set_layout_bindings.resize(descriptor_binding.set + 1);
        }
        descriptor_set_layout_bindings[descriptor_binding.set].push_back(descriptor_binding.layout_binding);
    }

    descriptor_set_layouts.clear();
    descriptor_set_layouts.reserve(descriptor_set_layout_bindings.size());
    for (auto const& layout_bindings : descriptor_set_layout_bindings)
    {
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.pNext = nullptr;
        descriptor_set_layout_create_info.flags = 0;
        descriptor_set_layout_create_info.bindingCount = layout_bindings.size();
        descriptor_set_layout_create_info.pBindings = layout_bindings.data();

        VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
        if (VK_FAILED(vkCreateDescriptorSetLayout(m_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout)))
        {
            return VK_NULL_HANDLE;
        }
        descriptor_set_layouts.push_back(descriptor_set_layout);
    }

    // Create generated pipeline layout for shader
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pNext = nullptr;
    pipeline_layout_create_info.flags = 0;
    pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
    pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();
    pipeline_layout_create_info.pushConstantRangeCount = reflector.get_push_constant_range_count();
    pipeline_layout_create_info.pPushConstantRanges = reflector.get_push_constant_ranges();

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    if (VK_FAILED(vkCreatePipelineLayout(m_device, &pipeline_layout_create_info, nullptr, &pipeline_layout)))
    {
        return VK_NULL_HANDLE;
    }

    return pipeline_layout;
}
