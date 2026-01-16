#ifndef WILDCARD_MATCH_H
#define WILDCARD_MATCH_H

#include <filesystem>
#include <string>
#include <vector>
#include <regex>
#include <stdexcept>
#include <stack>
#include <list>

/**
 * @brief Check for a match between a relative path and a pattern containing zero or more wildcards.
 * @details Check for the relative path matching a relative path-pattern possibly containing wildcard patterns. The following rules
 * to patterns apply:
 *  - Definition of path-part: paths are split into directories optionally followed by a filename. As separator '/' is used.
 *  - A directory name of '.' has a special meaning directing to the current directory.
 *  - A directory name of '..' has a special meaning directing to a directory one up.
 *  - Directory and file name can be separated with dots (for example "dir1.2/file0.bin").
 *  - Within the path-part following zero or more characters, use '?' for any one character that might fit. The '?' might be
 *    followed by zero or more characters and  wildcards.
 *  - Within the path-part following zero or more characters, use '*' for zero or more character until the next character or
 *    separator. The '*' might be followed by zero or more characters and wildcards.
 *  - It should be possible to use a directory as a pattern (without the file name) - then all the files of the directory are
 *    matching (but not the files of the subdirectories).
 *  - If the path-part equals '**', zero or more path-parts can be skipped to fit the rest of the path. This rule doesn't apply if
 *    the part-part equals '*' and is the last part in the path.
 * Examples are:
 *  - "subdir1" - directory only; all files within the directory (but not any sub-directories) match.
 *  - "subdir1" - all files within the directory match.
 *  - "subdir2/file2?.*n" - all files located in subdir2 starting with "file2" followed by a single digit or character with the
 *    extension ending with "n" match.
 *  - "subdir?_*" - all files from a directory starting with the name "subdir" followed by a single digit or character, followed
 *    with a "_" and zero or more characters.
 *  - "**\\/file*.bin" - all files starting with the name "file" followed by zero or more characters and the extension ".bin" in this
 *    and any subdirectory.
 * @param[in] rpathRel Reference to the relative path to check for a match.
 * @param[in] rssPattern Reference to the string containing the pattern to match.
 * @return Returns whether the path matches.
 */
