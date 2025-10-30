#pragma once
#ifndef BONSAI_RENDERER_VULKAN_HELPERS_HPP
#define BONSAI_RENDERER_VULKAN_HELPERS_HPP

namespace rhi::vk
{
    /// @brief Extend the pNext chain for a Vulkan struct, keeping the old pNext value intact.
    /// @param target Target struct to extend.
    /// @param extension Struct to append.
    template<typename T, typename U>
    constexpr void extend_pnext_chain(T& target, U& extension)
    {
        void* temp = target.pNext;
        target.pNext = &extension;
        extension.pNext = temp;
    }
} //namespace rhi::vk

#endif //BONSAI_RENDERER_VULKAN_HELPERS_HPP