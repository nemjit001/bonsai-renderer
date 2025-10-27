#include "vulkan_texture.hpp"
#if BONSAI_USE_VULKAN

VulkanTexture::VulkanTexture(VkImage image, TextureDesc const& desc)
    :
    ITexture(),
    m_imported(true),
    m_image(image),
    m_desc(desc)
{
    //
}

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
    if (m_imported)
    {
        return;
    }

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

    return VK_IMAGE_TYPE_MAX_ENUM;
}

VkFormat VulkanTexture::get_vulkan_format(Format format)
{
    switch (format)
    {
    case Format::Undefined:
        return VK_FORMAT_UNDEFINED;
    case Format::R8_UINT:
        return VK_FORMAT_R8_UINT;
    case Format::R8_SINT:
        return VK_FORMAT_R8_SINT;
    case Format::R8_UNORM:
        return VK_FORMAT_R8_UNORM;
    case Format::R8_SNORM:
        return VK_FORMAT_R8_SNORM;
    case Format::RG8_UINT:
        return VK_FORMAT_R8G8_UINT;
    case Format::RG8_SINT:
        return VK_FORMAT_R8G8_SINT;
    case Format::RG8_UNORM:
        return VK_FORMAT_R8G8_UNORM;
    case Format::RG8_SNORM:
        return VK_FORMAT_R8G8_SNORM;
    case Format::RGBA8_UINT:
        return VK_FORMAT_R8G8_UINT;
    case Format::RGBA8_SINT:
        return VK_FORMAT_R8G8B8A8_SINT;
    case Format::RGBA8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case Format::RGBA8_SNORM:
        return VK_FORMAT_R8G8B8A8_SNORM;
    case Format::RGBA8_UNORM_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case Format::BGRA8_UNORM:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case Format::BGRA8_UNORM_SRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case Format::R16_UINT:
        return VK_FORMAT_R16_UINT;
    case Format::R16_SINT:
        return VK_FORMAT_R16_SINT;
    case Format::R16_UNORM:
        return VK_FORMAT_R16_UNORM;
    case Format::R16_SNORM:
        return VK_FORMAT_R16_SNORM;
    case Format::R16_FLOAT:
        return VK_FORMAT_R16_SFLOAT;
    case Format::RG16_UINT:
        return VK_FORMAT_R16G16_UINT;
    case Format::RG16_SINT:
        return VK_FORMAT_R16G16_SINT;
    case Format::RG16_UNORM:
        return VK_FORMAT_R16G16_UNORM;
    case Format::RG16_SNORM:
        return VK_FORMAT_R16G16_SNORM;
    case Format::RG16_FLOAT:
        return VK_FORMAT_R16G16_SFLOAT;
    case Format::RGBA16_UINT:
        return VK_FORMAT_R16G16B16A16_UINT;
    case Format::RGBA16_SINT:
        return VK_FORMAT_R16G16B16A16_SINT;
    case Format::RGBA16_UNORM:
        return VK_FORMAT_R16G16B16A16_UNORM;
    case Format::RGBA16_SNORM:
        return VK_FORMAT_R16G16B16A16_SNORM;
    case Format::RGBA16_FLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case Format::R32_UINT:
        return VK_FORMAT_R32_UINT;
    case Format::R32_SINT:
        return VK_FORMAT_R32_SINT;
    case Format::R32_FLOAT:
        return VK_FORMAT_R32_SFLOAT;
    case Format::RG32_UINT:
        return VK_FORMAT_R32G32_UINT;
    case Format::RG32_SINT:
        return VK_FORMAT_R32G32_SINT;
    case Format::RG32_FLOAT:
        return VK_FORMAT_R32G32_SFLOAT;
    case Format::RGB32_UINT:
        return VK_FORMAT_R32G32B32_UINT;
    case Format::RGB32_SINT:
        return VK_FORMAT_R32G32B32_SINT;
    case Format::RGB32_FLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Format::RGBA32_UINT:
        return VK_FORMAT_R32G32B32A32_UINT;
    case Format::RGBA32_SINT:
        return VK_FORMAT_R32G32B32A32_SINT;
    case Format::RGBA32_FLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Format::Depth16:
        return VK_FORMAT_D16_UNORM;
    case Format::Depth24Stencil8:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case Format::Depth32:
        return VK_FORMAT_D32_SFLOAT;
    case Format::Depth32Stencil8:
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    default:
        break;
    }

    return VK_FORMAT_UNDEFINED;
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

VkImageTiling VulkanTexture::get_vulkan_image_tiling(TextureTiling tiling)
{
    switch (tiling)
    {
    case TextureTiling::Optimal:
        return VK_IMAGE_TILING_OPTIMAL;
    case TextureTiling::Linear:
        return VK_IMAGE_TILING_LINEAR;
    }

    return VK_IMAGE_TILING_MAX_ENUM;
}

#endif //BONSAI_USE_VULKAN