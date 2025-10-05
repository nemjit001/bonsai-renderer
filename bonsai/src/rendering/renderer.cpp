#include "renderer.hpp"
#if BONSAI_USE_VULKAN

#include <cstring>
#include <unordered_set>
#include <vector>
#include <volk.h>
#include "core/die.hpp"
#include "core/logger.hpp"
#include "platform/platform.hpp"
#include "platform/platform_vulkan.hpp"
#include "bonsai_config.hpp"

/// @brief Minimum supported Vulkan API version against which Bonsai is written.
static constexpr uint32_t   BONSAI_MINIMUM_VULKAN_VERSION   = VK_API_VERSION_1_3;
/// @brief Number of frames in flight that Bonsai initializes with.
static constexpr size_t     BONSAI_FRAMES_IN_FLIGHT         = 2;

/// @brief Swap chain capabilities, contains possible config values that can be used to initialize the SwapchainConfig.
struct SwapchainCapabilities
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> present_infos;
};

/// @brief Swap chain configuration state, selected from swap capabilities.
struct SwapchainConfig
{
    VkSurfaceKHR surface;
    uint32_t image_count;
    VkExtent2D extent;
    VkFormat preferred_format;
    VkColorSpaceKHR preferred_color_space;
    VkPresentModeKHR preferred_present_mode;
    VkSurfaceTransformFlagBitsKHR current_transform;
};

/// @brief Vulkan frame state, contains state that is better kept separated between frames.
struct FrameState
{
    VkFence frame_ready;
    VkSemaphore swap_available;
    VkSemaphore rendering_finished;
    VkCommandPool graphics_pool;
    VkCommandPool compute_pool;
    VkCommandPool transfer_pool;
    VkCommandBuffer frame_commands;
};

/// @brief Vulkan Renderer implementation.
struct Renderer::Impl
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    uint32_t graphics_queue_family;
    uint32_t compute_queue_family;
    uint32_t transfer_queue_family;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue compute_queue;
    VkQueue transfer_queue;
    SwapchainConfig swap_config;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swap_images;
    std::vector<VkImageView> swap_image_views;
    std::vector<FrameState> frame_states;
    uint64_t frame_index;
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
        // Check device properties
        VkPhysicalDeviceProperties2 device_properties2{};
        device_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

        vkGetPhysicalDeviceProperties2(device, &device_properties2);
        if (device_properties2.properties.apiVersion < BONSAI_MINIMUM_VULKAN_VERSION)
        {
            continue;
        }

        // Check required device features
        VkPhysicalDeviceVulkan13Features vulkan13_features{};
        vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

        VkPhysicalDeviceFeatures2 device_features2{};
        device_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        device_features2.pNext = &vulkan13_features;

        vkGetPhysicalDeviceFeatures2(device, &device_features2);
        if (device_features2.features.samplerAnisotropy == VK_FALSE
            || vulkan13_features.synchronization2 == VK_FALSE)
        {
            continue;
        }

        // TODO(nemjit001): Check physical device surface support before returning the device
        BONSAI_LOG_TRACE("Found suitable Vulkan device: {} ({})", device_properties2.properties.deviceName, device_properties2.properties.deviceID);
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

static SwapchainCapabilities get_swap_capabilities(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR surface_capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surface_capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, surface_formats.data());

    uint32_t mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, present_modes.data());

    SwapchainCapabilities capabilities{};
    capabilities.surface_capabilities = surface_capabilities;
    capabilities.surface_formats = surface_formats;
    capabilities.present_infos = present_modes;
    return capabilities;
}

