#include "vulkan_swap_chain.hpp"

#include "vulkan_texture.hpp"

bool VulkanSurfaceCapabilities::is_format_supported(Format format) const
{
    for (auto const& surface_format : supported_formats)
    {
        if (surface_format.format == VulkanTexture::get_vulkan_format(format))
        {
            return true;
        }
    }

    return false;
}

bool VulkanSurfaceCapabilities::is_present_mode_supported(SwapPresentMode present_mode) const
{
    for (auto const& surface_present_mode : supported_present_modes)
    {
        if (surface_present_mode == VulkanSwapChain::get_vulkan_present_mode(present_mode))
        {
            return true;
        }
    }

    return false;
}

VulkanSwapChain::VulkanSwapChain(
    VkInstance instance,
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkQueue present_queue,
    VkSurfaceKHR surface,
    VkSwapchainKHR swap_chain,
    SwapChainDesc const& desc
)
    :
    m_instance(instance),
    m_physical_device(physical_device),
    m_device(device),
    m_present_queue(present_queue),
    m_surface(surface),
    m_swap_chain(swap_chain),
    m_desc(desc)
{
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, nullptr);
    m_swap_images.resize(image_count);
    vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, m_swap_images.data());
}

VulkanSwapChain::~VulkanSwapChain()
{
    vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

VulkanSurfaceCapabilities VulkanSwapChain::get_surface_capabilities(VkPhysicalDevice physical_device, VkSurfaceKHR surface, SwapChainDesc const& desc)
{
    // Get surface capabilities
    VkSurfaceCapabilitiesKHR surface_caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_caps);

    // Get surface formats and present modes
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, surface_formats.data());

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());

    // Get preferred image count
    uint32_t preferred_image_count = desc.image_count;
    if (preferred_image_count < surface_caps.minImageCount + 1)
    {
        preferred_image_count = surface_caps.minImageCount + 1;
    }
    if (preferred_image_count > surface_caps.maxImageCount && surface_caps.maxImageCount != 0)
    {
        preferred_image_count = surface_caps.maxImageCount;
    }

    // Get surface size in pixels
    uint32_t width = surface_caps.currentExtent.width;
    uint32_t height = surface_caps.currentExtent.height;
    if (width == UINT32_MAX && height == UINT32_MAX)
    {
        width = desc.width;
        height = desc.height;
    }

    VulkanSurfaceCapabilities generated{};
    generated.preferred_image_count = preferred_image_count;
    generated.width = width;
    generated.height = height;
    generated.supported_formats = std::move(surface_formats);
    generated.supported_present_modes = std::move(present_modes);
    generated.current_transform = surface_caps.currentTransform;
    generated.composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    return generated;
}

VkPresentModeKHR VulkanSwapChain::get_vulkan_present_mode(SwapPresentMode present_mode)
{
    switch (present_mode)
    {
    case SwapPresentMode::FiFo:
        return VK_PRESENT_MODE_FIFO_KHR;
    case SwapPresentMode::Mailbox:
        return VK_PRESENT_MODE_MAILBOX_KHR;
    case SwapPresentMode::Immediate:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    default:
        break;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

bool VulkanSwapChain::resize_swap_buffers(uint32_t width, uint32_t height, SwapPresentMode present_mode)
{
    SwapChainDesc desc = m_desc;
    desc.width  = width;
    desc.height = height;
    desc.present_mode = present_mode;
    VulkanSurfaceCapabilities surface_capabilities = get_surface_capabilities(m_physical_device, m_surface, desc);
    VkPresentModeKHR chosen_present_mode = get_vulkan_present_mode(desc.present_mode);
    if (!surface_capabilities.is_present_mode_supported(desc.present_mode))
    {
        chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    }

    VkSwapchainKHR old_swap_chain = m_swap_chain;
    VkSwapchainCreateInfoKHR swap_chain_create_info{};
    swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_create_info.pNext = nullptr;
    swap_chain_create_info.flags = 0;
    swap_chain_create_info.surface = m_surface;
    swap_chain_create_info.minImageCount = surface_capabilities.preferred_image_count;
    swap_chain_create_info.imageFormat = VulkanTexture::get_vulkan_format(desc.format);
    swap_chain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swap_chain_create_info.imageExtent.width = surface_capabilities.width;
    swap_chain_create_info.imageExtent.height = surface_capabilities.height;
    swap_chain_create_info.imageArrayLayers = 1;
    swap_chain_create_info.imageUsage = VulkanTexture::get_vulkan_usage_flags(desc.usage);
    swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap_chain_create_info.queueFamilyIndexCount = 0;
    swap_chain_create_info.pQueueFamilyIndices = nullptr;
    swap_chain_create_info.preTransform = surface_capabilities.current_transform;
    swap_chain_create_info.compositeAlpha = surface_capabilities.composite_alpha;
    swap_chain_create_info.presentMode = chosen_present_mode;
    swap_chain_create_info.clipped = VK_FALSE;
    swap_chain_create_info.oldSwapchain = old_swap_chain;

    VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(m_device, &swap_chain_create_info, nullptr, &swap_chain) != VK_SUCCESS)
    {
        return false;
    }
    vkDestroySwapchainKHR(m_device, old_swap_chain, nullptr);

    // Update swap chain and desc handles
    m_swap_chain = swap_chain;
    m_desc = desc;

    // Fetch swap images
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, nullptr);
    m_swap_images.resize(image_count);
    vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, m_swap_images.data());
    return true;
}

bool VulkanSwapChain::acquire_next_image()
{
    if (vkAcquireNextImageKHR(m_device, m_swap_chain, UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &m_active_image_idx) != VK_SUCCESS)
    {
        return false;
    }

    return true;
}

bool VulkanSwapChain::present()
{
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 0; // TODO(nemjit001): Wait on render semaphore, needs RHI sync capabilities
    present_info.pWaitSemaphores = nullptr;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swap_chain;
    present_info.pImageIndices = &m_active_image_idx;
    present_info.pResults = nullptr;

    if (vkQueuePresentKHR(m_present_queue, &present_info) != VK_SUCCESS)
    {
        return false;
    }

    return true;
}

uint32_t VulkanSwapChain::current_image_idx()
{
    return m_active_image_idx;
}

uint32_t VulkanSwapChain::swap_image_count()
{
    return static_cast<uint32_t>(m_swap_images.size());
}

TextureHandle VulkanSwapChain::get_swap_image(uint32_t idx)
{
    if (idx >= m_swap_images.size())
    {
        return {};
    }

    TextureDesc texture_desc{};
    texture_desc.type = TextureType::Type2D;
    texture_desc.format = m_desc.format;
    texture_desc.width = m_desc.width;
    texture_desc.height = m_desc.height;
    texture_desc.depth_or_layers = 1;
    texture_desc.mip_levels = 1;
    texture_desc.sample_count = 1;
    texture_desc.tiling = TextureTiling::Optimal;
    texture_desc.usage = m_desc.usage;
    return TextureHandle(new VulkanTexture(m_device, m_swap_images[idx], texture_desc));
}
