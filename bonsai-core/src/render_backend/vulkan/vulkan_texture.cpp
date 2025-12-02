#include "vulkan_texture.hpp"

VulkanTexture::VulkanTexture(VmaAllocator allocator, VkImage image, VmaAllocation allocation)
    :
    m_allocator(allocator),
    m_image(image),
    m_allocation(allocation)
{
    //
}

VulkanTexture::~VulkanTexture()
{
    vmaDestroyImage(m_allocator, m_image, m_allocation);
}

VkImageLayout VulkanTexture::set_next_layout(VkImageLayout next_layout)
{
    VkImageLayout const previous = m_layout;
    m_layout = next_layout;
    return previous;
}
