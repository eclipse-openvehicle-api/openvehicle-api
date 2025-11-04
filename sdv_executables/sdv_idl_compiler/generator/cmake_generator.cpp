#include "context.h"
#include "cmake_generator.h"
#include "../exception.h"
#include <cassert>
#include <cctype>
#include <fstream>
#include <algorithm>
#include <set>
#include <mutex>

CIdlCompilerCMakeGenerator::CIdlCompilerCMakeGenerator(sdv::IInterfaceAccess* pParser) : CGenContext(pParser), m_mtx("SDV_IDL_COMPILER_GENERATE_CMAKE")
{}

CIdlCompilerCMakeGenerator::~CIdlCompilerCMakeGenerator()
{}

bool CIdlCompilerCMakeGenerator::Generate(const std::string& ssTargetLibName)
{
    // Synchronize CMake code generation among processes.
    std::unique_lock<ipc::named_mutex> lock(m_mtx);

    if (ssTargetLibName.empty())
        throw CCompileException("No target library name defined for proxy/stub CMake file.");

    // Create "proxy" directory
    std::filesystem::path pathPSTarget = GetOutputDir() / "ps";
    if (!std::filesystem::exists(pathPSTarget) && !std::filesystem::create_directory(pathPSTarget))
        throw CCompileException("Cannot create proxy/stub directory: ", pathPSTarget.generic_u8string());

    // The source string
    std::string ssSource;

    // File with "CMakeLists.txt" function; read completely if existing
    std::filesystem::path pathFile = pathPSTarget / "CMakeLists.txt";

    if (g_log_control.GetVerbosityMode() == EVerbosityMode::report_all)
        std::cout << "Target file: " << pathFile.generic_u8string() << std::endl;

    // Create or update CMakeLists.txt
    if (std::filesystem::exists(pathFile))
    {
        std::ifstream stream;
        stream.open(pathFile);
        if (!stream.is_open()) throw CCompileException("Failed to open the CMakeLists.txt file for reading.");

        // Read the complete source
        std::stringstream sstream;
        sstream << stream.rdbuf();
        ssSource = std::move(sstream.str());
    }
    else // Create the file in memory
    {
        CKeywordMap mapKeywords;
        mapKeywords.insert(std::make_pair("target_lib_name", ssTargetLibName));
        ssSource = ReplaceKeywords(R"code(# Enforce CMake version 3.20 or newer needed for path function
cmake_minimum_required (VERSION 3.20)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

# Define project
project(%target_lib_name% VERSION 1.0 LANGUAGES CXX)

# Use C++17 support
set(CMAKE_CXX_STANDARD 17)

# Library symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Set target name.
set(TARGET_NAME %target_lib_name%)

# Set the SDV_FRAMEWORK_DEV_INCLUDE if not defined yet
if (NOT DEFINED SDV_FRAMEWORK_DEV_INCLUDE)
    if (NOT DEFINED ENV{SDV_FRAMEWORK_DEV_INCLUDE})
        message( FATAL_ERROR "The environment variable SDV_FRAMEWORK_DEV_INCLUDE needs to be pointing to the SDV V-API development include files location!")
    endif()
    set (SDV_FRAMEWORK_DEV_INCLUDE "$ENV{SDV_FRAMEWORK_DEV_INCLUDE}")
endif()

# Include link to export directory of SDV V-API development include files location
include_directories(${SDV_FRAMEWORK_DEV_INCLUDE})

# Set platform specific compile flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX /wd4996 /wd4100 /permissive- /Zc:rvalueCast)
else()
    add_compile_options(-Werror -Wall -Wextra -Wshadow -Wpedantic -Wunreachable-code -fno-common)
endif()

# Add the dynamic library
add_library(${TARGET_NAME} SHARED)

# Set extension to .sdv
set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")
set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".sdv")

# TODO: set target name.
#add_dependencies(${TARGET_NAME} <add_cmake_target_this_depends_on>)
)code", mapKeywords);
    }

    // Search function for caseless finding in the string.
    auto fnNCFind = [&](const std::string& rssText, size_t nPos = 0) -> size_t
    {
        auto it = std::search(ssSource.begin() + nPos, ssSource.end(), rssText.begin(), rssText.end(),
            [](unsigned char ch1, unsigned char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
        );
        if (it == ssSource.end()) return std::string::npos;
        return std::distance(ssSource.begin(), it);
    };

    // Find the add_library function
    size_t nPos = fnNCFind("add_library");
    if (nPos == std::string::npos) throw CCompileException("Missing 'add_library' keyword.");

    // Search for shared keyword
    nPos = fnNCFind("shared", nPos);
    if (nPos == std::string::npos) throw CCompileException("Missing 'shared' keyword.");
    nPos += 6;

    // Build set with files
    size_t nStop = fnNCFind(")", nPos);
    if (nStop == std::string::npos) throw CCompileException("Missing ')' closing the 'add_library' statement.");
    std::set<std::string> setFiles;
    while (nPos < nStop)
    {
        // Skip whitespace
        while (std::isspace(ssSource[nPos])) nPos++;

        // Read file name
        size_t nFileBegin = nPos;
        while (nPos < nStop && !std::isspace(ssSource[nPos])) nPos++;

        // Store the file
        setFiles.insert(ssSource.substr(nFileBegin, nPos - nFileBegin));
    }

    // Insert additional files if needed
    size_t nSourceSize = ssSource.size();
    std::filesystem::path pathPSFileBase = GetSource().filename();
    pathPSFileBase.replace_extension("");
    std::string ssFileBase = pathPSFileBase.generic_u8string();
    if (setFiles.find(ssFileBase + "_stub.cpp") == setFiles.end())
        ssSource.insert(nStop, std::string("\n    ") + ssFileBase + "_stub.cpp");
    if (setFiles.find(ssFileBase + "_stub.h") == setFiles.end())
        ssSource.insert(nStop, std::string("\n    ") + ssFileBase + "_stub.h");
    if (setFiles.find(ssFileBase + "_proxy.cpp") == setFiles.end())
        ssSource.insert(nStop, std::string("\n    ") + ssFileBase + "_proxy.cpp");
    if (setFiles.find(ssFileBase + "_proxy.h") == setFiles.end())
        ssSource.insert(nStop, std::string("\n    ") + ssFileBase + "_proxy.h");

    // Write the file again if needed
    if (nSourceSize != ssSource.size())
    {
        std::ofstream stream;
        stream.open(pathFile, std::ofstream::trunc);
        if (!stream.is_open()) throw CCompileException("Failed to open the CMakeLists.txt file for writing.");

        // Write the complete source
        stream << ssSource;
    }

    // Done!
    return true;
}