/// @brief Get a swap chain configuration from a physical device and surface.
/// @param device Physical device to use for configuration query.
/// @param surface Surface to use for configuration query.
/// @param width Surface width in pixels.
/// @param height Surface height in pixels.
/// @return A new SwapchainConfig structure.
static SwapchainConfig get_swap_configuration(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height)
{
    SwapchainCapabilities const capabilities = get_swap_capabilities(device, surface);
    uint32_t image_count = capabilities.surface_capabilities.minImageCount + 1;
    if (image_count > capabilities.surface_capabilities.maxImageCount && capabilities.surface_capabilities.maxImageCount != 0)
    {
        image_count = capabilities.surface_capabilities.maxImageCount;
    }

    VkExtent2D image_extent = capabilities.surface_capabilities.currentExtent;
    if (image_extent.width == UINT32_MAX || image_extent.height == UINT32_MAX)
    {
        image_extent = VkExtent2D { width, height };
    }

    VkSurfaceFormatKHR preferred_surface_format = capabilities.surface_formats[0]; // Default to 1st available format
    for (auto const& format : capabilities.surface_formats)
    {
        if (format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            preferred_surface_format = format;
            break;
        }

        if (format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
        {
            preferred_surface_format = format;
            break;
        }
    }

    SwapchainConfig config{};
    config.surface = surface;
    config.image_count = image_count;
    config.extent = image_extent;
    config.preferred_format = preferred_surface_format.format;
    config.preferred_color_space = preferred_surface_format.colorSpace;
    config.preferred_present_mode = VK_PRESENT_MODE_FIFO_KHR; // Always supported
    config.current_transform = capabilities.surface_capabilities.currentTransform;
    return config;
}

/// @brief Create a swap chain using a given swap chain configuration.
/// @param device Logical device to use for swap chain creation.
/// @param swapchain_config Swap chain configuration to use.
/// @param old_swapchain Old swap chain, optional.
/// @param out_swapchain Output swap chain.
/// @return A VkResult indicating creation status.
static VkResult create_swapchain(VkDevice device, SwapchainConfig const& swapchain_config, VkSwapchainKHR old_swapchain, VkSwapchainKHR* out_swapchain)
{
    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = nullptr;
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = swapchain_config.surface;
    swapchain_create_info.minImageCount = swapchain_config.image_count;
    swapchain_create_info.imageFormat = swapchain_config.preferred_format;
    swapchain_create_info.imageColorSpace = swapchain_config.preferred_color_space;
    swapchain_create_info.imageExtent = swapchain_config.extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = nullptr;
    swapchain_create_info.preTransform = swapchain_config.current_transform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = swapchain_config.preferred_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = old_swapchain;

    if (vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, out_swapchain) != VK_SUCCESS)
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    vkDestroySwapchainKHR(device, old_swapchain, nullptr);
    return VK_SUCCESS;
}

