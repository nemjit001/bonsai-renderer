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

VkImageType VulkanTexture::get_vulkan_image_type(TextureType texture_type)
{
    switch (texture_type)
    {
    case TextureType::Type1D:
        return VK_IMAGE_TYPE_1D;
    case TextureType::Type2D:
        return VK_IMAGE_TYPE_2D;
    case TextureType::Type3D:
        return VK_IMAGE_TYPE_3D;
    }
}

VkImageUsageFlags VulkanTexture::get_vulkan_usage_flags(TextureUsageFlags usage_flags)
{
    VkImageUsageFlags texture_usage = 0;
    if (usage_flags & TextureUsageTransferSrc)
    {
        texture_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (usage_flags & TextureUsageTransferDst)
    {
        texture_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (usage_flags & TextureUsageSampled)
    {
        texture_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (usage_flags & TextureUsageStorage)
    {
        texture_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (usage_flags & TextureUsageColorAttachment)
    {
        texture_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (usage_flags & TextureUsageDepthStencilAttachment)
    {
        texture_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    return texture_usage;
}

VkSampleCountFlagBits VulkanTexture::get_vulkan_sample_count(size_t sample_count)
{
    return static_cast<VkSampleCountFlagBits>(sample_count);
}
