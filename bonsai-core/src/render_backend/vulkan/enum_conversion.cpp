#include "enum_conversion.hpp"

VkFormat get_vulkan_format(RenderFormat format)
{
    switch (format)
    {
    case RenderFormatUndefined:
        return VK_FORMAT_UNDEFINED;
    case RenderFormatR8_UNORM:
        return VK_FORMAT_R8_UNORM;
    case RenderFormatR8_SNORM:
        return VK_FORMAT_R8_SNORM;
    case RenderFormatR8_UINT:
        return VK_FORMAT_R8_UINT;
    case RenderFormatR8_SINT:
        return VK_FORMAT_R8_SINT;
    case RenderFormatRG8_UNORM:
        return VK_FORMAT_R8G8_UNORM;
    case RenderFormatRG8_SNORM:
        return VK_FORMAT_R8G8_SNORM;
    case RenderFormatRG8_UINT:
        return VK_FORMAT_R8G8_UINT;
    case RenderFormatRG8_SINT:
        return VK_FORMAT_R8G8_SINT;
    case RenderFormatRGBA8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case RenderFormatRGBA8_SNORM:
        return VK_FORMAT_R8G8B8A8_SNORM;
    case RenderFormatRGBA8_UINT:
        return VK_FORMAT_R8G8B8A8_UINT;
    case RenderFormatRGBA8_SINT:
        return VK_FORMAT_R8G8B8A8_SINT;
    case RenderFormatRGBA8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case RenderFormatBGRA8_UNORM:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case RenderFormatBGRA8_SNORM:
        return VK_FORMAT_B8G8R8A8_SNORM;
    case RenderFormatBGRA8_UINT:
        return VK_FORMAT_B8G8R8A8_UINT;
    case RenderFormatBGRA8_SINT:
        return VK_FORMAT_B8G8R8A8_SINT;
    case RenderFormatBGRA8_SRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case RenderFormatR16_SFLOAT:
        return VK_FORMAT_R16_SFLOAT;
    case RenderFormatR16_UNORM:
        return VK_FORMAT_R16_UNORM;
    case RenderFormatR16_SNORM:
        return VK_FORMAT_R16_SNORM;
    case RenderFormatR16_UINT:
        return VK_FORMAT_R16_UINT;
    case RenderFormatR16_SINT:
        return VK_FORMAT_R16_SINT;
    case RenderFormatRG16_SFLOAT:
        return VK_FORMAT_R16G16_SFLOAT;
    case RenderFormatRG16_UNORM:
        return VK_FORMAT_R16G16_UNORM;
    case RenderFormatRG16_SNORM:
        return VK_FORMAT_R16G16_SNORM;
    case RenderFormatRG16_UINT:
        return VK_FORMAT_R16G16_UINT;
    case RenderFormatRG16_SINT:
        return VK_FORMAT_R16G16_SINT;
    case RenderFormatRGBA16_SFLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case RenderFormatRGBA16_UNORM:
        return VK_FORMAT_R16G16B16A16_UNORM;
    case RenderFormatRGBA16_SNORM:
        return VK_FORMAT_R16G16B16A16_SNORM;
    case RenderFormatRGBA16_UINT:
        return VK_FORMAT_R16G16B16A16_UINT;
    case RenderFormatRGBA16_SINT:
        return VK_FORMAT_R16G16B16A16_SINT;
    case RenderFormatR32_SFLOAT:
        return VK_FORMAT_R32_SFLOAT;
    case RenderFormatR32_UINT:
        return VK_FORMAT_R32_UINT;
    case RenderFormatR32_SINT:
        return VK_FORMAT_R32_SINT;
    case RenderFormatRG32_SFLOAT:
        return VK_FORMAT_R32G32_SFLOAT;
    case RenderFormatRG32_UINT:
        return VK_FORMAT_R32G32_UINT;
    case RenderFormatRG32_SINT:
        return VK_FORMAT_R32G32_SINT;
    case RenderFormatRGB32_SFLOAT:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case RenderFormatRGB32_UINT:
        return VK_FORMAT_R32G32B32_UINT;
    case RenderFormatRGB32_SINT:
        return VK_FORMAT_R32G32B32_SINT;
    case RenderFormatRGBA32_SFLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case RenderFormatRGBA32_UINT:
        return VK_FORMAT_R32G32B32A32_UINT;
    case RenderFormatRGBA32_SINT:
        return VK_FORMAT_R32G32B32A32_SINT;
    case RenderFormatD16_UNORM:
        return VK_FORMAT_D16_UNORM;
    case RenderFormatD24_UNORM_S8_UINT:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case RenderFormatD32_SFLOAT:
        return VK_FORMAT_D32_SFLOAT;
    case RenderFormatD32_SFLOAT_S8_UINT:
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    default:
        break;
    }

    return VK_FORMAT_UNDEFINED;
}

