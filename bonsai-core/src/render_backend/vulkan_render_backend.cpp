#include "vulkan_render_backend.hpp"
#define VOLK_IMPLEMENTATION

#include <algorithm>
#include <backends/imgui_impl_vulkan.h>
#include <volk.h>

#include "bonsai/core/fatal_exit.hpp"
#include "bonsai/core/logger.hpp"
#include "vulkan/vk_check.hpp"
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

VulkanRenderBackend::VulkanRenderBackend(PlatformSurface* platform_surface)
{
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

    m_swapchain_capabilities = get_swapchain_capabilities(m_physical_device, m_surface);
    if (!configure_swapchain(m_main_surface, m_physical_device, m_surface, m_device, m_swapchain_capabilities, m_swapchain_config))
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

    vkDestroySemaphore(m_device, m_swap_available, nullptr);
    vkDestroyFence(m_device, m_frame_ready, nullptr);

    for (size_t i = 0; i < m_swapchain_config.swap_images.size(); i++)
    {
        vkDestroySemaphore(m_device, m_swapchain_config.swap_released_semaphores[i], nullptr);
        vkDestroyImageView(m_device, m_swapchain_config.swap_image_views[i], nullptr);
    }
    vkDestroySwapchainKHR(m_device, m_swapchain_config.swapchain, nullptr);

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

    vkResetFences(m_device, 1, &m_frame_ready);
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
    frame_submit_info.commandBufferCount = 0; // TODO(nemjit001): Submit frame command buffer here :)
    frame_submit_info.pCommandBuffers = nullptr;
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
        return RenderBackendFrameResult::SwapOutOfDate;
    }
    else if (VK_FAILED(present_result))
    {
        return RenderBackendFrameResult::FatalError;
    }

    return RenderBackendFrameResult::Ok;
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

    VkSurfaceFormatKHR preferred_format = surface_formats[0];
    for (auto const format : surface_formats)
    {
        if ((format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB)
            && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            preferred_format = format;
            break;
        }
        if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
            && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
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
        preferred_format,
        present_modes
    };
}

bool VulkanRenderBackend::configure_swapchain(
    PlatformSurface const* platform_surface,
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    VkDevice device,
    VulkanSwapchainCapabilities const& swap_capabilities,
    VulkanSwapchainConfiguration& swapchain_config
)
{
    for (auto const& image_view : swapchain_config.swap_image_views)
    {
        vkDestroyImageView(device, image_view, nullptr);
    }

    for (auto const& semaphore : swapchain_config.swap_released_semaphores)
    {
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    VkSurfaceCapabilitiesKHR surface_capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    VkExtent2D image_extent = surface_capabilities.currentExtent;
    if (image_extent.width == UINT32_MAX && image_extent.height == UINT32_MAX)
    {
        platform_surface->get_size_in_pixels(image_extent.width, image_extent.height);
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
    swapchain_config.swapchain = swapchain;

    uint32_t swap_image_count = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &swap_image_count, nullptr);
    swapchain_config.swap_images.resize(swap_image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &swap_image_count, swapchain_config.swap_images.data());

    swapchain_config.swap_image_views.resize(swap_image_count);
    swapchain_config.swap_released_semaphores.resize(swap_image_count);
    for (uint32_t i = 0; i < swap_image_count; i++)
    {
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

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (VK_FAILED(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &swapchain_config.swap_released_semaphores[i])))
        {
            return false;
        }
    }

    return true;
}
