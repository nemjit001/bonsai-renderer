#include "rhi_vulkan.hpp"
#if BONSAI_USE_VULKAN

#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include <volk.h>
#include <vk_mem_alloc.h>
#include "platform/assert.hpp"
#include "platform/logger.hpp"
#include "platform/platform_vulkan.hpp"
#include "core/die.hpp"
#include "vulkan/vulkan_buffer.hpp"
#include "vulkan/vulkan_helpers.hpp"
#include "bonsai_config.hpp"

std::vector<uint32_t> VulkanQueueFamilies::get_unique() const
{
    std::vector<uint32_t> queue_families = { graphicsFamily, transferFamily, computeFamily };
    std::sort(queue_families.begin(), queue_families.end());
    queue_families.erase(std::unique(queue_families.begin(), queue_families.end()), queue_families.end());
    return queue_families;
}

VulkanRenderDevice::VulkanRenderDevice(
    bool headless,
    VkPhysicalDevice physical_device,
    VulkanQueueFamilies const& queue_families,
    VkDevice device,
    VmaAllocator allocator
)
    :
    m_headless(headless),
    m_physical_device(physical_device),
    m_queue_families(queue_families),
    m_device(device),
    m_allocator(allocator)
{
    BONSAI_ASSERT(m_physical_device != VK_NULL_HANDLE && "Vulkan physical device was null!");
    BONSAI_ASSERT(m_device != VK_NULL_HANDLE && "Vulkan device was null!");
    BONSAI_ASSERT(m_allocator != VK_NULL_HANDLE && "Vulkan allocator was null!");

    vkGetDeviceQueue(m_device, queue_families.graphicsFamily, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, queue_families.transferFamily, 0, &m_transfer_queue);
    vkGetDeviceQueue(m_device, queue_families.computeFamily, 0, &m_compute_queue);
}

VulkanRenderDevice::~VulkanRenderDevice()
{
    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(m_device, nullptr);
}

bool VulkanRenderDevice::is_headless() const
{
    return m_headless;
}

BufferHandle VulkanRenderDevice::create_buffer(BufferDesc& desc)
{
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = nullptr;
    buffer_create_info.flags = 0;
    buffer_create_info.size = desc.size;
    buffer_create_info.usage = VulkanBuffer::get_vulkan_usage_flags(desc.usage);
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create_info.queueFamilyIndexCount = 0;
    buffer_create_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo allocation_create_info{};
    allocation_create_info.flags = 0; // TODO(nemjit001): Add mapping & host access flags
    allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    if (vmaCreateBuffer(m_allocator, &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr) != VK_SUCCESS)
    {
        return {};
    }

    return BufferHandle(new VulkanBuffer(m_allocator, buffer, allocation, desc));
}

TextureHandle VulkanRenderDevice::create_texture(TextureDesc& desc)
{
    return {};
}

CommandBufferHandle VulkanRenderDevice::create_command_buffer(CommandQueueType queue)
{
    return {};
}

RHIInstanceHandle create_rhi_instance()
{
    return RHIInstanceHandle(new VulkanRHIInstance());
}