VkImageAspectFlags get_vulkan_aspect_flags(RenderFormat format)
{
    switch (format)
    {
    case RenderFormatUndefined:
        return VK_IMAGE_ASPECT_NONE;
    case RenderFormatR8_UNORM:
    case RenderFormatR8_SNORM:
    case RenderFormatR8_UINT:
    case RenderFormatR8_SINT:
    case RenderFormatRG8_UNORM:
    case RenderFormatRG8_SNORM:
    case RenderFormatRG8_UINT:
    case RenderFormatRG8_SINT:
    case RenderFormatRGBA8_UNORM:
    case RenderFormatRGBA8_SNORM:
    case RenderFormatRGBA8_UINT:
    case RenderFormatRGBA8_SINT:
    case RenderFormatRGBA8_SRGB:
    case RenderFormatBGRA8_UNORM:
    case RenderFormatBGRA8_SNORM:
    case RenderFormatBGRA8_UINT:
    case RenderFormatBGRA8_SINT:
    case RenderFormatBGRA8_SRGB:
    case RenderFormatR16_SFLOAT:
    case RenderFormatR16_UNORM:
    case RenderFormatR16_SNORM:
    case RenderFormatR16_UINT:
    case RenderFormatR16_SINT:
    case RenderFormatRG16_SFLOAT:
    case RenderFormatRG16_UNORM:
    case RenderFormatRG16_SNORM:
    case RenderFormatRG16_UINT:
    case RenderFormatRG16_SINT:
    case RenderFormatRGBA16_SFLOAT:
    case RenderFormatRGBA16_UNORM:
    case RenderFormatRGBA16_SNORM:
    case RenderFormatRGBA16_UINT:
    case RenderFormatRGBA16_SINT:
    case RenderFormatR32_SFLOAT:
    case RenderFormatR32_UINT:
    case RenderFormatR32_SINT:
    case RenderFormatRG32_SFLOAT:
    case RenderFormatRG32_UINT:
    case RenderFormatRG32_SINT:
    case RenderFormatRGB32_SFLOAT:
    case RenderFormatRGB32_UINT:
    case RenderFormatRGB32_SINT:
    case RenderFormatRGBA32_SFLOAT:
    case RenderFormatRGBA32_UINT:
    case RenderFormatRGBA32_SINT:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    case RenderFormatD16_UNORM:
    case RenderFormatD32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case RenderFormatD24_UNORM_S8_UINT:
    case RenderFormatD32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
        break;
    }

    return VK_IMAGE_ASPECT_NONE;
}

VkImageType get_vulkan_image_type(RenderTextureType texture_type)
{
    switch (texture_type)
    {
    case RenderTextureType1D:
        return VK_IMAGE_TYPE_1D;
    case RenderTextureType2D:
        return VK_IMAGE_TYPE_2D;
    case RenderTextureType3D:
        return VK_IMAGE_TYPE_3D;
    default:
        break;
    }

    return VK_IMAGE_TYPE_MAX_ENUM;
}