/// @brief Create swap chain image resources.
/// @param device
/// @param swapchain
/// @param swapchain_config
/// @param out_swap_images
/// @param out_swap_image_views
static void create_swapchain_image_resources(
    VkDevice device,
    VkSwapchainKHR swapchain,
    SwapchainConfig const& swapchain_config,
    std::vector<VkImage>& out_swap_images,
    std::vector<VkImageView>& out_swap_image_views
)
{
    uint32_t swap_image_count = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &swap_image_count, nullptr);
    out_swap_images.resize(swap_image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &swap_image_count, out_swap_images.data());

    out_swap_image_views.clear();
    out_swap_image_views.reserve(swap_image_count);
    for (auto const& image : out_swap_images)
    {
        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.pNext = nullptr;
        image_view_create_info.flags = 0;
        image_view_create_info.image = image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = swapchain_config.preferred_format;
        image_view_create_info.components = VkComponentMapping{};
        image_view_create_info.subresourceRange = VkImageSubresourceRange{
            VK_IMAGE_ASPECT_COLOR_BIT,
            0, 1,
            0, 1,
        };

        VkImageView view = VK_NULL_HANDLE;
        if (vkCreateImageView(device, &image_view_create_info, nullptr, &view) != VK_SUCCESS)
        {
            bonsai::die("Failed to create Vulkan swap image view");
        }
        out_swap_image_views.push_back(view);
    }
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
    m_impl->compute_queue_family = find_queue_family(m_impl->physical_device, VK_NULL_HANDLE, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    m_impl->transfer_queue_family = find_queue_family(m_impl->physical_device, VK_NULL_HANDLE, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (m_impl->compute_queue_family == UINT32_MAX
        || m_impl->transfer_queue_family == UINT32_MAX)
    {   // Search for queues that support compute or transfer without specifically being unique from the graphics queue
        m_impl->compute_queue_family = find_queue_family(m_impl->physical_device, VK_NULL_HANDLE, VK_QUEUE_COMPUTE_BIT, 0);
        m_impl->transfer_queue_family = find_queue_family(m_impl->physical_device, VK_NULL_HANDLE, VK_QUEUE_TRANSFER_BIT, 0);
    }

    if (m_impl->graphics_queue_family == UINT32_MAX
        || m_impl->compute_queue_family == UINT32_MAX
        || m_impl->transfer_queue_family == UINT32_MAX)
    {
        bonsai::die("Failed to find required device queue families");
    }

    std::vector<char const*> device_extension_names{};
    device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (!has_device_extensions(m_impl->physical_device, device_extension_names))
    {
        bonsai::die("Not all required Vulkan device extensions are available");
    }

    VkPhysicalDeviceVulkan13Features vulkan13_features{};
    vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vulkan13_features.synchronization2 = VK_TRUE;

    VkPhysicalDeviceFeatures2 device_features2{};
    device_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    device_features2.pNext = &vulkan13_features;
    device_features2.features.samplerAnisotropy = VK_TRUE;

    std::unordered_set<uint32_t> const unique_queue_families = {
        m_impl->graphics_queue_family,
        m_impl->compute_queue_family,
        m_impl->transfer_queue_family,
    };
    std::vector<std::vector<float>> queue_priorities{};
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
    queue_priorities.reserve(unique_queue_families.size()); // Reserve here is needed to keep pointers to queue priorities stable across vector pushes
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
    device_create_info.pNext = &device_features2;
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
    vkGetDeviceQueue(m_impl->device, m_impl->compute_queue_family, 0, &m_impl->compute_queue);
    vkGetDeviceQueue(m_impl->device, m_impl->transfer_queue_family, 0, &m_impl->transfer_queue);
    BONSAI_LOG_TRACE("Initialized Vulkan device");
    BONSAI_LOG_TRACE("Enabled {} device extension(s)", device_extension_names.size());
    for (auto const& extension : device_extension_names)
    {
        BONSAI_LOG_TRACE("- {}", extension);
    }

    uint32_t surface_width = 0, surface_height = 0;
    surface->get_size(surface_width, surface_height);
    m_impl->swap_config = get_swap_configuration(m_impl->physical_device, m_impl->surface, surface_width, surface_height);
    if (create_swapchain(m_impl->device, m_impl->swap_config, VK_NULL_HANDLE, &m_impl->swapchain) != VK_SUCCESS)
    {
        bonsai::die("Failed to create Vulkan swap chain");
    }
    create_swapchain_image_resources(m_impl->device, m_impl->swapchain, m_impl->swap_config, m_impl->swap_images, m_impl->swap_image_views);
    BONSAI_LOG_TRACE("Initialized Vulkan swap chain");

    BONSAI_LOG_TRACE("Starting Vulkan FrameState initialization");
    m_impl->frame_states.reserve(BONSAI_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < BONSAI_FRAMES_IN_FLIGHT; i++)
    {
        FrameState frame_state{};

        VkFenceCreateInfo frame_fence_create_info{};
        frame_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        frame_fence_create_info.pNext = nullptr;
        frame_fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_create_info.pNext = nullptr;
        semaphore_create_info.flags = 0;

        if (vkCreateFence(m_impl->device, &frame_fence_create_info, nullptr, &frame_state.frame_ready) != VK_SUCCESS
            || vkCreateSemaphore(m_impl->device, &semaphore_create_info, nullptr, &frame_state.swap_available) != VK_SUCCESS
            || vkCreateSemaphore(m_impl->device, &semaphore_create_info, nullptr, &frame_state.rendering_finished) != VK_SUCCESS)
        {
            bonsai::die("Failed to create Vulkan FrameState sync primitives");
        }
        BONSAI_LOG_TRACE("Initialized Vulkan FrameState sync primitives (frame {})", i + 1);

        VkCommandPoolCreateInfo graphics_pool_create_info{};
        graphics_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        graphics_pool_create_info.pNext = nullptr;
        graphics_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        graphics_pool_create_info.queueFamilyIndex = m_impl->graphics_queue_family;

        VkCommandPoolCreateInfo compute_pool_create_info{};
        compute_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        compute_pool_create_info.pNext = nullptr;
        compute_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Used as async compute
        compute_pool_create_info.queueFamilyIndex = m_impl->compute_queue_family;

        VkCommandPoolCreateInfo transfer_pool_create_info{};
        transfer_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        transfer_pool_create_info.pNext = nullptr;
        transfer_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Used for one-shot transfer jobs
        transfer_pool_create_info.queueFamilyIndex = m_impl->transfer_queue_family;

        if (vkCreateCommandPool(m_impl->device, &graphics_pool_create_info, nullptr, &frame_state.graphics_pool) != VK_SUCCESS
            || vkCreateCommandPool(m_impl->device, &compute_pool_create_info, nullptr, &frame_state.compute_pool) != VK_SUCCESS
            || vkCreateCommandPool(m_impl->device, &transfer_pool_create_info, nullptr, &frame_state.transfer_pool) != VK_SUCCESS)
        {
            bonsai::die("Failed to create Vulkan FrameState command pools");
        }

        VkCommandBufferAllocateInfo frame_command_buffer_allocate_info{};
        frame_command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        frame_command_buffer_allocate_info.pNext = nullptr;
        frame_command_buffer_allocate_info.commandPool = frame_state.graphics_pool;
        frame_command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        frame_command_buffer_allocate_info.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_impl->device, &frame_command_buffer_allocate_info, &frame_state.frame_commands) != VK_SUCCESS)
        {
            bonsai::die("Failed to allocate Vulkan frame command buffer");
        }
        BONSAI_LOG_TRACE("Initialized Vulkan FrameState command state (frame {})", i + 1);
        m_impl->frame_states.push_back(frame_state);
    }
    BONSAI_LOG_TRACE("Initialized Vulkan frame states ({} frames in flight)", BONSAI_FRAMES_IN_FLIGHT);
    m_impl->frame_index = 0;
}

