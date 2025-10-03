#include "renderer.hpp"

#include <vector>
#include <volk.h>
#include "core/die.hpp"
#include "core/logger.hpp"
#include "platform/platform_vulkan.hpp"

static constexpr uint32_t BONSAI_MINIMUM_VULKAN_VERSION = VK_API_VERSION_1_3;

struct Renderer::Impl
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    uint32_t graphics_queue_family;
    VkDevice device;
    VkQueue graphics_queue;
};

/// @brief Vulkan debug callback for routing validation data through logger.
static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
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

/// @brief Get the default debug utils configuration.
static VkDebugUtilsMessengerCreateInfoEXT get_debug_utils_create_info()
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

/// @brief Pick the first found suitable physical device for the given Vulkan instance and surface.
/// @param instance
/// @param surface
/// @return A physical device, or VK_NULL_HANDLE if no device was found.
static VkPhysicalDevice pick_physical_device(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(count);
    vkEnumeratePhysicalDevices(instance, &count, physical_devices.data());
    for (auto const& device : physical_devices)
    {
        // TODO(nemjit001): validate device
        // - Validate Vulkan API version on device w/ min version
        // - Check surface present support on at least 1 queue
        // - Check required features are supported on device
        return device;
    }

    return VK_NULL_HANDLE;
}

Renderer::Renderer(Surface const* surface)
    :
    m_impl(new Impl{})
{
    if (volkInitialize() != VK_SUCCESS)
    {
        bonsai::die("Failed to load Vulkan symbols");
    }
    BONSAI_LOG_TRACE("Loaded Vulkan symbols (v{}.{}.{})",
        VK_VERSION_MAJOR(volkGetInstanceVersion()), VK_API_VERSION_MINOR(volkGetInstanceVersion()), VK_API_VERSION_PATCH(volkGetInstanceVersion()));

    std::vector<char const*> layer_names{};
#ifndef NDEBUG
    layer_names.push_back("VK_LAYER_KHRONOS_validation");
    layer_names.push_back("VK_LAYER_KHRONOS_synchronization2");
#endif //NDEBUG

    uint32_t extension_count = 0;
    char const** window_extension_names = platform_enumerate_vulkan_instance_extensions(&extension_count);
    std::vector<char const*> extension_names(window_extension_names, window_extension_names + extension_count);
    extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = "Bonsai";
    app_info.applicationVersion = 0;
    app_info.apiVersion = BONSAI_MINIMUM_VULKAN_VERSION;

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = nullptr;
    instance_create_info.flags = 0;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = static_cast<uint32_t>(layer_names.size());
    instance_create_info.ppEnabledLayerNames = layer_names.data();
    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extension_names.size());
    instance_create_info.ppEnabledExtensionNames = extension_names.data();
#ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT const debug_create_info = get_debug_utils_create_info();
    instance_create_info.pNext = &debug_create_info;
#endif //NDEBUG
    if (vkCreateInstance(&instance_create_info, nullptr, &m_impl->instance) != VK_SUCCESS)
    {
        bonsai::die("Failed to create Vulkan instance");
    }
    volkLoadInstance(m_impl->instance);

#ifndef NDEBUG
    if (vkCreateDebugUtilsMessengerEXT(m_impl->instance, &debug_create_info, nullptr, &m_impl->debug_messenger) != VK_SUCCESS)
    {
        bonsai::die("Failed to create Vulkan debug messenger");
    }
#endif //NDEBUG
    BONSAI_LOG_TRACE("Initialized Vulkan instance");

    if (!platform_create_vulkan_surface(surface, m_impl->instance, nullptr, &m_impl->surface))
    {
        bonsai::die("Failed to create Vulkan surface");
    }
    BONSAI_LOG_TRACE("Initialized Vulkan surface");

    m_impl->physical_device = pick_physical_device(m_impl->instance, m_impl->surface);
    if (m_impl->physical_device == VK_NULL_HANDLE)
    {
        bonsai::die("Failed to select suitable physical device");
    }
    BONSAI_LOG_TRACE("Picked suitable Vulkan physical device");
}

Renderer::~Renderer()
{
    vkDestroySurfaceKHR(m_impl->instance, m_impl->surface, nullptr);
    vkDestroyDebugUtilsMessengerEXT(m_impl->instance, m_impl->debug_messenger, nullptr);
    vkDestroyInstance(m_impl->instance, nullptr);
    volkFinalize();
    delete m_impl;
}

void Renderer::on_resize(uint32_t width, uint32_t height)
{
    //
}

void Renderer::render(World const& render_world, double delta)
{
    //
}
