#include "manifest_util.h"
#include <string>
#include <filesystem>

#ifdef _WIN32
#pragma push_macro("interface")
#undef interface
#pragma push_macro("GetObject")
#undef GetObject

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <Windows.h>
#elif defined __unix__
#else
#error OS is not supported!
#endif

sdv::u8string CManifestUtil::ReadModuleManifest(/*in*/ const sdv::u8string& ssModule)
{
    sdv::u8string ssManifest;
    std::filesystem::path pathModule(static_cast<std::string>(ssModule));
    if (!std::filesystem::exists(pathModule)) return ssManifest;

    // Load the module
#ifdef _WIN32
    sdv::core::TModuleID tModuleID = reinterpret_cast<sdv::core::TModuleID>(LoadLibrary(pathModule.native().c_str()));
#elif defined __unix__
    sdv::core::TModuleID tModuleID = reinterpret_cast<sdv::core::TModuleID>(dlopen(pathModule.native().c_str(), RTLD_LAZY));
#else
#error OS not supported!
#endif
    if (!tModuleID) return ssManifest;

    // Check whether the module exposes the necessary functions
    using TFNHasActiveObjects = bool();
    using TFNGetModuleFactory = sdv::IInterfaceAccess*(uint32_t);
    using TFNGetManifest = const char*();

#ifdef _WIN32
    std::function<TFNGetModuleFactory> fnGetFactory = reinterpret_cast<TFNGetModuleFactory*>(GetProcAddress(reinterpret_cast<HMODULE>(tModuleID), "GetModuleFactory"));
    std::function<TFNHasActiveObjects> fnActiveObjects = reinterpret_cast<TFNHasActiveObjects*>(GetProcAddress(reinterpret_cast<HMODULE>(tModuleID), "HasActiveObjects"));
    std::function<TFNGetManifest> fnGetManifest = reinterpret_cast<TFNGetManifest*>(GetProcAddress(reinterpret_cast<HMODULE>(tModuleID), "GetManifest"));
#elif defined __unix__
    std::function<TFNGetModuleFactory> fnGetFactory = reinterpret_cast<TFNGetModuleFactory*>(dlsym(reinterpret_cast<void*>(tModuleID), "GetModuleFactory"));
    std::function<TFNHasActiveObjects> fnActiveObjects = reinterpret_cast<TFNHasActiveObjects*>(dlsym(reinterpret_cast<void*>(tModuleID), "HasActiveObjects"));
    std::function<TFNGetManifest> fnGetManifest = reinterpret_cast<TFNGetManifest*>(dlsym(reinterpret_cast<void*>(tModuleID), "GetManifest"));
#else
#error OS not supported!
#endif

    // Check for functions and the correct version
    if (fnGetFactory && fnActiveObjects && fnGetManifest && fnGetManifest())
        ssManifest = fnGetManifest();

    // Release the library
#ifdef _WIN32
    FreeLibrary(reinterpret_cast<HMODULE>(tModuleID));
#elif defined __unix__
    dlclose(reinterpret_cast<void*>(tModuleID));
#else
#error OS not supported!
#endif

    // Return the manifest
    return ssManifest;
}