inline bool CheckWildcardPathMatch(const std::filesystem::path& rpathRel, const std::string& rssPattern)
{
    std::filesystem::path pathPattern = std::filesystem::u8path(rssPattern);
    auto itPathRel = rpathRel.begin();
    auto itPathPattern = pathPattern.begin();

    // Iterate through the paths
    bool bLastPartAsterisk = false; // Special case when the last checked part is a single asterisk.
    while (itPathRel != rpathRel.end() && itPathPattern != pathPattern.end())
    {
        // Get the parts and increase
        std::string ssPathPart = itPathRel->generic_u8string();
        std::string ssPatternPart = itPathPattern->generic_u8string();
        itPathRel++;
        itPathPattern++;

        // Compose a new path from the rest of the path starting at the provided iterator.
        auto fnBuildNewPath = [](std::filesystem::path::iterator& ritPath, const std::filesystem::path& rpath)
        {
            std::filesystem::path pathNew;
            while (ritPath != rpath.end())
            {
                pathNew /= *ritPath;
                ++ritPath;
            }
            return pathNew;
        };

        // Special case... skipping zero or more path parts.
        if (ssPatternPart == "**")
        {
            // The pattern "**/.." or even multiple higher directories like "**/../.." would indicate that any number of directories
            // can be skipped, but at least the amount of directories need to be present to be able to go higher. By building
            // potential relative paths (below), skip the amount of paths going higher.
            size_t nSkipParent = 0;
            while (itPathPattern != pathPattern.end() && itPathPattern->generic_u8string() == "..")
            {
                itPathPattern++;
                nSkipParent++;
            }

            // Build a new pattern from the rest of the pattern.
            std::filesystem::path pathNewPattern = fnBuildNewPath(itPathPattern, pathPattern);

            // Build a list of new potential relative paths.
            // NOTE: More efficient would be to execute the matching algorithm for each path-part until the end of the path.
            // Since the path iterator doesn't allow copying like it does with the container classes, it doesn't work to reuse the
            // ierator to build each new path on the fly. Instead, first create a list of potential paths and then execute the
            // matching process.
            std::vector<std::filesystem::path> vecNewRelPaths;
            itPathRel--;    // Include the current path part as well.
            while (itPathRel != rpathRel.end())
            {
                // Skip the amount of parent parts if ".." was detected.
                if (nSkipParent)
                {
                    itPathRel++;
                    nSkipParent--;
                    continue;
                }

                // Add to all existing paths in the vector
                for (auto& rpath : vecNewRelPaths)
                    rpath /= *itPathRel;

                // Add as a separate part to the vector
                vecNewRelPaths.push_back(*itPathRel);

                // Next part
                itPathRel++;
            }

            // When the amount of parents to be skipped is higher than 0, there is no parent to skip anymore.
            if (nSkipParent) return false;

            // Iterate through the list of potential relative paths and check for at least one fit...
            for (const auto& rpathNewRel : vecNewRelPaths)
            {
                // Check for this path. If it matches... it is valid.
                if (CheckWildcardPathMatch(rpathNewRel, pathNewPattern.generic_u8string()))
                    return true;
            }

            // If none fit, there is no match.
            return false;
        }

        // Special case... ignore current test.
        if (ssPatternPart == ".")
        {
            // Redo the last path iteration and continue with the matching process.
            itPathRel--;
            continue;
        }

        // Special case... use parent path.
        if (ssPatternPart == "..")
        {
            // If the path also has a higher directory, everything is good
            if (ssPathPart == "..") continue;

            // Redo the last path iteration.
            itPathRel--;

            // Does a higher part path exists?
            if (itPathRel == rpathRel.begin())
            {
                // A higher directory doesn't exist. Create a new pattern starting with "..".
                std::filesystem::path pathNewPattern = fnBuildNewPath(itPathPattern, pathPattern);

                //  Create a new path from the path starting with "..".
                std::filesystem::path pathNew = ".." / rpathRel;

                // Redo the matching process.
                // NOTE: Since ".." was prepended, only generic patterns with "*" and "**" can match.
                return CheckWildcardPathMatch(pathNew, pathNewPattern.generic_u8string());
            }
            else
            {
                // Redo the path before the last path iteration as well.
                itPathRel--;
                continue;
            }
        }

        // Special case... skipping the current part
        if (ssPatternPart == "*")
        {
            // Are there any pattern parts following; if not, the last pattern part is an asterisk.
            if (itPathPattern == pathPattern.end())
                bLastPartAsterisk = true;
            else if (itPathPattern->generic_u8string() == "..")
            {
                // The combination "*/.." cancels itself.
                itPathRel--;
                itPathPattern++;
                continue;
            }
        }

        // Iterate through the path parts
        size_t nPathPartPos = 0;
        size_t nPatternPartPos = 0;
        while (nPathPartPos != ssPathPart.size() && nPatternPartPos != ssPatternPart.size())
        {
            // Get the characters and increase
            char cPathPart = ssPathPart[nPathPartPos];
            char cPatternPart = ssPatternPart[nPatternPartPos];
            nPathPartPos++;
            nPatternPartPos++;

            // Classify and check with the pattern.
            switch (cPatternPart)
            {
            case '?':
                // Any character is allowed.
                break;
            case '*':
                // Skip zero or more wildcard-characters following the asterisk.
                while (nPatternPartPos != ssPatternPart.size() &&
                    (ssPatternPart[nPatternPartPos] == '*' || ssPatternPart[nPatternPartPos] == '?'))
                    nPatternPartPos++;

                // When there are no more characters following, the rest of the path part fits.
                if (nPatternPartPos == ssPatternPart.size())
                {
                    nPathPartPos = ssPathPart.size();
                    break;
                }

                // Skip zero or more characters in the path part until the next fitting characters.
                cPatternPart = ssPatternPart[nPatternPartPos];
                nPathPartPos--; // Start with the current path
                while (nPathPartPos != ssPathPart.size() && ssPathPart[nPathPartPos] != cPatternPart)
                    nPathPartPos++;

                // When the path part pos has reached the end, there is no match.
                if (nPathPartPos == ssPathPart.size())
                    return false;

                // Go to the next character
                nPathPartPos++;
                nPatternPartPos++;
                break;
            default:
                // Character needs to fit
                if (cPathPart != cPatternPart) return false;
                break;
            }
        }

        // If the path part has reached the end, but the pattern still has space, this might indicate a mismatch. But only when the
        // rest of the pattern doesn't contain one or more asterisk '*' until the end of the path. Simply skip these characters.
        while (nPatternPartPos != ssPatternPart.size() && ssPatternPart[nPatternPartPos] == '*')
            nPatternPartPos++;

        // When both have reached the end, the path parts are identical.
        if (nPathPartPos == ssPathPart.size() && nPatternPartPos == ssPatternPart.size())
            continue;

        // No match
        return false;
    }

    // When both have reached the end of iteration, we have a match of files.
    if (itPathRel == rpathRel.end() && itPathPattern == pathPattern.end())
        return true;

    // When only itPathPattern has reached the end of iteration, we have a match of directories... All files in the directory (but
    // not in any sub-directories) are valid. The exception would be if the pattern was "*", which is forced here to match files
    // only. In that case bLastPartAsterisk would be true.
    if (itPathRel != rpathRel.end() && itPathPattern == pathPattern.end() && !bLastPartAsterisk)
    {
        itPathRel++;
        return itPathRel == rpathRel.end();
    }

    // No match...
    return false;
}

