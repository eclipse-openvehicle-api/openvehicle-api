#ifndef CONTEXT_H
#define CONTEXT_H

#include <cstdint>
#include <filesystem>
#include <support/string.h>
#include <support/sequence.h>

/**
 * @brief SDV context
*/
struct SContext
{
    bool                            bSilent = false;            ///< Silence flag
    bool                            bVerbose = false;           ///< Verbose flag
    bool                            bServerSilent = false;      ///< Silence flag of server
    bool                            bServerVerbose = false;     ///< Verbose flag of server
    uint32_t                        uiInstanceID = 1000;        ///< Instance ID
    bool                            bListNoHdr = false;         ///< Do not print a header with the listing table.
    bool                            bListShort = false;         ///< Print only a shortened list with one column.
    sdv::sequence<sdv::u8string>    seqCmdLine;                 ///< The commands provided on the command line.
    std::filesystem::path           pathInstallDir;             ///< Optional installation directory.
};

#include <cctype>    // std::tolower
#include <algorithm> // std::equal

/**
 * @brief Compare whether two characters are identical when both are converted to lower case.
 * @remarks Cannot deal with Unicode letters comprising of more than one characters.
 * @param[in] cLeft First character
 * @param[in] cRight Second character
 * @return The case independent equality of the characters.
 */
inline bool ichar_equals(char cLeft, char cRight)
{
    return std::tolower(static_cast<unsigned char>(cLeft)) ==
        std::tolower(static_cast<unsigned char>(cRight));
}

/**
 * @brief Compare for case-independent equality.
 * @param[in] rssLeft Reference to the left string.
 * @param[in] rssRight Reference to the right string.
 * @return The case-independent equality of the strings.
 */
inline bool iequals(const std::string& rssLeft, const std::string& rssRight)
{
    return std::equal(rssLeft.begin(), rssLeft.end(), rssRight.begin(), rssRight.end(), ichar_equals);
}

#endif // !defined CONTEXT_H