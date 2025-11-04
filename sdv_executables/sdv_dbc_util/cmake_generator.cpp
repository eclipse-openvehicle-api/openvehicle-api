#include "cmake_generator.h"
#include <sstream>
#include <fstream>
#include <set>
#include <algorithm>
#include <thread>
#include <chrono>

CDbcUtilCMakeGenerator::CDbcUtilCMakeGenerator(const std::filesystem::path& rpathOutputDir, const std::string& rssComponent)
{
    if (rssComponent.empty())  return;      // TODO: Exception

    // Create project directory
    if (!rpathOutputDir.empty())
        m_pathProject = rpathOutputDir;
    if (!rssComponent.empty())
        m_pathProject /= "can_dl";
    std::filesystem::create_directories(m_pathProject);

    // The source string
    std::string ssSource;

    // File with "CMakeLists.txt" function; read completely if existing
    std::filesystem::path pathFile = m_pathProject / "CMakeLists.txt";
    if (std::filesystem::exists(pathFile))
    {
        std::ifstream stream;
        stream.open(pathFile);
        if (!stream.is_open()) return;      // TODO: Exception: throw CCompileException("Failed to open the CMakeLists.txt file for reading.");

        // Read the complete source
        std::stringstream sstream;
        sstream << stream.rdbuf();
        ssSource = std::move(sstream.str());
    }
    else // Create the file in memory
    {
        ssSource = R"code(# Enforce CMake version 3.20 or newer needed for path function
cmake_minimum_required (VERSION 3.20)

# Use new policy for project version settings and default warning level
cmake_policy(SET CMP0048 NEW)   # requires CMake 3.14
cmake_policy(SET CMP0092 NEW)   # requires CMake 3.15

# Define project
project(%component% VERSION 1.0 LANGUAGES CXX)

# Use C++17 support
set(CMAKE_CXX_STANDARD 17)

# Library symbols are hidden by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

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
add_library(%component% SHARED)

# Set extension to .sdv
set_target_properties(%component% PROPERTIES PREFIX "")
set_target_properties(%component% PROPERTIES SUFFIX ".sdv")
)code";
        CKeywordMap mapKeywords;
        mapKeywords["component"] = rssComponent;
        ssSource = ReplaceKeywords(ssSource, mapKeywords);
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
    if (nPos == std::string::npos) return;      // TODO: Exception: throw CCompileException("Missing 'add_library' keyword.");

    // Search for shared keyword
    nPos = fnNCFind("shared", nPos);
    if (nPos == std::string::npos) return;      // TODO: Exception: throw CCompileException("Missing 'shared' keyword.");
    nPos += 6;

    // Build set with files
    size_t nStop = fnNCFind(")", nPos);
    if (nStop == std::string::npos) return;      // TODO: Exception: throw CCompileException("Missing ')' closing the 'add_library' statement.");
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
    if (setFiles.find("datalink.h") == setFiles.end())
        ssSource.insert(nStop, std::string("\n    datalink.h"));
    if (setFiles.find("datalink.cpp") == setFiles.end())
        ssSource.insert(nStop, std::string("\n    datalink.cpp"));

    // Write the file again if needed
    if (nSourceSize != ssSource.size())
    {
        std::ofstream stream;
        stream.open(pathFile, std::ofstream::trunc);
        if (!stream.is_open()) return;      // TODO: Exception: throw CCompileException("Failed to open the CMakeLists.txt file for writing.");

        // Write the complete source
        stream << ssSource;
    }
}

CDbcUtilCMakeGenerator::~CDbcUtilCMakeGenerator()
{}

