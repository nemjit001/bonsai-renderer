#pragma once
#ifndef BONSAI_RENDERER_PLATFORM_HPP
#define BONSAI_RENDERER_PLATFORM_HPP

#include <cstdint>

class Platform;

/// @brief Platform surface configuration for surface attributes.
struct SurfaceConfig
{
    bool resizable;
    bool allow_high_dpi;
};

/// @brief Opaque platform surface implementation.
struct SurfaceImpl;

/// @brief Platform surface, represents something that can be rendered to such as an application window.
class Surface
{
public:
    friend Platform;

    ~Surface();
    Surface(Surface const&) = delete;
    Surface& operator=(Surface const&) = delete;

    /// @brief Get the surface size in pixels.
    /// @param width Width in pixels
    /// @param height Height in pixels
    void get_size(uint32_t& width, uint32_t& height) const;

    /// @brief Set the user data pointer for this surface, this pointer will be passed to the platform surface callbacks.
    /// @param user_data User data pointer to set.
    void set_user_data(void* user_data);

    /// @brief Get the raw platform surface implementation. Opaque, unless you know what you're doing...
    [[nodiscard]] SurfaceImpl* raw_surface() const { return m_impl; }

private:
    SurfaceImpl* m_impl = nullptr;

    /// @brief Constructor, only accessible through platform.
    explicit Surface(SurfaceImpl* impl);
};

typedef void(*PFN_PlatformQuitCallback)(void*);
typedef void(*PFN_PlatformSurfaceResizeCallback)(void*, uint32_t, uint32_t);
typedef void(*PFN_PlatformSurfaceClosedCallback)(void*);
typedef void(*PFN_PlatformSurfaceKeyCallback)(void* user_data, int32_t, int32_t, bool);

/// @brief Platform manager, exposes platform interface with platform-dependent implementation.
class Platform
{
public:
    Platform();
    ~Platform();
    Platform(Platform const&) = delete;
    Platform& operator=(Platform const&) = delete;

    /// @brief Pump platform message loop.
    void pump_messages();

    /// @brief Create a platform surface.
    /// @param title Surface title.
    /// @param width Surface inner width in pixels.
    /// @param height Surface inner height in pixels.
    /// @param config Surface configuration for new surface.
    /// @return The newly created surface, or nullptr on error.
    Surface* create_surface(char const* title, uint32_t width, uint32_t height, SurfaceConfig const& config);

    /// @brief Destroy a platform surface.
    /// @param surface Surface to destroy.
    void destroy_surface(Surface* surface);

    /// @brief Set the user data pointer for the platform. This opaque pointer is used in platform callback functions.
    /// @param user_data User data pointer.
    void set_user_data(void* user_data);

    /// @brief Set the platform quit callback.
    /// @param callback Callback to set, may be nullptr.
    void set_platform_quit_callback(PFN_PlatformQuitCallback const& callback);

    /// @brief Set the platform surface resize callback.
    /// @param callback Callback to set, may be nullptr.
    void set_platform_surface_resize_callback(PFN_PlatformSurfaceResizeCallback const& callback);

    /// @brief Set the platform surface closed callback.
    /// @param callback Callback to set, may be nullptr.
    void set_platform_surface_closed_callback(PFN_PlatformSurfaceClosedCallback const& callback);

    /// @brief Set the paltform surface key input callback.
    /// @param callback Callback to set, may be nullptr.
    void set_platform_surface_key_callback(PFN_PlatformSurfaceKeyCallback const& callback);

private:
    /// @brief Platform implementation.
    struct Impl;
    Impl* m_impl = nullptr;
};

#endif //BONSAI_RENDERER_PLATFORM_HPP