#pragma once
#ifndef BONSAI_RENDERER_PLATFORM_HPP
#define BONSAI_RENDERER_PLATFORM_HPP

#include <cstdint>

#if     BONSAI_USE_VULKAN
    #define VK_NO_PROTOTYPES
    #include <vulkan/vulkan.h>
#endif //BONSAI_USE_VULKAN

class PlatformSurface;
typedef void (*PFN_PlatformSurfaceResized)(PlatformSurface*, uint32_t, uint32_t);

/// @brief Platform surface configuration.
struct PlatformSurfaceConfig
{
    bool resizable;
    bool high_dpi;
};

/// @brief Opaque platform surface handle.
struct RawPlatformSurface;

/// @brief Platform surface type that represents an interactable region on the underlying platform.
class PlatformSurface
{
public:
    explicit PlatformSurface(RawPlatformSurface* raw_surface);
    ~PlatformSurface() = default;

    PlatformSurface(PlatformSurface const&) = default;
    PlatformSurface& operator=(PlatformSurface const&) = default;

    /// @brief Get the surface size in screen coordinates, often pixels, but high DPI surfaces should use
    /// PlatformSurface::get_size_in_pixels.
    /// @param width Output width.
    /// @param height Output height.
    void get_size(uint32_t& width, uint32_t& height) const;

    /// @brief Get the surface size in pixels. Useful for high DPI surfaces.
    /// @param width Output width.
    /// @param height Output height.
    void get_size_in_pixels(uint32_t& width, uint32_t& height) const;

    /// @brief Set the user data pointer for this surface.
    /// @param user_data New user data pointer.
    void set_user_data(void* user_data);

    /// @brief Get the user data pointer for this surface.
    /// @return The current user data pointer.
    void* get_user_data();

    /// @brief Get the raw platform surface handle.
    /// @return The raw platform surface handle.
    RawPlatformSurface const* get_raw() const { return m_raw_surface; }

#if     BONSAI_USE_VULKAN
    bool create_vulkan_surface(VkInstance instance, VkAllocationCallbacks const* allocator, VkSurfaceKHR* surface) const;
#endif //BONSAI_USE_VULKAN

private:
    RawPlatformSurface* m_raw_surface = nullptr;
};

/// @brief The Platform class wraps a concrete platform implementation.
class Platform
{
public:
    Platform();
    ~Platform();

    Platform(Platform const&) = delete;
    Platform& operator=(Platform const&) = delete;

    /// @brief Pump the platform message loop.
    /// @return True on continue, false on platform quit.
    [[nodiscard]]
    bool pump_messages();

    /// @brief Set or replace the surface resized callback.
    /// @param callback New callback.
    void set_surface_resized_callback(PFN_PlatformSurfaceResized callback);

    /// @brief Create a platform surface.
    /// @param title Surface display title.
    /// @param width Width of client area in pixels.
    /// @param height Height of client area in pixels.
    /// @param config Surface configuration.
    /// @return A platform surface handle.
    PlatformSurface* create_surface(char const* title, uint32_t width, uint32_t height, PlatformSurfaceConfig const& config);

    /// @brief Destroy a platform surface.
    /// @param surface Surface to destroy.
    void destroy_surface(PlatformSurface* surface);

#if     BONSAI_USE_VULKAN
    static char const** get_vulkan_instance_extensions(uint32_t* count);
#endif //BONSAI_USE_VULKAN

private:
    struct Impl;
    Impl* m_impl = nullptr;
};

#endif //BONSAI_RENDERER_PLATFORM_HPP