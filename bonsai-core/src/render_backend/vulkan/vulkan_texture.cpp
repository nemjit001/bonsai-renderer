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
