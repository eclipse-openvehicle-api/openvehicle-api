#ifndef FILESYSTEM_HELPER
#define FILESYSTEM_HELPER

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <regex>

/**
 * @brief Check for a match of the path with the pattern containing zero or more wildcards.
 * @param[in] rpath Reference to the path to compare.
 * @param[in] rpathPattern Reference to the path containing zero or more wildcards.
 * @return Returns 'true' when the paths match, 'false' when not.
 */
inline bool MatchFileWithWildcards(const std::filesystem::path& rpath, const std::filesystem::path& rpathPattern)
{
    std::regex regexStarReplace("\\*");
    std::regex regexQuestionmarkReplace("\\?");

    std::string ssWildcardPattern = std::regex_replace(
        std::regex_replace(rpathPattern.generic_u8string(), regexStarReplace, ".*"),
        regexQuestionmarkReplace, ".");

    std::regex regexWildcardPattern("^" + ssWildcardPattern + "$");

    return std::regex_match(rpath.generic_u8string(), regexWildcardPattern);
}

/**
 * @brief Find the files using a split-up vector containing the path chunks and potential wildcard patterns.
 * @param[in] rvecFiles Reference to the list of files being found. This list will be extended iteratively.
 * @param[in] rpathBase Reference to the base path to match with the path/pattern chunks from the vector starting at the specified
 * index.
 * @param[in] rvecPatternChunks Reference with the split search path containing the path/pattern chunks.
 * @param[in] nIndex The current index that must match with the sub-directories and files in the base path.
 * @param[in] rsetSearch Set of search paths being updated to prevent searching a path multiple times. Searching multiple times
 * cannot be excluded when using generic wildcards that could represent zero or more sub-directories "*".
 */
void FindFilesWithWildcards(std::vector<std::filesystem::path>& rvecFiles, const std::filesystem::path& rpathBase,
    const std::vector<std::string>& rvecPatternChunks, size_t nIndex, std::set<std::filesystem::path>& rsetSearch)
{
    // Done if itPos points to the end of the split vector
    if (nIndex >= rvecPatternChunks.size())
    {
        // If this is a file, add the file to the list
        if (std::filesystem::is_regular_file(rpathBase))
            rvecFiles.push_back(rpathBase);
        else if (std::filesystem::is_directory(rpathBase))
        {
            // Recursively iterate through the directories and add all files.
            for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{ rpathBase })
            {
                if (std::filesystem::is_regular_file(dir_entry.path()))
                    rvecFiles.push_back(dir_entry.path());
            }
        }
        return;
    }

    const std::string& rssPatternChunk = rvecPatternChunks[nIndex];

    // Further processing is only possible if the pathBase is a directory...
    if (!std::filesystem::is_directory(rpathBase))
    {
        // In some rare cases, the path cannot be identified as directory, but still is one. Add the next chunk.
        FindFilesWithWildcards(rvecFiles, rpathBase / rssPatternChunk, rvecPatternChunks, nIndex + 1, rsetSearch);
        return;
    }

    // Check whether the current position contains wildcards; if not, add it to the base path and go one deeper.
    if (rvecPatternChunks[nIndex].find_first_of("*?") == std::string::npos)
    {
        FindFilesWithWildcards(rvecFiles, rpathBase / rssPatternChunk, rvecPatternChunks, nIndex + 1, rsetSearch);
        return;
    }

    // Due to the use of wildcards, it could happen, that directories get searched multiple times. If searched before, skip another
    // search...
    std::filesystem::path pathSearchPath = rpathBase / rssPatternChunk;
    if (rsetSearch.find(pathSearchPath) != rsetSearch.end()) return;
    rsetSearch.insert(pathSearchPath);

    // Iterator through the base directory to see if a matching path can be made
    std::filesystem::path pathPattern = rpathBase;
    pathPattern /= rssPatternChunk;
    for (auto const& dir_entry : std::filesystem::directory_iterator{ rpathBase })
    {
        // Does the pattern fit?
        if (MatchFileWithWildcards(dir_entry.path(), pathPattern))
        {
            // Special case: if the current string is "*" this might also be valid for any child path. Check this first...
            if (rssPatternChunk == "*")
                FindFilesWithWildcards(rvecFiles, dir_entry.path(), rvecPatternChunks, nIndex, rsetSearch);

            // Increase the position of our pattern, check whether the path first
            FindFilesWithWildcards(rvecFiles, dir_entry.path(), rvecPatternChunks, nIndex + 1, rsetSearch);

            // Special case: if the current string is "*" the rest of the pattern might fit the current path (doing as if the
            // wildcard doesn't exist).
            if (rssPatternChunk == "*")
                FindFilesWithWildcards(rvecFiles, rpathBase, rvecPatternChunks, nIndex + 1, rsetSearch);
        }
    }
}

///
/// @brief Find the files using the provided path.
/// @details Search for the files matching the providing search path. The search path could contain wildcards ('*' and '?').
/// Examples of path search criteria:
/// @code
/// /usr/*              - all files in /usr and all sub-directories
/// /usr/*.sdv          - all .sdv-files in /usr
/// /usr/*/*.sdv        - all .sdv-files in /usr and all sub-directories
/// /usr/*/abc/*.sdv    - looks in /usr for all .sdv-files in a sub-directory abc
/// /usr/abc*/*.sdv     - looks in /usr for all directories starting with abc and takes the .sdv-files from this directories
/// @endcode
/// @param[in] rpathSearch Search path with zero or more wildcards. The path is assumed to be an absolute path.
/// @return Returns a vector with all the found files (full path).
///
inline std::vector<std::filesystem::path> FindFilesWithWildcards(const std::filesystem::path& rpathSearch)
{
    // NOTE: The std::filesystem::path type has an iterator to iterate through the path. Unfortunately it is not possible to clone
    // an iterator and iterate independently further... the clone operators on the same iterator as the original.
    // The solution is to build a vector with the path-chunks and a vector index, which in any case is cloneable.

    // Split the path
    std::vector<std::filesystem::path> vecFiles;
    std::vector<std::string> vecPatternChunks;
    for (const std::filesystem::path& rpathChunk : rpathSearch)
        vecPatternChunks.push_back(rpathChunk.u8string());
    if (vecPatternChunks.empty()) return vecFiles;

    // Build the root search path without the wildcard
    std::set<std::filesystem::path> setSearch;
    FindFilesWithWildcards(vecFiles, vecPatternChunks[0], vecPatternChunks, 1, setSearch);
    return vecFiles;
}


#endif // !defined FILESYSTEM_HELPER