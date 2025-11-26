#include "vulkan_render_backend.hpp"
#define VOLK_IMPLEMENTATION

#include <volk.h>

#include "bonsai/core/fatal_exit.hpp"
#include "bonsai/core/logger.hpp"
#include "vulkan/vk_check.hpp"

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

VulkanRenderBackend::VulkanRenderBackend(PlatformSurface* platform_surface)
{
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
    app_info.applicationVersion = 0;
    app_info.pEngineName = "Bonsai Renderer";
    app_info.engineVersion = 0;
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

    if (!platform_surface->create_vulkan_surface(m_instance, nullptr, &m_surface))
    {
        BONSAI_FATAL_EXIT("Failed to create Vulkan surface\n");
    }

    VkPhysicalDeviceProperties device_properties{};
    VulkanDeviceFeatures enabled_features{};
    m_physical_device = find_physical_device(m_instance, device_properties, enabled_features);
    if (m_physical_device == VK_NULL_HANDLE)
    {
        BONSAI_FATAL_EXIT("Failed to find suitable physical device\n");
    }
    BONSAI_ENGINE_LOG_TRACE("Selected Vulkan physical device: \"{} ({})\"",
        device_properties.deviceName,
        device_properties.deviceID
    );
}

VulkanRenderBackend::~VulkanRenderBackend()
{
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
#ifndef NDEBUG
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
#endif //NDEBUG
    vkDestroyInstance(m_instance, nullptr);
    volkFinalize();
}

VkPhysicalDevice VulkanRenderBackend::find_physical_device(
    VkInstance instance,
    VkPhysicalDeviceProperties& device_properties,
    VulkanDeviceFeatures& enabled_device_features
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

        return device;
    }
    return VK_NULL_HANDLE;
}
