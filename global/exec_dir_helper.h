/**
 *
 * @file      exec_dir_helper.h
 * @brief     This file contains helper functions e.g. filesystem
 * @version   0.1
 * @date      2022.11.14
 * @author    Thomas.pfleiderer@zf.com
 * @copyright Copyright ZF Friedrichshaven AG (c) 2022
 *
 */
#ifndef EXEC_DIR_HELPER_H
#define EXEC_DIR_HELPER_H

#include <filesystem>
#include <string>

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
#include <array>

// Resolve conflict
#pragma pop_macro("GetObject")
#pragma pop_macro("interface")
#ifdef GetClassInfo
#undef GetClassInfo
#endif
#elif defined __linux__
#include <linux/limits.h>
#include <unistd.h>
#else
#error OS is not supported
#endif

/**
 * @brief Get the absolute path to the currently run executable
 * @return Absolute std::filesystem::path to the currently run executable
 */
inline std::filesystem::path GetExecDirectory()
{
    static std::filesystem::path pathExeDir;
    if (!pathExeDir.empty()) return pathExeDir;
#ifdef _WIN32
    // Windows specific
    std::wstring ssPath(32768, '\0');
    GetModuleFileNameW(NULL, ssPath.data(), static_cast<DWORD>(ssPath.size() - 1));
#elif defined __linux__
    // Linux specific
    std::string ssPath(PATH_MAX + 1, '\0');
    const ssize_t nCount = readlink("/proc/self/exe", ssPath.data(), PATH_MAX);
    if (nCount < 0 || nCount >= PATH_MAX)
        return pathExeDir; // some error
    ssPath.at(nCount) = '\0';
#else
#error OS is not supported!
#endif
    pathExeDir = std::filesystem::path{ssPath.c_str()}.parent_path() / ""; // To finish the folder path with (back)slash
    return pathExeDir;
}

/**
 * @brief Get the filename of the currently run executable
 * @return The filename to the currently run executable
 */
inline std::filesystem::path GetExecFilename()
{
    static std::filesystem::path pathExeFilename;
    if (!pathExeFilename.empty()) return pathExeFilename;
#ifdef _WIN32
    // Windows specific
    std::wstring ssPath(32768, '\0');
    GetModuleFileNameW(NULL, ssPath.data(), static_cast<DWORD>(ssPath.size() - 1));
#elif defined __linux__
    // Linux specific
    std::string ssPath(PATH_MAX + 1, '\0');
    const ssize_t nCount = readlink("/proc/self/exe", ssPath.data(), PATH_MAX);
    if (nCount < 0 || nCount >= PATH_MAX)
        return pathExeFilename; // some error
    ssPath.at(nCount) = '\0';
#else
#error OS is not supported!
#endif
    pathExeFilename = std::filesystem::path{ssPath.c_str()}.filename();
    return pathExeFilename;
}

#endif // !defined EXEC_DIR_HELPER_H
