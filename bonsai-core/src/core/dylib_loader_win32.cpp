#include "bonsai/core/dylib_loader.hpp"
#if _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct DylibHandle
{
    HMODULE library;
};

std::string bonsai_lib_name(std::string const& name)
{
    return name + ".dll";
}

DylibHandle* bonsai_load_library(std::string const& name)
{
    HMODULE library = ::LoadLibraryA(name.c_str());
    if (library == nullptr)
    {
        return nullptr;
    }

    return new DylibHandle{ library };
}

void bonsai_unload_library(DylibHandle const* handle)
{
    if (handle != nullptr)
    {
        ::FreeLibrary(handle->library);
        delete handle;
    }
}

void* bonsai_get_proc_address(DylibHandle const* handle, char const* name)
{
    if (handle == nullptr)
    {
        return nullptr;
    }

    return reinterpret_cast<void*>(::GetProcAddress(handle->library, name));
}

#endif //_WIN32