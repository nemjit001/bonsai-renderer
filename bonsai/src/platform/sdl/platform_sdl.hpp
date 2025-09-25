#pragma once
#ifndef BONSAI_RENDERER_PLATFORM_SDL_HPP
#define BONSAI_RENDERER_PLATFORM_SDL_HPP

#include <unordered_map>
#include <SDL3/SDL.h>
#include "platform/platform.hpp"

struct SurfaceImpl
{
    SDL_WindowID window_id;
    SDL_Window* window;
    void* user_data;
};

struct Platform::Impl
{
    void* user_data;
    std::unordered_map<SDL_WindowID, Surface*> surfaces;
    PFN_PlatformQuitCallback quit_callback;
    PFN_PlatformSurfaceResizeCallback surface_resize_callback;
    PFN_PlatformSurfaceClosedCallback surface_closed_callback;
    PFN_PlatformSurfaceKeyCallback surface_key_callback;
};

#endif //BONSAI_RENDERER_PLATFORM_SDL_HPP