Renderer::~Renderer()
{
    vkDeviceWaitIdle(m_impl->device);
    for (auto const& frame_state : m_impl->frame_states)
    {
        vkDestroyCommandPool(m_impl->device, frame_state.transfer_pool, nullptr);
        vkDestroyCommandPool(m_impl->device, frame_state.compute_pool, nullptr);
        vkDestroyCommandPool(m_impl->device, frame_state.graphics_pool, nullptr);

        vkDestroyFence(m_impl->device, frame_state.frame_ready, nullptr);
        vkDestroySemaphore(m_impl->device, frame_state.swap_available, nullptr);
        vkDestroySemaphore(m_impl->device, frame_state.rendering_finished, nullptr);
    }

    for (auto const& view : m_impl->swap_image_views)
    {
        vkDestroyImageView(m_impl->device, view, nullptr);
    }
    vkDestroySwapchainKHR(m_impl->device, m_impl->swapchain, nullptr);

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
    m_impl->swap_config = get_swap_configuration(m_impl->physical_device, m_impl->surface, width, height);
    if (width == 0 || height == 0)
    {
        return;
    }

    for (auto const& view : m_impl->swap_image_views)
    {
        vkDestroyImageView(m_impl->device, view, nullptr);
    }
    m_impl->swap_image_views.clear();
    m_impl->swap_images.clear();

    VkSwapchainKHR old_swapchain = m_impl->swapchain;
    if (create_swapchain(m_impl->device, m_impl->swap_config, old_swapchain, &m_impl->swapchain) != VK_SUCCESS)
    {
        bonsai::die("Failed to recreate Vulkan swap chain");
    }
    create_swapchain_image_resources(m_impl->device, m_impl->swapchain, m_impl->swap_config, m_impl->swap_images, m_impl->swap_image_views);
    BONSAI_LOG_TRACE("Recreated swap resources");
}

