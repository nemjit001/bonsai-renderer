#pragma once
#ifndef BONSAI_RENDERER_BONSAI_API_HPP
#define BONSAI_RENDERER_BONSAI_API_HPP

#if _MSC_VER
#if BONSAI_EXPORT_SYMBOLS
    #define BONSAI_API      __declspec(dllexport)
#else
    #define BONSAI_API      __declspec(dllimport)
#endif
    #define BONSAI_APICALL  __stdcall
#elif __GNUC__
#if BONSAI_EXPORT_SYMBOLS
    #define BONSAI_API      __attribute__((visibility("default")))
#else
    #define BONSAI_API
#endif
    #define BONSAI_APICALL
#else
    #define BONSAI_API
    #define BONSAI_APICALL
#endif

#endif //BONSAI_RENDERER_BONSAI_API_HPP