/**
 * @brief Normalizes a regex path pattern string.
 *
 * This function processes a path string to resolve current ('.') and parent ('..')
 * directory indicators according to standard path canonicalization rules, plus
 * custom rules for regex generation.
 *
 * Rules applied:
 * 1.  Path segments of '.' are removed (e.g., "a/./b" -> "a/b").
 * 2.  A directory followed by '..' is resolved (e.g., "a/b/../c" -> "a/c"). Special case if the parent is any directory ".*\\.\\./"
 *     and ".*\/\\.\\./". Both will result in any directory ".*\/".
 * 3.  If a path attempts to navigate above the root (e.g., "../a" or "a/../../b"), this only works if the following directory is
 *     any directory (starts with ".*").
 * @param[in] rssPattern Reference to the regex pattern string to normalize.
 * @return The normalized regex pattern string.
 */
inline std::string NormalizeRegexPattern(const std::string& rssPattern)
{
    // Throw regex group mismatch exception
    auto fnThrowGroupException = [](char c)
    {
        switch (c)
        {
        case '(':
        case ')':
            throw std::regex_error(std::regex_constants::error_paren);
        case '[':
        case ']':
            throw std::regex_error(std::regex_constants::error_brack);
        case '{':
        case '}':
            throw std::regex_error(std::regex_constants::error_brace);
        default:
            break;
        }
    };

    // Return the matching group character (parenthesis, brakcets or braces)
    auto fnGroupMatch = [](char cOpenGroup) -> char
    {
        switch (cOpenGroup)
        {
        case '(': return ')';
        case ')': return '(';
        case '[': return ']';
        case ']': return '[';
        case '{': return '}';
        case '}': return '{';
        default: return cOpenGroup;
        }
    };

    // Split the pattern in chunks
    std::list<std::string> lstPattern;
    std::string ssPart;
    size_t nPos = 0;
    std::stack<char> stackGroup;
    while (nPos < rssPattern.size())
    {
        char c = rssPattern[nPos];
        nPos++;

        switch (c)
        {
        case '(': // Group (...)
        case '[': // Group [...]
        case '{': // Group {...}
            stackGroup.push(c);
            ssPart += c;
            break;
        case ')': // Group (...)
        case ']': // Group [...]
        case '}': // Group {...}
            if (stackGroup.empty()) fnThrowGroupException(c);
            if (fnGroupMatch(stackGroup.top()) != c) fnThrowGroupException(stackGroup.top());
            stackGroup.pop();
            ssPart += c;
            break;
        case '\\': // Escape
            ssPart += c;
            if (nPos >= rssPattern.size()) throw std::regex_error(std::regex_constants::error_escape);
            c = rssPattern[nPos];
            ssPart += c;
            nPos++;
            break;
        case '/': // Separator
            ssPart += c;
            if (stackGroup.empty())
            {
                lstPattern.push_back(std::move(ssPart));
                ssPart.clear();
            }
            break;
        default:
            ssPart += c;
            break;
        }
    }
    if (!stackGroup.empty()) fnThrowGroupException(stackGroup.top());
    if (!ssPart.empty())
        lstPattern.push_back(std::move(ssPart));

    // Erase all entries to the current directory
    lstPattern.erase(std::remove(lstPattern.begin(), lstPattern.end(), "\\./"), lstPattern.end());

    // Handle parent directories
    auto itPart = lstPattern.begin();
    while (itPart != lstPattern.end())
    {
        // Next
        auto itCurrent = itPart;
        itPart++;

        // Check for any directory followed by a parent directory - change to any directory: ".*\\.\\./"
        if (*itCurrent == ".*\\.\\./")
        {
            *itCurrent = ".*/";
            continue;
        }

        // Check for parent directory
        if (*itCurrent == "\\.\\./")
        {
            // Check whether there is a directory before
            if (itCurrent != lstPattern.begin())
            {
                // Get the directory before
                auto itBefore = itCurrent;
                itBefore--;

                // Anything with .* in it will be truncated: "abc.*/", ".*jojo/", ".*/"
                nPos = itBefore->rfind(".*");
                if (nPos != std::string::npos)
                {
                    *itBefore = itBefore->substr(nPos, 2) + '/';
                    lstPattern.erase(itCurrent);
                    continue;
                }

                // All others are considered a single directory and will be removed
                lstPattern.erase(itBefore);
                lstPattern.erase(itCurrent);
                continue;
            }
            else if (itPart != lstPattern.end())
            {
                // Get the directory behind
                auto itBehind = itPart;
                itPart++;

                // Get the directory after. This can only work if the directory after can be anything: starting with ".*/", ".*anything" or "[^/]+/"
                if (*itBehind == ".*/")
                {
                    lstPattern.erase(itCurrent);
                    *itBehind = ".*";   // Remove trailing slash
                    continue;
                }
                if (itBehind->substr(0, 2) == ".*")
                {
                    lstPattern.erase(itCurrent);
                    continue;
                }
                else if (*itBehind == "[^/]+/")
                {
                    lstPattern.erase(itCurrent);
                    lstPattern.erase(itBehind);
                    continue;
                }

                // Invalid use of "\\.\\."
                throw std::regex_error(std::regex_constants::error_complexity);
            }
            else
            {
                // Invalid use of "\\.\\."
                throw std::regex_error(std::regex_constants::error_complexity);
            }
        }
    }

    // Rebuild the pattern
    std::string ssPatternNew;
    for (const std::string& rssPart : lstPattern)
        ssPatternNew += rssPart;

    return ssPatternNew;
}