void Renderer::render(World const& render_world, double delta)
{
    size_t const current_frame_index = m_impl->frame_index % m_impl->frame_states.size();
    if (m_impl->swap_config.extent.width == 0 || m_impl->swap_config.extent.height == 0)
    {
        return;
    }

    FrameState const& frame_state = m_impl->frame_states[current_frame_index];
    vkWaitForFences(m_impl->device, 1, &frame_state.frame_ready, VK_TRUE, UINT64_MAX);

    uint32_t swap_image_idx = 0;
    if (vkAcquireNextImageKHR(m_impl->device, m_impl->swapchain, UINT64_MAX, frame_state.swap_available, VK_NULL_HANDLE, &swap_image_idx) != VK_SUCCESS)
    {
        bonsai::die("Failed to acquire swap image");
    }
    vkResetFences(m_impl->device, 1, &frame_state.frame_ready);

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = nullptr;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    vkResetCommandBuffer(frame_state.frame_commands, 0);
    if (vkBeginCommandBuffer(frame_state.frame_commands, &command_buffer_begin_info) != VK_SUCCESS)
    {
        bonsai::die("Failed to start command recording for frame ({})", m_impl->frame_index);
    }

    VkImageMemoryBarrier2 swap_present_barrier{};
    swap_present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    swap_present_barrier.pNext = nullptr;
    swap_present_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    swap_present_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    swap_present_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    swap_present_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    swap_present_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    swap_present_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    swap_present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    swap_present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    swap_present_barrier.image = m_impl->swap_images[swap_image_idx];
    swap_present_barrier.subresourceRange = VkImageSubresourceRange{
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, 1,
        0, 1,
    };

    VkDependencyInfo swap_dependencies{};
    swap_dependencies.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    swap_dependencies.pNext = nullptr;
    swap_dependencies.dependencyFlags = 0;
    swap_dependencies.imageMemoryBarrierCount = 1;
    swap_dependencies.pImageMemoryBarriers = &swap_present_barrier;

    vkCmdPipelineBarrier2(frame_state.frame_commands, &swap_dependencies);

    if (vkEndCommandBuffer(frame_state.frame_commands) != VK_SUCCESS)
    {
        bonsai::die("Failed to end command recording for frame ({})", m_impl->frame_index);
    }

    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.pWaitSemaphores = &frame_state.swap_available;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame_state.frame_commands;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &frame_state.rendering_finished;

    if (vkQueueSubmit(m_impl->graphics_queue, 1, &submit_info, frame_state.frame_ready) != VK_SUCCESS)
    {
        bonsai::die("Failed to submit rendered frame ({})", m_impl->frame_index);
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &frame_state.rendering_finished;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_impl->swapchain;
    present_info.pImageIndices = &swap_image_idx;
    present_info.pResults = nullptr;

    if (vkQueuePresentKHR(m_impl->graphics_queue, &present_info) != VK_SUCCESS)
    {
        bonsai::die("Failed to present rendered frame({})", m_impl->frame_index);
    }

    m_impl->frame_index += 1;
}

#endif //BONSAI_USE_VULKAN