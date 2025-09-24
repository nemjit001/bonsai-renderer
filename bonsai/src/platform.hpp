#pragma once
#ifndef BONSAI_RENDERER_PLATFORM_HPP
#define BONSAI_RENDERER_PLATFORM_HPP

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

private:
    Platform();

private:
    struct Impl;
    Impl* m_pImpl = nullptr;
};

#endif //BONSAI_RENDERER_PLATFORM_HPP