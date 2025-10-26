#include "rhi_vulkan.hpp"
#if BONSAI_USE_VULKAN

#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include <volk.h>
#include <vk_mem_alloc.h>
#include <vector>
#include "platform/assert.hpp"
#include "platform/logger.hpp"
#include "platform/platform_vulkan.hpp"
#include "core/die.hpp"
#include "bonsai_config.hpp"

VulkanRenderDevice::VulkanRenderDevice(
    VkPhysicalDevice physical_device,
    VulkanQueueFamilies const& queue_families,
    VkDevice device,
    VmaAllocator allocator
)
    :
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
    return true;
}

BufferHandle VulkanRenderDevice::create_buffer(BufferDesc& desc)
{
    return {};
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

static VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info()
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
    return RenderDeviceHandle(new VulkanRenderDevice(VK_NULL_HANDLE, {}, VK_NULL_HANDLE, VK_NULL_HANDLE));
}

#endif //BONSAI_USE_VULKAN