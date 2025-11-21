#pragma once
#ifndef BONSAI_RENDERER_DYLIB_LOADER_HPP
#define BONSAI_RENDERER_DYLIB_LOADER_HPP

#include <string>

/// @brief Dynamic library handle as loaded by the OS specific dylib loader.
struct DylibHandle;

/// @brief Generate a platform specific library name from a base name string.
/// @param name Base library name.
/// @return The platform specific library name.
std::string bonsai_lib_name(std::string const& name);

/// @brief Load a dynamic library.
/// @param name Library name generated using `bonsai_lib_name`.
/// @return A new DylibHandle object, nullptr on failure.
DylibHandle* bonsai_load_library(std::string const& name);

/// @brief Unload a dynamic library.
/// @param handle Library handle to unload.
void bonsai_unload_library(DylibHandle const* handle);

/// @brief Get a library symbol by name.
/// @param handle Dynamic library handle.
/// @param name Symbol name.
/// @return An opaque pointer to the loaded symbol.
void* bonsai_get_proc_address(DylibHandle const* handle, char const* name);

#endif //BONSAI_RENDERER_DYLIB_LOADER_HPP