/**
 * @brief Checks if a path matches a given regex pattern.
 * @details The matching is performed against the entire path.
 * @remarks For the regex patterns, it is assumed that navigation directories like "." and ".." are to be matched as literal text.
 * Therefore, in the regex pattern, dots should be escaped (e.g., `\.` and `\.\.`).
 * @param[in] rpathRel Reference to the existing relative filesystem path to check.
 * @param[in] rssPattern Reference to the pattern expressed as a regular expression.
 * @return Returns if the path matches the pattern.
 */
inline bool CheckRegexPathMatch(const std::filesystem::path& rpathRel, const std::string& rssPattern)
{
    try
    {
        // Convert paths to generic, portable string format (with '/' separators)
        const std::string ssPath = rpathRel.generic_u8string();

        // Use the ECMAScript grammar, which is the default and most common.
        const std::regex rgx(NormalizeRegexPattern(rssPattern), std::regex::ECMAScript);

        // std::regex_match checks if the entire string is a match for the pattern.
        return std::regex_match(ssPath, rgx);
    }
    catch (const std::regex_error& e)
    {
        // In case of an invalid regex pattern, log the error and return false.
        std::cerr << "Regex error: " << e.what() << " for pattern: " << rssPattern << std::endl;
        return false;
    }
}

/**
 * @brief Algorithm to be used for matching the path to the pattern.
 */
enum class EPathMatchAlgorithm
{
    wildcards,      ///< Use wildcards like '?', '*' and '**'
    regex,          ///< Use regular expressins
};

/**
 * @brief Collect a vector of paths from a wildcard pattern.
 * @param[in] rpathBase Reference to the base path to start collecting.
 * @param[in] rssPattern Reference to the string containing the pattern to match files with.
 * @param[in] eAlgorithm The algorithm to use for path collection.
 * @return Returns a vector with paths that are matching the pattern.
 */