VkImageTiling get_vulkan_image_tiling(RenderTextureTilingMode tiling_mode)
{
    switch (tiling_mode)
    {
    case RenderTextureTilingLinear:
        return VK_IMAGE_TILING_LINEAR;
    case RenderTextureTilingOptimal:
        return VK_IMAGE_TILING_OPTIMAL;
    default:
        break;
    }

    return VK_IMAGE_TILING_MAX_ENUM;
}

VkPrimitiveTopology get_vulkan_topology(PrimitiveTopologyType primitive_topology)
{
    switch (primitive_topology)
    {
    case PrimitiveTopologyTypePointList:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case PrimitiveTopologyTypeLineList:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case PrimitiveTopologyTypeLineStrip:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case PrimitiveTopologyTypeTriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case PrimitiveTopologyTypeTriangleStrip:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case PrimitiveTopologyTypeTriangleFan:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    case PrimitiveTopologyTypeLineListWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case PrimitiveTopologyTypeLineStripWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case PrimitiveTopologyTypeTriangleListWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case PrimitiveTopologyTypeTriangleStripWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    case PrimitiveTopologyTypePatchList:
        return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    default:
        break;
    }

    return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

VkPolygonMode get_vulkan_polygon_mode(PolygonMode polygon_mode)
{
    switch (polygon_mode)
    {
    case PolygonModeFill:
        return VK_POLYGON_MODE_FILL;
    case PolygonModeLine:
        return VK_POLYGON_MODE_LINE;
    case PolygonModePoint:
        return VK_POLYGON_MODE_POINT;
    default:
        break;
    }

    return VK_POLYGON_MODE_MAX_ENUM;
}

VkCullModeFlags get_vulkan_cull_mode(CullMode cull_mode)
{
    switch (cull_mode)
    {
    case CullModeNone:
        return VK_CULL_MODE_NONE;
    case CullModeBack:
        return VK_CULL_MODE_BACK_BIT;
    case CullModeFront:
        return VK_CULL_MODE_FRONT_BIT;
    default:
        break;
    }

    return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
}

VkCompareOp get_vulkan_compare_op(CompareOp compare_op)
{
    switch (compare_op)
    {
    case CompareOpNever:
        return VK_COMPARE_OP_NEVER;
    case CompareOpLess:
        return VK_COMPARE_OP_LESS;
    case CompareOpEqual:
        return VK_COMPARE_OP_EQUAL;
    case CompareOpLessOrEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOpGreater:
        return VK_COMPARE_OP_GREATER;
    case CompareOpNotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOpGreaterOrEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareOpAlways:
        return VK_COMPARE_OP_ALWAYS;
    default:
        break;
    }

    return VK_COMPARE_OP_MAX_ENUM;
}

VkStencilOp get_vulkan_stencil_op(StencilOp stencil_op)
{
    switch (stencil_op)
    {
    case StencilOpKeep:
        return VK_STENCIL_OP_KEEP;
    case StencilOpZero:
        return VK_STENCIL_OP_ZERO;
    case StencilOpReplace:
        return VK_STENCIL_OP_REPLACE;
    case StencilOpIncrementSaturate:
        return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case StencilOpDecrementSaturate:
        return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case StencilOpInvert:
        return VK_STENCIL_OP_INVERT;
    case StencilOpIncrementWrap:
        return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case StencilOpDecrementWrap:
        return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    default:
        break;
    }

    return VK_STENCIL_OP_MAX_ENUM;
}

VkLogicOp get_vulkan_logic_op(LogicOp logic_op)
{
    switch (logic_op)
    {
    case LogicOpClear:
        return VK_LOGIC_OP_CLEAR;
    case LogicOpAND:
        return VK_LOGIC_OP_AND;
    case LogicOpANDReverse:
        return VK_LOGIC_OP_AND_REVERSE;
    case LogicOpCopy:
        return VK_LOGIC_OP_COPY;
    case LogicOpANDInverted:
        return VK_LOGIC_OP_AND_INVERTED;
    case LogicOpNoOp:
        return VK_LOGIC_OP_NO_OP;
    case LogixOpXOR:
        return VK_LOGIC_OP_XOR;
    case LogicOpOR:
        return VK_LOGIC_OP_OR;
    case LogicOpNOR:
        return VK_LOGIC_OP_NOR;
    case LogicOpEquivalent:
        return VK_LOGIC_OP_EQUIVALENT;
    case LogicOpInvert:
        return VK_LOGIC_OP_INVERT;
    case LogicOpORReverse:
        return VK_LOGIC_OP_OR_REVERSE;
    case LogicOpCopyInverted:
        return VK_LOGIC_OP_COPY_INVERTED;
    case LogicOpORInverted:
        return VK_LOGIC_OP_OR_INVERTED;
    case LogicOpNAND:
        return VK_LOGIC_OP_NAND;
    case LogicOpSet:
        return VK_LOGIC_OP_SET;
    default:
        break;
    }

    return VK_LOGIC_OP_MAX_ENUM;
}

VkBlendFactor get_vulkan_blend_factor(BlendFactor blend_factor)
{
    switch (blend_factor)
    {
    case BlendFactorZero:
        return VK_BLEND_FACTOR_ZERO;
    case BlendFactorOne:
        return VK_BLEND_FACTOR_ONE;
    case BlendFactorSrcColor:
        return VK_BLEND_FACTOR_SRC_COLOR;
    case BlendFactorOneMinusSrcColor:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case BlendFactorDstColor:
        return VK_BLEND_FACTOR_DST_COLOR;
    case BlendFactorOneMinusDstColor:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case BlendFactorSrcAlpha:
        return VK_BLEND_FACTOR_SRC_ALPHA;
    case BlendFactorOneMinusSrcAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case BlendFactorDstAlpha:
        return VK_BLEND_FACTOR_DST_ALPHA;
    case BlendFactorOneMinusDstAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case BlendFactorConstantColor:
        return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case BlendFactorOneMinusConstantColor:
        return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case BlendFactorConstantAlpha:
        return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case BlendFactorOneMinusConstantAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case BlendFactorSrcAlphaSaturate:
        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case BlendFactorSrc1Color:
        return VK_BLEND_FACTOR_SRC1_COLOR;
    case BlendFactorOneMinusSrc1Color:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case BlendFactorSrc1Alpha:
        return VK_BLEND_FACTOR_SRC1_ALPHA;
    case BlendFactorOneMinusSrc1Alpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    default:
        break;
    }

    return VK_BLEND_FACTOR_MAX_ENUM;
}

VkBlendOp get_vulkan_blend_op(BlendOp blend_op)
{
    switch (blend_op)
    {
    case BlendOpAdd:
        return VK_BLEND_OP_ADD;
    case BlendOpSubtract:
        return VK_BLEND_OP_SUBTRACT;
    case BlendOpReverseSubtract:
        return VK_BLEND_OP_REVERSE_SUBTRACT;
    case BlendOpMin:
        return VK_BLEND_OP_MIN;
    case BlendOpMax:
        return VK_BLEND_OP_MAX;
    default:
        break;
    }

    return VK_BLEND_OP_MAX_ENUM;
}

VkAttachmentLoadOp get_vulkan_load_op(RenderLoadOp load_op)
{
    switch (load_op)
    {
    case RenderLoadOpLoad:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case RenderLoadOpClear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case RenderLoadOpDontCare:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    default:
        break;
    }

    return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
}

VkAttachmentStoreOp get_vulkan_store_op(RenderStoreOp store_op)
{
    switch (store_op)
    {
    case RenderStoreOpStore:
        return VK_ATTACHMENT_STORE_OP_STORE;
    case RenderStoreOpDontCare:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    default:
        break;
    }

    return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
}

VkIndexType get_vulkan_index_type(IndexType index_type)
{
    switch (index_type)
    {
    case IndexTypeUint16:
        return VK_INDEX_TYPE_UINT16;
    case IndexTypeUint32:
        return VK_INDEX_TYPE_UINT32;
    default:
        break;
    }

    return VK_INDEX_TYPE_MAX_ENUM;
}
