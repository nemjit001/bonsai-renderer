#include "../../include/bonsai/core/dylib_loader.hpp"
#if __unix__

#include <dlfcn.h>

struct DylibHandle
{
    void* library;
};

std::string bonsai_lib_name(std::string const& name)
{
    return "lib" + name + ".so";
}

DylibHandle* bonsai_load_library(std::string name)
{
    void* library = dlopen(name.c_str(), RTLD_NOW);
    if (library == nullptr)
    {
        return nullptr;
    }

    return new DylibHandle{ library };
}

void bonsai_unload_library(DylibHandle* handle)
{
    if (handle != nullptr)
    {
        dlclose(handle->library);
        delete handle;
    }
}

void* bonsai_get_proc_address(DylibHandle* handle, char const* name)
{
    if (handle == nullptr)
    {
        return nullptr;
    }

    return dlsym(handle->library, name);
}

#endif //__unix__