inline std::vector<std::filesystem::path> CollectWildcardPath(const std::filesystem::path& rpathBase,
    const std::string& rssPattern, EPathMatchAlgorithm eAlgorithm = EPathMatchAlgorithm::wildcards)
{
    std::vector<std::filesystem::path> pathMatches;
    try
    {
        std::filesystem::path pathBaseCopy = rpathBase;
        std::string ssPatternCopy = rssPattern;
        bool bUseAbsolutePath = false;
        if (pathBaseCopy.empty())
        {
            // Get the first position of a path not including wildcard and/or regex special characters
            size_t nPos = rssPattern.find_first_of("[](){}*?.\\+");

            // Get the directory separator before that
            nPos = rssPattern.substr(0, nPos).find_last_of('/');

            // The base pattern is everything before the '/'.
            if (nPos != std::string::npos)
            {
                pathBaseCopy = rssPattern.substr(0, nPos);
                ssPatternCopy = rssPattern.substr(nPos + 1);
                bUseAbsolutePath = true;
            }
        }
        else if (std::filesystem::path(rssPattern).is_absolute())
        {
            // Separator processing function.
            size_t nSearchPos = 0;
            auto fnProcessSeparator = [&rssPattern, &nSearchPos]() -> bool
            {
#ifdef _WIN32
                if (nSearchPos >= rssPattern.size() || (rssPattern[nSearchPos] != '/' && rssPattern[nSearchPos] != '\\'))
#else
                if (nSearchPos >= rssPattern.size() || rssPattern[nSearchPos] != '/')
#endif
                    return false;
                nSearchPos++;
                return true;
            };

            // Remove the base from the pattern.
            bool bExplicitSeparator = false;
            for (const auto& rpathBasePart : rpathBase)
            {
                // Skip separator in base path
#ifdef _WIN32
                if (rpathBasePart == "/" || rpathBasePart == "\\")
#else
                if (rpathBasePart == "/")
#endif
                {
                    fnProcessSeparator();
                    bExplicitSeparator = true;
                    continue;
                }

                // Separator needed?
                if (!bExplicitSeparator && nSearchPos && !fnProcessSeparator())
                    return {};
                bExplicitSeparator = false;

                // Check for the base part inside the pattern
                std::string ssPathPart = rpathBasePart.generic_u8string();
                if (rssPattern.substr(nSearchPos, ssPathPart.size()) != ssPathPart)
                    return {};
                nSearchPos += ssPathPart.size();
            }

            // Skip trailing slash if existing
#ifdef _WIN32
            if (nSearchPos < rssPattern.size() &&
                (rssPattern[nSearchPos] == '/' || (rssPattern[nSearchPos] == '\\' && eAlgorithm == EPathMatchAlgorithm::wildcards)))
#else
            if (nSearchPos < rssPattern.size() && rssPattern[nSearchPos] == '/')
#endif
                nSearchPos++;

            // Copy the pattern following the parg
            ssPatternCopy = rssPattern.substr(nSearchPos);

            // If the pattern was absolute and the represents exactly the base path, search for all files in the top most directory.
            if (ssPatternCopy.empty())
            {
                // Add wildcards or regex structure for all files in all directories
                switch (eAlgorithm)
                {
                case EPathMatchAlgorithm::regex:
                    ssPatternCopy = "[^/]+";
                    break;
                case EPathMatchAlgorithm::wildcards:
                default:
                    ssPatternCopy = "*";
                    break;
                }
            }
        }

        // If there is no pattern, then all content of the directory of the base path is required.
        if (ssPatternCopy.empty())
        {
            // Add wildcards or regex structure for all files in all directories
            switch (eAlgorithm)
            {
            case EPathMatchAlgorithm::regex:
                ssPatternCopy = ".*";
                break;
            case EPathMatchAlgorithm::wildcards:
            default:
                ssPatternCopy = "**";
                break;
            }
        }

        // Iterate through the path files of the base path.
        for (auto const& rDirEntry : std::filesystem::recursive_directory_iterator(pathBaseCopy))
        {
            if (!rDirEntry.is_regular_file()) continue;
            std::filesystem::path pathRel = rDirEntry.path().lexically_relative(pathBaseCopy);
            std::filesystem::path pathToAdd = bUseAbsolutePath ? rDirEntry.path() : pathRel;
            switch (eAlgorithm)
            {
            case EPathMatchAlgorithm::wildcards:
                if (CheckWildcardPathMatch(pathRel, ssPatternCopy))
                    pathMatches.push_back(pathToAdd);
                break;
            case EPathMatchAlgorithm::regex:
                if (CheckRegexPathMatch(pathRel, ssPatternCopy))
                    pathMatches.push_back(pathToAdd);
                break;
            default:
                break;
            }
        }
    }
    catch (std::filesystem::filesystem_error& rexception)
    {
        std::cerr << rexception.what();
    }
    return pathMatches;
}

#endif // WILDCARD_MATCH_H