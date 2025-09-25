#pragma once
#ifndef BONSAI_RENDERER_PLATFORM_VULKAN_HPP
#define BONSAI_RENDERER_PLATFORM_VULKAN_HPP

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.hpp>
#include "platform.hpp"

char const** platform_enumerate_vulkan_instance_extensions(uint32_t* out_count);

bool platform_create_vulkan_surface(Surface* platform_surface, VkInstance instance, VkAllocationCallbacks const* allocator, VkSurfaceKHR* out_surface);

#endif //BONSAI_RENDERER_PLATFORM_VULKAN_HPP