#include "vulkan_command_buffer.hpp"

#include <vector>

#include "vulkan_texture.hpp"

VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer)
    :
    ICommandBuffer(),
    m_device(device),
    m_command_pool(command_pool),
    m_command_buffer(command_buffer)
{
    //
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);
}

VkAttachmentLoadOp VulkanCommandBuffer::get_vulkan_attachment_load_op(AttachmentLoadOp load_op)
{
    switch (load_op)
    {
    case AttachmentLoadOp::Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case AttachmentLoadOp::Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case AttachmentLoadOp::DontCare:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    default:
        break;
    }

    return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
}

VkAttachmentStoreOp VulkanCommandBuffer::get_vulkan_attachment_store_op(AttachmentStoreOp store_op)
{
    switch (store_op)
    {
    case AttachmentStoreOp::Store:
        return VK_ATTACHMENT_STORE_OP_STORE;
    case AttachmentStoreOp::DontCare:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    default:
        break;
    }

    return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
}

bool VulkanCommandBuffer::begin()
{
    if (vkResetCommandBuffer(m_command_buffer, 0 /* no  flags */) != VK_SUCCESS)
    {
        return false;
    }

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    return vkBeginCommandBuffer(m_command_buffer, &begin_info) == VK_SUCCESS;
}

bool VulkanCommandBuffer::close()
{
    return vkEndCommandBuffer(m_command_buffer) == VK_SUCCESS;
}

void VulkanCommandBuffer::resource_barrier(ResourceBarrier const& resource_barrier)
{
    std::vector<VkImageMemoryBarrier2> image_memory_barriers{};
    if (resource_barrier.type == ResourceBarrierType::Texture)
    {
        image_memory_barriers.reserve(resource_barrier.barrier_count);
        for (size_t i = 0; i < resource_barrier.barrier_count; i++) {
            TextureResourceBarrier const& texture_barrier = resource_barrier.texture_barriers[i];
            TextureHandle const texture = texture_barrier.texture;

            // FIXME(nemjit001): This is suboptimal, using stage & access masks depending on layout types will probably improve transition performance
            VkImageMemoryBarrier2 image_barrier{};
            image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            image_barrier.pNext = nullptr;
            image_barrier.srcStageMask = 0;
            image_barrier.dstStageMask = 0;
            image_barrier.srcAccessMask = 0;
            image_barrier.dstAccessMask = 0;
            image_barrier.oldLayout = VulkanTexture::get_vulkan_image_layout(texture_barrier.old_layout);
            image_barrier.newLayout = VulkanTexture::get_vulkan_image_layout(texture_barrier.new_layout);
            image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            image_barrier.image = texture_barrier.texture->get_object<VkImage>();
            image_barrier.subresourceRange = VkImageSubresourceRange{
                VulkanTextureView::get_vulkan_aspect_flags(texture->fomat()),
                0, texture->mip_levels(),
                0, texture->layers(),
            };

            image_memory_barriers.push_back(image_barrier);
        }
    }

    VkDependencyInfo dependency_info{};
    dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependency_info.pNext = nullptr;
    dependency_info.dependencyFlags = 0;
    dependency_info.imageMemoryBarrierCount = static_cast<uint32_t>(image_memory_barriers.size());
    dependency_info.pImageMemoryBarriers = image_memory_barriers.data();

    vkCmdPipelineBarrier2(m_command_buffer, &dependency_info);
}

void VulkanCommandBuffer::begin_render_pass(RenderPassDesc const& desc)
{
    std::vector<VkRenderingAttachmentInfo> color_attachments{};
    color_attachments.reserve(8); // DX12 allows max 8 color attachments, keep similar worst-case bound here
    for (size_t i = 0; i < desc.color_attachment_count; i++)
    {
        RenderAttachmentDesc const& attachment = desc.color_attachments[i];
        VkRenderingAttachmentInfo rendering_attachment{};
        rendering_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        rendering_attachment.pNext = nullptr;
        rendering_attachment.imageView = attachment.view->get_object<VkImageView>();
        rendering_attachment.imageLayout = VulkanTexture::get_vulkan_image_layout(attachment.layout);
        rendering_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
        rendering_attachment.resolveImageView = VK_NULL_HANDLE;
        rendering_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        rendering_attachment.loadOp = get_vulkan_attachment_load_op(attachment.load_op);
        rendering_attachment.storeOp = get_vulkan_attachment_store_op(attachment.store_op);
        rendering_attachment.clearValue.color = {
            {
                attachment.clear_value.color.float32[0],
                attachment.clear_value.color.float32[1],
                attachment.clear_value.color.float32[2],
                attachment.clear_value.color.float32[3],
            }
        };

        color_attachments.push_back(rendering_attachment);
    }

    bool has_depth_attachment = false;
    VkRenderingAttachmentInfo depth_attachment{};
    if (desc.depth_attachment != nullptr)
    {
        has_depth_attachment = true;
        depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depth_attachment.pNext = nullptr;
        depth_attachment.imageView = desc.depth_attachment->view->get_object<VkImageView>();
        depth_attachment.imageLayout = VulkanTexture::get_vulkan_image_layout(desc.depth_attachment->layout);
        depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
        depth_attachment.resolveImageView = VK_NULL_HANDLE;
        depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.loadOp = get_vulkan_attachment_load_op(desc.depth_attachment->load_op);
        depth_attachment.storeOp = get_vulkan_attachment_store_op(desc.depth_attachment->store_op);
        depth_attachment.clearValue.depthStencil = {
            desc.depth_attachment->clear_value.depth_stencil.depth,
            desc.depth_attachment->clear_value.depth_stencil.stencil,
        };
    }

    bool has_stencil_attachment = false;
    VkRenderingAttachmentInfo stencil_attachment{};
    if (desc.stencil_attachment != nullptr)
    {
        has_stencil_attachment = true;
        stencil_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        stencil_attachment.pNext = nullptr;
        stencil_attachment.imageView = desc.stencil_attachment->view->get_object<VkImageView>();
        stencil_attachment.imageLayout = VulkanTexture::get_vulkan_image_layout(desc.stencil_attachment->layout);
        stencil_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
        stencil_attachment.resolveImageView = VK_NULL_HANDLE;
        stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        stencil_attachment.loadOp = get_vulkan_attachment_load_op(desc.stencil_attachment->load_op);
        stencil_attachment.storeOp = get_vulkan_attachment_store_op(desc.stencil_attachment->store_op);
        stencil_attachment.clearValue.depthStencil = {
            desc.stencil_attachment->clear_value.depth_stencil.depth,
            desc.stencil_attachment->clear_value.depth_stencil.stencil,
        };
    }

    VkRenderingInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.pNext = nullptr;
    rendering_info.flags = 0;
    rendering_info.renderArea.offset = VkOffset2D{ desc.render_area.offset.x, desc.render_area.offset.y };
    rendering_info.renderArea.extent = VkExtent2D{ desc.render_area.extent.width, desc.render_area.extent.height };
    rendering_info.layerCount = 1;
    rendering_info.viewMask = 0;
    rendering_info.colorAttachmentCount = static_cast<uint32_t>(color_attachments.size());
    rendering_info.pColorAttachments = color_attachments.data();
    rendering_info.pDepthAttachment = has_depth_attachment ? &depth_attachment : nullptr;
    rendering_info.pStencilAttachment = has_stencil_attachment ? &stencil_attachment : nullptr;

    vkCmdBeginRendering(m_command_buffer, &rendering_info);
}

void VulkanCommandBuffer::end_render_pass()
{
    vkCmdEndRendering(m_command_buffer);
}
