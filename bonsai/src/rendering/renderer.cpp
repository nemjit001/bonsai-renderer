#include "renderer.hpp"

#include <cstring>
#include <vector>
#include <unordered_set>
#include <volk.h>
#include "core/die.hpp"
#include "core/logger.hpp"
#include "platform/platform_vulkan.hpp"
#include "bonsai_config.hpp"

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
[[maybe_unused]] static VkDebugUtilsMessengerCreateInfoEXT get_debug_utils_create_info()
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

/// @brief Check if a list of validation layer names are all available in the instance.
/// @param layer_names
/// @return A boolean indicating all layer names are available in the Vulkan instance.
static bool has_validation_layers(std::vector<char const*> const& layer_names)
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> available_layers(count);
    vkEnumerateInstanceLayerProperties(&count, available_layers.data());
    for (auto const& name : layer_names)
    {
        bool found = false;
        for (auto const& available_layer : available_layers)
        {
            if (std::strcmp(name, available_layer.layerName) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            BONSAI_LOG_WARNING("Requested validation layer {} not found!", name);
            return false;
        }
    }

    return true;
}

/// @brief Validate a list of extension names against a list of available extensions.
/// @param extension_names
/// @param available_extensions
/// @return A boolean indicating all extension names are available in the available extension list.
static bool validate_extensions(std::vector<char const*> const& extension_names, std::vector<VkExtensionProperties> const& available_extensions)
{
    for (auto const& name : extension_names)
    {
        bool found = false;
        for (auto const& available_extension : available_extensions)
        {
            if (std::strcmp(name, available_extension.extensionName) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            BONSAI_LOG_WARNING("Requested extension {} not found!", name);
            return false;
        }
    }

    return true;
}

/// @brief Check if a list of instance extensions are all available.
/// @param extension_names
/// @return A boolean indicating complete availability.
static bool has_instance_extensions(std::vector<char const*> const& extension_names)
{
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, available_extensions.data());
    return validate_extensions(extension_names, available_extensions);
}

/// @brief Check if a list of device extensions are all available.
/// @param device Physical device to use for the check.
/// @param extension_names
/// @return A boolean indicating complete availability.
static bool has_device_extensions(VkPhysicalDevice device, std::vector<char const*> const& extension_names)
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available_extensions.data());
    return validate_extensions(extension_names, available_extensions);
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

/// @brief Find a queue family index by queue flags.
/// @param device Physical device to use for queue queries.
/// @param surface Surface to use for surface support queries, optional.
/// @param required Required queue flags.
/// @param ignored Ignored queue flags.
/// @return The found queue family or UINT32_MAX if no queue family was found.
static uint32_t find_queue_family(VkPhysicalDevice device, VkSurfaceKHR surface, VkQueueFlags required, VkQueueFlags ignored)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queue_families.data());

    uint32_t queue_idx = 0;
    for (auto const& queue_family : queue_families)
    {
        VkBool32 surface_support = VK_TRUE; // Assume supported
        if (surface != VK_NULL_HANDLE)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_idx, surface, &surface_support);
        }

        if ((queue_family.queueFlags & required) == required
            && (queue_family.queueFlags & ignored) == 0
            && surface_support == VK_TRUE)
        {
            return queue_idx;
        }

        queue_idx += 1;
    }

    return UINT32_MAX;
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
    if (volkGetInstanceVersion() < BONSAI_MINIMUM_VULKAN_VERSION)
    {
        bonsai::die("The installed Vulkan driver version is not compatible with Bonsai's minimum required Vulkan version");
    }

    std::vector<char const*> layer_names{};
#ifndef NDEBUG
    layer_names.push_back("VK_LAYER_KHRONOS_validation");
    layer_names.push_back("VK_LAYER_KHRONOS_synchronization2");
#endif //NDEBUG
    if (!has_validation_layers(layer_names))
    {
        bonsai::die("Not all required Vulkan validation layers are available");
    }

    uint32_t extension_count = 0;
    char const** window_extension_names = platform_enumerate_vulkan_instance_extensions(&extension_count);
    std::vector<char const*> extension_names(window_extension_names, window_extension_names + extension_count);
    extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    if (!has_instance_extensions(extension_names))
    {
        bonsai::die("Not all required Vulkan instance extensions are available");
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = "Bonsai";
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, BONSAI_VERSION_MAJOR, BONSAI_VERSION_MINOR, BONSAI_VERSION_PATCH);
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
    BONSAI_LOG_TRACE("Enabled {} instance extension(s)", extension_names.size());
    for (auto const& extension : extension_names)
    {
        BONSAI_LOG_TRACE("- {}", extension);
    }

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

    m_impl->graphics_queue_family = find_queue_family(m_impl->physical_device, m_impl->surface, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, 0);
    if (m_impl->graphics_queue_family == UINT32_MAX)
    {
        bonsai::die("Failed to find required device queue families");
    }
    BONSAI_LOG_TRACE("Picked suitable Vulkan physical device");

    std::vector<char const*> device_extension_names{};
    device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (!has_device_extensions(m_impl->physical_device, device_extension_names))
    {
        bonsai::die("Not all required Vulkan device extensions are available");
    }

    std::unordered_set<uint32_t> const unique_queue_families = { m_impl->graphics_queue_family, };
    std::vector<std::vector<float>> queue_priorities{};
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
    queue_priorities.reserve(unique_queue_families.size());
    queue_create_infos.reserve(unique_queue_families.size());
    for (auto const& queue_family : unique_queue_families)
    {
        queue_priorities.push_back({ 1.0F }); // Single queue with 100% priority

        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.pNext = nullptr;
        queue_create_info.flags = 0;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = queue_priorities.back().size();
        queue_create_info.pQueuePriorities = queue_priorities.back().data();

        queue_create_infos.push_back(queue_create_info);
    }

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = nullptr;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.enabledLayerCount = static_cast<uint32_t>(layer_names.size()); //NOTE(nemjit001): Deprecated, but required by Vulkan 1.0
    device_create_info.ppEnabledLayerNames = layer_names.data();
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extension_names.size());
    device_create_info.ppEnabledExtensionNames = device_extension_names.data();
    device_create_info.pEnabledFeatures = nullptr;

    if (vkCreateDevice(m_impl->physical_device, &device_create_info, nullptr, &m_impl->device) != VK_SUCCESS)
    {
        bonsai::die("Failed to create Vulkan device");
    }
    volkLoadDevice(m_impl->device);
    vkGetDeviceQueue(m_impl->device, m_impl->graphics_queue_family, 0, &m_impl->graphics_queue);
    BONSAI_LOG_TRACE("Initialized Vulkan device");
    BONSAI_LOG_TRACE("Enabled {} device extension(s)", device_extension_names.size());
    for (auto const& extension : device_extension_names)
    {
        BONSAI_LOG_TRACE("- {}", extension);
    }
}

Renderer::~Renderer()
{
    vkDestroyDevice(m_impl->device, nullptr);
    vkDestroySurfaceKHR(m_impl->instance, m_impl->surface, nullptr);
    vkDestroyDebugUtilsMessengerEXT(m_impl->instance, m_impl->debug_messenger, nullptr);
    vkDestroyInstance(m_impl->instance, nullptr);
    volkFinalize();
    delete m_impl;
}

void Renderer::on_resize(uint32_t width, uint32_t height)
{
    vkDeviceWaitIdle(m_impl->device);
    // TODO(nemjit001): Resize swap chain buffers & any dependent resources
}

void Renderer::render(World const& render_world, double delta)
{
    //
}
