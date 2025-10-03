#pragma once
#ifndef BONSAI_RENDERER_PLATFORM_WIN32_HPP
#define BONSAI_RENDERER_PLATFORM_WIN32_HPP
#if _WIN32

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class Surface;

/// @brief Get the Win32 HWND handle from a platform surface.
/// @param surface Surface to get HWND for.
/// @return The HWND associated with this platform surface.
HWND platform_get_surface_hwnd(Surface* surface);

#endif //_WIN32
#endif //BONSAI_RENDERER_PLATFORM_WIN32_HPP