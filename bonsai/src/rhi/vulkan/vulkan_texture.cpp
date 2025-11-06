#include "vulkan_texture.hpp"

VulkanTextureView::VulkanTextureView(VkDevice device, VkImageView view)
    :
    m_device(device),
    m_view(view)
{
    //
}

VulkanTextureView::~VulkanTextureView()
{
    vkDestroyImageView(m_device, m_view, nullptr);
}

VkImageViewType VulkanTextureView::get_vulkan_view_type(TextureViewType view_type)
{
    switch (view_type)
    {
    case TextureViewType::Type1D:
        return VK_IMAGE_VIEW_TYPE_1D;
    case TextureViewType::Type2D:
        return VK_IMAGE_VIEW_TYPE_2D;
    case TextureViewType::Type3D:
        return VK_IMAGE_VIEW_TYPE_3D;
    case TextureViewType::TypeCube:
        return VK_IMAGE_VIEW_TYPE_CUBE;
    case TextureViewType::Type1DArray:
        return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case TextureViewType::Type2DArray:
        return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case TextureViewType::TypeCubeArray:
        return VK_IMAGE_VIEW_TYPE_CUBE;
    default:
        break;
    }

    return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}

VkImageAspectFlags VulkanTextureView::get_vulkan_aspect_flags(Format format)
{
    switch (format)
    {
    case Format::Undefined:
        return 0;
    case Format::Depth16:
    case Format::Depth32:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case Format::Depth24Stencil8:
    case Format::Depth32Stencil8:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
        break;
    }

    return VK_IMAGE_ASPECT_COLOR_BIT;
}

VulkanTexture::VulkanTexture(VkDevice device, VkImage image, TextureDesc const& desc)
    :
    ITexture(),
    m_imported(true),
    m_device(device),
    m_image(image),
    m_desc(desc)
{
    //
}

VulkanTexture::VulkanTexture(VkDevice device, VmaAllocator allocator, VkImage image, VmaAllocation allocation, TextureDesc const& desc)
    :
    ITexture(),
    m_device(device),
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
    default:
        break;
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
    default:
        break;
    }

    return VK_IMAGE_TILING_MAX_ENUM;
}

VkImageLayout VulkanTexture::get_vulkan_image_layout(TextureLayout layout)
{
    switch (layout)
    {
    case TextureLayout::Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case TextureLayout::General:
        return VK_IMAGE_LAYOUT_GENERAL;
    case TextureLayout::ColorAttachment:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case TextureLayout::DepthStencilAttachment:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case TextureLayout::DepthStencilAttachmentReadOnly:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case TextureLayout::ShaderResource:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case TextureLayout::TransferSrc:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case TextureLayout::TransferDst:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case TextureLayout::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:
        break;
    }

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

TextureViewHandle VulkanTexture::create_view(TextureViewDesc const* view_desc)
{
    VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    VkFormat view_format = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags image_aspect = 0;
    if (view_desc != nullptr)
    {
        view_type = VulkanTextureView::get_vulkan_view_type(view_desc->type);
        view_format = get_vulkan_format(view_desc->format);
        image_aspect = VulkanTextureView::get_vulkan_aspect_flags(view_desc->format); // TODO(nemjit001): Let view creator decide this
    }
    else
    {
        if (m_desc.type == TextureType::Type1D && m_desc.depth_or_layers == 1)
        {
            view_type = VK_IMAGE_VIEW_TYPE_1D;
        }
        else if (m_desc.type == TextureType::Type1D && m_desc.depth_or_layers > 1)
        {
            view_type = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        }
        else if (m_desc.type == TextureType::Type2D && m_desc.depth_or_layers == 1)
        {
            view_type = VK_IMAGE_VIEW_TYPE_2D;
        }
        else if (m_desc.type == TextureType::Type2D && m_desc.depth_or_layers > 1)
        {
            view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }
        else if (m_desc.type == TextureType::Type3D)
        {
            view_type = VK_IMAGE_VIEW_TYPE_3D;
        }

        view_format = get_vulkan_format(m_desc.format);
        image_aspect = VulkanTextureView::get_vulkan_aspect_flags(m_desc.format);
    }

    VkImageViewCreateInfo view_create_info = {};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.pNext = nullptr;
    view_create_info.flags = 0;
    view_create_info.image = m_image;
    view_create_info.viewType = view_type;
    view_create_info.format = view_format;
    view_create_info.components = VkComponentMapping{};
    view_create_info.subresourceRange = VkImageSubresourceRange{
        image_aspect,
        0, static_cast<uint32_t>(m_desc.mip_levels), // TODO(nemjit001): Let creator decide subresource range for view
        0, this->layers(),
    };

    VkImageView view = VK_NULL_HANDLE;
    if (vkCreateImageView(m_device, &view_create_info, nullptr, &view) != VK_SUCCESS)
    {
        return {};
    }

    return TextureViewHandle(new VulkanTextureView(m_device, view));
}
