#pragma once
#ifndef BONSAI_RENDERER_PLATFORM_HPP
#define BONSAI_RENDERER_PLATFORM_HPP

#include <cstdint>

/// @brief Platform surface implementation.
struct Surface;

/// @brief Platform surface configuration for surface attributes.
struct SurfaceConfig
{
    bool resizable;
    bool allow_high_dpi;
};

/// @brief Platform singleton wrapper, exposes platform interface with platform-dependent implementation.
class Platform
{
public:
    static Platform& get();

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
    Surface* create_surface(char const* title, uint32_t width, uint32_t height, SurfaceConfig const& config);

    /// @brief Destroy a platform surface.
    /// @param surface Surface to destroy.
    void destroy_surface(Surface* surface);

private:
    Platform();

private:
    /// @brief Platform implementation.
    struct Impl;
    Impl* m_pImpl = nullptr;
};

#endif //BONSAI_RENDERER_PLATFORM_HPP