/// @brief Default Vulkan debug callback for the RHI.
static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
    VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
    [[maybe_unused]] void* user_data
)
{
    switch (severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        BONSAI_LOG_TRACE("[Vulkan] {}", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        BONSAI_LOG_INFO("[Vulkan] {}", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        BONSAI_LOG_WARNING("[Vulkan] {}", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        BONSAI_LOG_ERROR("[Vulkan] {}", callback_data->pMessage);
        break;
    default:
        break;
    }
    return VK_FALSE;
}

VulkanRHIInstance::VulkanRHIInstance()
{
    volkInitialize();
    uint32_t vk_instance_version = volkGetInstanceVersion();
    if (vk_instance_version < BONSAI_VULKAN_VERSION)
    {
        bonsai::die("Available Vulkan version not supported");
    }

    uint32_t window_extension_count = 0;
    char const** window_extension_names = platform_enumerate_vulkan_instance_extensions(&window_extension_count);
    std::vector<char const*> enabled_extensions(window_extension_names, window_extension_names + window_extension_count);
    enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    std::vector<char const*> enabled_layers;
#ifndef NDEBUG
    enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif //NDEBUG

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, BONSAI_VERSION_MAJOR, BONSAI_VERSION_MINOR, BONSAI_VERSION_PATCH);
    app_info.pApplicationName = "Bonsai Renderer";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, BONSAI_VERSION_MAJOR, BONSAI_VERSION_MINOR, BONSAI_VERSION_PATCH);
    app_info.pEngineName = "Bonsai Renderer";
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
    VkDebugUtilsMessengerCreateInfoEXT const debug_messenger_create_info = get_debug_messenger_create_info();
    instance_create_info.pNext = &debug_messenger_create_info;
#endif //NDEBUG

    if (vkCreateInstance(&instance_create_info, nullptr, &m_instance) != VK_SUCCESS)
    {
        bonsai::die("Failed to create Vulkan instance");
    }
    volkLoadInstance(m_instance);

#ifndef NDEBUG
    if (vkCreateDebugUtilsMessengerEXT(m_instance, &debug_messenger_create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
    {
        bonsai::die("Failed to create Vulkan debug messenger");
    }
#endif //NDEBUG
}

VulkanRHIInstance::~VulkanRHIInstance()
{
#ifndef NDEBUG
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
#endif //NDEBUG
    vkDestroyInstance(m_instance, nullptr);
    volkFinalize();
}

RenderDeviceHandle VulkanRHIInstance::create_render_device(RenderDeviceDesc const& desc)
{
    // Test if device should be headless
    bool is_headless = true;
    VkSurfaceKHR compatible_surface = VK_NULL_HANDLE; // Temp surface used to test surface support for physical devices.
    if (desc.compatible_surface != nullptr)
    {
        is_headless = false;
        if (!platform_create_vulkan_surface(desc.compatible_surface, m_instance, nullptr, &compatible_surface))
        {
            return {}; // Cannot create device if we can't create the required surface.
        }
    }

    // Get physical device & required queue families
    VkPhysicalDevice physical_device = find_physical_device(m_instance, compatible_surface);
    if (physical_device == VK_NULL_HANDLE)
    {
        bonsai::die("Failed to find suitable Vulkan physical device");
    }

    VulkanQueueFamilies queue_families{};
    queue_families.graphicsFamily = find_queue_family(physical_device, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, 0, compatible_surface);
    queue_families.transferFamily = find_queue_family(physical_device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE);
    queue_families.computeFamily = find_queue_family(physical_device, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, VK_NULL_HANDLE);

    // Test possibly async queue families, fall back to shared queues otherwise
    queue_families.transferFamily = (queue_families.transferFamily == VK_QUEUE_FAMILY_IGNORED) ?
        find_queue_family(physical_device, VK_QUEUE_TRANSFER_BIT, 0, VK_NULL_HANDLE) : queue_families.transferFamily;
    queue_families.computeFamily = (queue_families.computeFamily == VK_QUEUE_FAMILY_IGNORED) ?
        find_queue_family(physical_device, VK_QUEUE_COMPUTE_BIT, 0, VK_NULL_HANDLE) : queue_families.computeFamily;

    // Enable required extensions
    std::vector<char const*> enabled_extensions;
    if (!is_headless)
    {
        enabled_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    // Enable required device features
    VkPhysicalDeviceVulkan12Features vulkan12_features{};
    vulkan12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12_features.pNext = nullptr;
    vulkan12_features.bufferDeviceAddress = true;

    VkPhysicalDeviceFeatures2 enabled_features2{};
    enabled_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    enabled_features2.pNext = &vulkan12_features;
    enabled_features2.features.samplerAnisotropy = true;

    // Set up device queue create infos
    std::vector<uint32_t> unique_queue_families = queue_families.get_unique();
    std::vector<std::vector<float>> queue_priorities{};
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
    for (auto const& queue_family : unique_queue_families)
    {
        queue_priorities.push_back({ 1.0F }); // Single queue with 100% priority
        std::vector<float> const& priorities = queue_priorities.back();

        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.pNext = nullptr;
        queue_create_info.flags = 0;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = static_cast<uint32_t>(priorities.size());
        queue_create_info.pQueuePriorities = priorities.data();

        queue_create_infos.push_back(queue_create_info);
    }

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &enabled_features2;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
    device_create_info.ppEnabledExtensionNames = enabled_extensions.data();

    VkDevice device = VK_NULL_HANDLE;
    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device) != VK_SUCCESS)
    {
        bonsai::die("Failed to create Vulkan logical device");
    }

    VmaAllocatorCreateInfo allocator_create_info{};
    allocator_create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT
        | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocator_create_info.instance = m_instance;
    allocator_create_info.physicalDevice = physical_device;
    allocator_create_info.device = device;
    allocator_create_info.vulkanApiVersion = BONSAI_VULKAN_VERSION;

    VmaVulkanFunctions vma_vulkan_functions{};
    if (vmaImportVulkanFunctionsFromVolk(&allocator_create_info, &vma_vulkan_functions) != VK_SUCCESS)
    {
        bonsai::die("Failed to load VMA Vulkan function pointers");
    }
    allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;

    VmaAllocator allocator = VK_NULL_HANDLE;
    if (vmaCreateAllocator(&allocator_create_info, &allocator) != VK_SUCCESS)
    {
        bonsai::die("Failed to create Vulkan allocator");
    }

    if (!is_headless)
    {
        vkDestroySurfaceKHR(m_instance, compatible_surface, nullptr);
    }
    return RenderDeviceHandle(new VulkanRenderDevice(is_headless, physical_device, queue_families, device, allocator));
}

VkDebugUtilsMessengerCreateInfoEXT VulkanRHIInstance::get_debug_messenger_create_info()
{
    VkDebugUtilsMessengerCreateInfoEXT create_info = {};
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
    create_info.pfnUserCallback = vk_debug_callback;
    create_info.pUserData = nullptr;

    return create_info;
}

VkPhysicalDevice VulkanRHIInstance::find_physical_device(VkInstance instance, VkSurfaceKHR compatible_surface)
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    for (auto const& device : devices)
    {
        // Test device properties
        VkPhysicalDeviceProperties2 properties2{};
        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

        vkGetPhysicalDeviceProperties2(device, &properties2);
        if (properties2.properties.apiVersion < BONSAI_VULKAN_VERSION)
        {
            continue;
        }

        // Test device features
        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = nullptr;

        VkPhysicalDeviceVulkan12Features vulkan12_features{};
        vulkan12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vulkan12_features.pNext = nullptr;
        rhi_vk::extend_pnext_chain(features2, vulkan12_features);

        vkGetPhysicalDeviceFeatures2(device, &features2);
        if (features2.features.samplerAnisotropy == VK_FALSE
            || vulkan12_features.bufferDeviceAddress == VK_FALSE)
        {
            continue;
        }

        return device;
    }

    return VK_NULL_HANDLE;
}

uint32_t VulkanRHIInstance::find_queue_family(VkPhysicalDevice physical_device, VkQueueFlags required_flags, VkQueueFlags ignored_flags, VkSurfaceKHR compatible_surface)
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; i++)
    {
        VkQueueFamilyProperties const& queue_properties = queue_families[i];
        VkBool32 surface_support = VK_TRUE; // Assume support until told otherwise
        if (compatible_surface != VK_NULL_HANDLE)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, compatible_surface, &surface_support);
        }

        if ((queue_properties.queueFlags & required_flags) == required_flags
            && (queue_properties.queueFlags & ignored_flags) == 0
            && surface_support == VK_TRUE)
        {
            return i;
        }
    }

    return VK_QUEUE_FAMILY_IGNORED;
}

#endif //BONSAI_USE_VULKAN