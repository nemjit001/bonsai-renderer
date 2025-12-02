#include "vulkan_texture.hpp"

VulkanTexture::VulkanTexture(VkImage image, VkImageView image_view)
    :
    m_image(image),
    m_image_view(image_view)
{
    //
}

VulkanTexture::VulkanTexture(VkDevice device, VmaAllocator allocator, VkImage image, VkImageView image_view, VmaAllocation allocation)
    :
    m_device(device),
    m_allocator(allocator),
    m_image(image),
    m_image_view(image_view),
    m_allocation(allocation)
{
    //
}

VulkanTexture::~VulkanTexture()
{
    // When allocator and allocation are unset the resource is externally managed
    if (m_allocator != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_device, m_image_view, nullptr);
        vmaDestroyImage(m_allocator, m_image, m_allocation);
    }
}

VkImageLayout VulkanTexture::set_next_layout(VkImageLayout next_layout)
{
    VkImageLayout const previous = m_layout;
    m_layout = next_layout;
    return previous;
}
