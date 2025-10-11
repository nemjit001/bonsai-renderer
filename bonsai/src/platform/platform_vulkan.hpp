#pragma once
#ifndef BONSAI_RENDERER_PLATFORM_VULKAN_HPP
#define BONSAI_RENDERER_PLATFORM_VULKAN_HPP
#if BONSAI_USE_VULKAN

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.hpp>

class Surface;

/// @brief Enumerate the platform's required Vulkan instance extensions.
/// @param out_count Number of extensions in the returned extension names array.
/// @return An array containing the instance extension names.
char const** platform_enumerate_vulkan_instance_extensions(uint32_t* out_count);

/// @brief Create a Vulkan surface from a platform surface.
/// @param platform_surface Platform surface to use for Vulkan surface creation.
/// @param instance
/// @param allocator
/// @param out_surface Vulkan surface output variable.
/// @return A boolean indicating successful surface creation.
bool platform_create_vulkan_surface(Surface const* platform_surface, VkInstance instance, VkAllocationCallbacks const* allocator, VkSurfaceKHR* out_surface);

#endif //BONSAI_USE_VULKAN
#endif //BONSAI_RENDERER_PLATFORM_VULKAN_HPP