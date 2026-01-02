#pragma once
#ifndef BONSAI_RENDERER_ENUM_CONVERSION_HPP
#define BONSAI_RENDERER_ENUM_CONVERSION_HPP

#include <volk.h>
#include <bonsai/render_backend/render_backend.hpp>

VkFormat get_vulkan_format(RenderFormat format);

VkImageAspectFlags get_vulkan_aspect_flags(RenderFormat format);

VkImageType get_vulkan_image_type(RenderTextureType texture_type);

VkImageTiling get_vulkan_image_tiling(RenderTextureTilingMode tiling_mode);

VkPrimitiveTopology get_vulkan_topology(PrimitiveTopologyType primitive_topology);

VkPolygonMode get_vulkan_polygon_mode(PolygonMode polygon_mode);

VkCullModeFlags get_vulkan_cull_mode(CullMode cull_mode);

VkCompareOp get_vulkan_compare_op(CompareOp compare_op);

VkStencilOp get_vulkan_stencil_op(StencilOp stencil_op);

VkLogicOp get_vulkan_logic_op(LogicOp logic_op);

VkBlendFactor get_vulkan_blend_factor(BlendFactor blend_factor);

VkBlendOp get_vulkan_blend_op(BlendOp blend_op);

VkAttachmentLoadOp get_vulkan_load_op(RenderLoadOp load_op);

VkAttachmentStoreOp get_vulkan_store_op(RenderStoreOp store_op);

VkIndexType get_vulkan_index_type(IndexType index_type);

#endif //BONSAI_RENDERER_ENUM_CONVERSION_HPP