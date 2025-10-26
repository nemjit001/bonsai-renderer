#include "vulkan_texture.hpp"

VulkanTexture::VulkanTexture(VmaAllocator allocator, VkImage image, VmaAllocation allocation, TextureDesc const& desc)
    :
    ITexture(),
    m_allocator(allocator),
    m_image(image),
    m_allocation(allocation),
    m_desc(desc)
{
    //
}

VulkanTexture::~VulkanTexture()
{
    vmaDestroyImage(m_allocator, m_image, m_allocation);
}
