if (NOT DXC_FOUND)
    set(DXC_FOUND TRUE)

    add_library(directx-compiler INTERFACE)
    target_link_libraries(directx-compiler INTERFACE dxcompiler)
endif()