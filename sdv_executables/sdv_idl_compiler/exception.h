#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "logger.h"
#include "token.h"
#include <exception>
#include <sstream>
#include <cassert>
#include <filesystem>
#include <iostream>

// Forward declaration
class CParser;

/**
 * @brief Lexer exception
 */
struct CCompileException : public sdv::idl::XCompileError
{
    friend class CParser;

public:
    /**
     * @brief Copy the compile error.
     * @param[in] rxCompileError Reference to the compile error.
    */
    CCompileException(const sdv::idl::XCompileError& rxCompileError);

    /**
     * @brief Constructor without token and path. The parser might add the last used token.
     * @remarks The path can be added to the exception using the SetPath function.
     * @param[in] szReason Zero terminated string containing the reason of the exception. Must not be NULL.
     * @param[in] tAdditionalReasons Optional other reasons.
     */
    template <class... TAdditionalReason>
    explicit CCompileException(const char* szReason, TAdditionalReason... tAdditionalReasons);

    /**
     * @brief Constructor without path.
     * @remarks The path can be added to the exception using the SetPath function.
     * @param[in] rtoken Token in the source file that triggered the exception.
     * @param[in] szReason Zero terminated string containing the reason of the exception. Must not be NULL.
     * @param[in] tAdditionalReasons Optional other reasons.
     */
    template <class... TAdditionalReason>
    explicit CCompileException(const CToken& rtoken, const char* szReason, TAdditionalReason... tAdditionalReasons);

    /**
     * @brief Constructor with path.
     * @param[in] rpath Reference to the source file path.
     * @param[in] rtoken Token in the source file that triggered the exception.
     * @param[in] szReason Zero terminated string containing the reason of the exception. Must not be NULL.
     * @param[in] tAdditionalReasons Optional other reasons.
     */
    template <class... TAdditionalReason>
    explicit CCompileException(const std::filesystem::path& rpath, const CToken& rtoken,
        const char* szReason, TAdditionalReason... tAdditionalReasons);

    /**
     * @brief Get the path.
     * @return Reference to the path of the file causing the exception (if there is one).
     */
     std::string GetPath() const;

    /**
     * @brief Get the reason.
     * @return The explanatory text.
     */
    std::string GetReason() const;

    /**
     * @brief Get the line number (starts at 1).
     * @return The line number or 0 when there is no specific code causing this error.
     */
    uint32_t GetLineNo() const;

    /**
     * @brief Get the column number (starts at 1).
     * @return The column number or 0 when there is no specific code causing this error.
     */
    uint32_t GetColNo() const;

    /**
     * @brief Get the token causing the compilation error.
     * @return The token string or an empty string when no specific code is causing this error.
     */
    std::string GetToken() const;

    /**
     * @brief Get the line containing the error.
     * @return The line that contains the error or an empty string when no specific code is causiong this error.
     */
    std::string GetLine() const;

private:
    /**
     * @brief Set the path if not assigned before.
     * @param[in] rpath Reference to the source file path.
     */
    void SetPath(const std::filesystem::path& rpath);

    /**
     * @brief Set the location if not assigned before.
     * @param[in] rtoken The token that caused the exception.
     */
    void SetLocation(const CToken& rtoken);
};

template <class TParam>
void AddParamListToString(std::stringstream& rsstream, TParam param)
{
    rsstream << param;
}

template <class TParam, class... TAdditionalParams>
void AddParamListToString(std::stringstream& rsstream, TParam param, TAdditionalParams... tAdditionalParams)
{
    rsstream << param;
    AddParamListToString(rsstream, tAdditionalParams...);
}

template <class... TAdditionalReason>
CCompileException::CCompileException(const char* szReason, TAdditionalReason... tAdditionalReasons) : sdv::idl::XCompileError{}
{
    assert(szReason);
    if (szReason)
    {
        std::stringstream sstream;
        AddParamListToString(sstream, szReason, tAdditionalReasons...);
        ssReason = sstream.str();
    }
    CLog log;
    log << "EXCEPTION: " << ssReason << std::endl;
}

template <class... TAdditionalReason>
CCompileException::CCompileException(const CToken& rtoken, const char* szReason, TAdditionalReason... tAdditionalReasons) :
    sdv::idl::XCompileError{}
{
    assert(szReason);
    if (szReason)
    {
        std::stringstream sstream;
        AddParamListToString(sstream, szReason, tAdditionalReasons...);
        ssReason = sstream.str();
    }
    SetLocation(rtoken);
    CLog log;
    log << "EXCEPTION " << "[" << GetLineNo() << ", " << GetColNo() << "]: " << ssReason << std::endl;
}

template <class... TAdditionalReason>
CCompileException::CCompileException(const std::filesystem::path& rpath, const CToken& rtoken,
    const char* szReason, TAdditionalReason... tAdditionalReasons) :
    CCompileException::CCompileException(rtoken, szReason, tAdditionalReasons...)
{
    assert(szReason);
    if (szReason)
    {
        std::stringstream sstream;
        AddParamListToString(sstream, szReason, tAdditionalReasons...);
        ssReason = sstream.str();
    }
    SetLocation(rtoken);
    ssFile = rpath.generic_u8string();
    CLog log;
    log << "EXCEPTION " << rpath.generic_u8string() << " [" << GetLineNo() << ", " << GetColNo() << "]: " << ssReason << std::endl;
}

#endif // !defined EXCEPTION_H
