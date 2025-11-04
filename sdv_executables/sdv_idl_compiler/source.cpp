#ifdef _MSC_VER
#pragma push_macro("interface")
#undef interface
#define NOMINMAX
#include <Windows.h>
#ifdef GetClassInfo
    #undef GetClassInfo
#endif
#pragma pop_macro("interface")
#endif

#include "source.h"
#include "codepos.h"
#include "exception.h"
#include "token.h"
#include <fstream>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <cuchar>

CSource::CSource()
{}

CSource::CSource(const std::filesystem::path& rpath)
{
    ReadFile(rpath);
}

CSource::CSource(const char* szCode)
{
    if (!szCode) throw CCompileException("Cannot set code");

    m_ssSource = szCode;
    m_path = std::filesystem::current_path() / "unknown.idl";
}

CSource::CSource(CSource&& rSource) noexcept: m_path(std::move(rSource.m_path)), m_ssSource(std::move(rSource.m_ssSource))
{}

CSource& CSource::operator=(CSource&& rSource) noexcept
{
    m_path = std::move(rSource.m_path);
    m_ssSource = std::move(rSource.m_ssSource);
    return *this;
}

const std::filesystem::path& CSource::GetPathRef() const
{
    return m_path;
}

const std::string& CSource::GetCodeRef() const
{
    return m_ssSource;
}

void CSource::ReadFile(const std::filesystem::path& rpath)
{
    // Set the locale to work with UTF8
    std::setlocale(LC_ALL, "en_US.utf8");

    if (rpath.empty())
        throw CCompileException("File name missing");

    // Store the file name
    m_path = std::filesystem::absolute(rpath);

    // Open the file
    std::ifstream fstream(m_path, std::ios_base::in | std::ios::binary);

    // Is the file opened
    if (!fstream.is_open())
        throw CCompileException(m_path, CToken(), "Cannot open file");

    // Get the size of the file
    fstream.seekg(0, std::ios::end);
    size_t nSize = fstream.tellg();

    // Allocate space for the content
    std::unique_ptr<uint8_t[]> ptruiRawContent = std::make_unique<uint8_t[]>(nSize);
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptruiRawContent)
        throw CCompileException(m_path, CToken(), "Cannot allocate buffer for reading the file");

    // Read the file
    fstream.seekg(0);
    fstream.read(reinterpret_cast<char*>(ptruiRawContent.get()), nSize);
    if (static_cast<size_t>(fstream.gcount()) != nSize)
        throw CCompileException(m_path, CToken(), "Could not read complete file");

    // Determine the BOM
    if (nSize >= 3 && ptruiRawContent[0] == 0xEF && ptruiRawContent[1] == 0xBB && ptruiRawContent[2] == 0xBF)
    {
        // UTF8 BOM - assign the string directly to the member
        if (nSize > 3)
            m_ssSource = std::string(reinterpret_cast<const char*>(ptruiRawContent.get()) + 3, nSize - 3);
    } else
    if (nSize >= 4 && ptruiRawContent[0] == 0x00 && ptruiRawContent[1] == 0x00 && ptruiRawContent[2] == 0xFE && ptruiRawContent[3] == 0xFF)
    {
        // UTF32 BE BOM - conversion needed
        char32_t* szBuffer = reinterpret_cast<char32_t*>(ptruiRawContent.get()) + 1;
        nSize = nSize / sizeof(char32_t) - 1;
        PotentialSwapBuffer(szBuffer, nSize, true);
        m_ssSource = ConvertToUTF8(szBuffer, nSize);
    } else
    if (nSize >= 4 && ptruiRawContent[0] == 0xFF && ptruiRawContent[1] == 0xFE && ptruiRawContent[2] == 0x00 && ptruiRawContent[3] == 0x00)
    {
        // UTF32 LE BOM - conversion needed
        char32_t* szBuffer = reinterpret_cast<char32_t*>(ptruiRawContent.get()) + 1;
        nSize = nSize / sizeof(char32_t) - 1;
        PotentialSwapBuffer(szBuffer, nSize, false);
        m_ssSource = ConvertToUTF8(szBuffer, nSize);
    } else
    if (nSize >= 2 && ptruiRawContent[0] == 0xFE && ptruiRawContent[1] == 0xFF)
    {
        // UTF16 BE BOM - conversion needed
        char16_t* szBuffer = reinterpret_cast<char16_t*>(ptruiRawContent.get()) + 1;
        nSize = nSize / sizeof(char16_t) - 1;
        PotentialSwapBuffer(szBuffer, nSize, true);
        m_ssSource = ConvertToUTF8(szBuffer, nSize);
    } else
    if (nSize >= 2 && ptruiRawContent[0] == 0xFF && ptruiRawContent[1] == 0xFE)
    {
        // UTF16 LE BOM - conversion needed
        char16_t* szBuffer = reinterpret_cast<char16_t*>(ptruiRawContent.get()) + 1;
        nSize = nSize / sizeof(char16_t) - 1;
        PotentialSwapBuffer(szBuffer, nSize, false);
        m_ssSource = ConvertToUTF8(szBuffer, nSize);
    } else
    {
        // No BOM - ANSI character set assumed
        m_ssSource = std::string(reinterpret_cast<const char*>(ptruiRawContent.get()), nSize);
    }
}

constexpr bool CSource::IsSystemBigEndian()
{
    // Use a constant value to check for the byte order
    // False cppcheck warning; uiVal is seen as unused. This is not the case. Suppress warning
    // cppcheck-suppress unusedStructMember
    const union {uint16_t uiVal; uint8_t rguiBytes[2];} uTest{0x0011};

    // Check for big-endian byte order.
    return uTest.rguiBytes[0] == 0x00 && uTest.rguiBytes[1] == 0x11;
}

constexpr bool CSource::IsSystemLittleEndian()
{
    // Use a constant value to check for the byte order
    // False cppcheck warning; uiVal is seen as unused. This is not the case. Suppress warning
    // cppcheck-suppress unusedStructMember
    const union {uint16_t uiVal; uint8_t rguiBytes[2];} uTest{0x0011};

    // Check for big-endian byte order.
    return uTest.rguiBytes[0] == 0x11 && uTest.rguiBytes[1] == 0x00;
}

std::string CSource::ConvertToUTF8(const char16_t* szBuffer, size_t nSize)
{
    if (!szBuffer) return std::string();

    // Return the result.
    return sdv::MakeUtf8String(szBuffer, nSize);
}

std::string CSource::ConvertToUTF8(const char32_t* szBuffer, size_t nSize)
{
    if (!szBuffer) return std::string();

    // Return the result.
    return sdv::MakeUtf8String(szBuffer, nSize);
}
