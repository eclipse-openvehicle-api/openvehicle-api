#include "path_match_test_suite.h"
#include "../../../global/path_match.h"
#include "../../../global/exec_dir_helper.h"

using CRegexPathMatchTest = CPathMatchTest;

TEST_F(CRegexPathMatchTest, DirectFile)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir1/file10\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match exactly "subdir1/file10.bin"
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir3_xyz/subdir4/subdir5/file50a\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match exactly "subdir3_xyz/subdir4/subdir5/file50a.bin"
}

TEST_F(CRegexPathMatchTest, DirectDir)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir2/[^/]+", EPathMatchAlgorithm::regex).size(), 3); // Match all files of the "subdir2" directory
}

TEST_F(CRegexPathMatchTest, DoubleAsterisk_Wildcard)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    EXPECT_EQ(CollectWildcardPath(rpathBase, ".*", EPathMatchAlgorithm::regex).size(), 10); // Match all files of all directories
    EXPECT_EQ(CollectWildcardPath(rpathBase, ".*/file50a\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match "file50a.bin" in any directory
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir3_xyz/.*", EPathMatchAlgorithm::regex).size(), 5); // Match all files in subdir3_xyz directory and any directory underneath
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir3_xyz/.*/file50a\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match "file50a.bin" in subdir3_xyz directory and any directory underneath
    EXPECT_EQ(CollectWildcardPath(rpathBase, "(.*/)?subdir1/[^/]+", EPathMatchAlgorithm::regex).size(), 2); // Match all files in any directory with the name "subdir1"
    EXPECT_EQ(CollectWildcardPath(rpathBase, "(.*/)?subdir4/[^/]+", EPathMatchAlgorithm::regex).size(), 1); // Match all files in any directory with the name "subdir4"
    EXPECT_EQ(CollectWildcardPath(rpathBase, "(.*/)?subdir3_xyz/.*/subdir5/[^/]+", EPathMatchAlgorithm::regex).size(), 1); // Match all files in any directory with the name "subdir5" underneath any directory with the name "subdir3_xyz"
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir3_xyz/(.*/)?subdir4/(.*/)?subdir5/[^/]+", EPathMatchAlgorithm::regex).size(), 1); // Match all files in the directory with the name "subdir5" underneath any directory with the name "subdir4" underneath the directory with the name "subdir3_xyz"
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir[^/]*/.*", EPathMatchAlgorithm::regex).size(), 10); // Match all files in any directory (as well as subdirectories) starting with the name "subdir"
    EXPECT_EQ(CollectWildcardPath(rpathBase, ".*/subdir5/[^/]+", EPathMatchAlgorithm::regex).size(), 1); // Match all files in any directory or subdirectory with the name "subdir5"
}

TEST_F(CRegexPathMatchTest, File_Asterisk_Wildcard)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir1/[^/]+", EPathMatchAlgorithm::regex).size(), 2); // Match all files in the "subdir1" directory
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir1/[^/]+", EPathMatchAlgorithm::regex).size(), 2); // Match all files with any filename and any extension in the "subdir1" directory
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir2/[^/]*2[^/]*", EPathMatchAlgorithm::regex).size(), 3); // Match all files with any filename having "2" in the name and any extension in the "subdir2" directory
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir3_xyz/file[^/]*", EPathMatchAlgorithm::regex).size(), 3); // Match all files in the "subdir3_xyz" directory (but not subdirectory) starting with "file" in its filename
    EXPECT_EQ(CollectWildcardPath(rpathBase, "[^/]+/[^/]+", EPathMatchAlgorithm::regex).size(), 8); // Match all files in any top directory (but not from the subdirectories)
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir3_xyz/[^/]+", EPathMatchAlgorithm::regex).size(), 3); // Match all files in "subdir3_xyz" (but not from the subdirectories)
}

TEST_F(CRegexPathMatchTest, Dir_Asterisk_Wildcard)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir[^/]*/[^/]+", EPathMatchAlgorithm::regex).size(), 8); // Match all files in the top directory starting with "subdir" (but not from the subdirectories)
}

TEST_F(CRegexPathMatchTest, File_QuestionMark_Wildcard)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir2/file2\\w\\.bin", EPathMatchAlgorithm::regex).size(), 2); // Match all files in "subdir2" starting with the filename "file2" followed by an alpha-numerical character and ending with the extension "bin"
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir1/file1[^1]\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match all files in "subdir1" starting with the filename "file1" followed by an alpha-numerical character except the the character "1" and ending with the extension "bin"
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir2/file2\\w\\.b\\w\\w", EPathMatchAlgorithm::regex).size(), 3); // Match all files in "subdir2" starting with the filename "file2" followed by an alpha-numerical character and an extension starting with "b" followed by two alpha-numerical characters
}

TEST_F(CRegexPathMatchTest, Dir_QuestionMark_Wildcard)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir\\w/[^/]+", EPathMatchAlgorithm::regex).size(), 5); // Match all files from the directory starting with "subdir" followed by an alpha-numerical character (but not from the sub-directories)
}

TEST_F(CRegexPathMatchTest, Combination_Wildcards)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir\\w+_xyz/[^/]+", EPathMatchAlgorithm::regex).size(), 3); // Match all files with any name from the directory starting with "subdir" followed by an alpha-numerical character and followed by "*xyz"
    EXPECT_EQ(CollectWildcardPath(rpathBase, ".*/file\\w{2}a\\..*", EPathMatchAlgorithm::regex).size(), 3); // Match all files in any of the directories starting with "file" followed by two alpha-numerical characters followed by "a" with any extension
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir3_.*/[^/]*50[^/]*", EPathMatchAlgorithm::regex).size(), 1); // Match all files having "50" in its file name with any extension in the directory starting with "subdir3*" followed by any text or in any of its subdirectories
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir\\w/[^/]+", EPathMatchAlgorithm::regex).size(), 5); // Match all files having any name in the directory starting with "subdir" followed by an alpha-numerical character (but not from the sub-directories)
}

TEST_F(CRegexPathMatchTest, Extension_Wildcards)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    EXPECT_EQ(CollectWildcardPath(rpathBase, ".*/[^/]+\\.bin", EPathMatchAlgorithm::regex).size(), 8); // Match all files in any directory with any name having the extension "bin"
    EXPECT_EQ(CollectWildcardPath(rpathBase, ".*/[^/]+\\.(?!bin$)[^/]+", EPathMatchAlgorithm::regex).size(), 2); // Match all files in any directory with any name not having the extension "bin"
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir[^/]*/[^/]+\\.bin", EPathMatchAlgorithm::regex).size(), 6); // Match all files in the directory starting with "subdir" followed by any text (but not from the subdirectories) having the extension "bin"
}

TEST_F(CRegexPathMatchTest, NoMatch)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir1/file1", EPathMatchAlgorithm::regex).size(), 0); // Match exactly "subdir1/file1" (which doesn't exist)
    EXPECT_EQ(CollectWildcardPath(rpathBase, "nonexistent/[^/]+", EPathMatchAlgorithm::regex).size(), 0); // Match all files with any name in the directory "nonexistent" (which do not exist)
    EXPECT_EQ(CollectWildcardPath(rpathBase, "[^/]+", EPathMatchAlgorithm::regex).size(), 0); // Match all files with any name in the root directory (which do not exist)
    EXPECT_EQ(CollectWildcardPath(rpathBase, "subdir1/[^/]+\\.(?!bin$).*", EPathMatchAlgorithm::regex).size(), 0); // Match all files in "subdir1" not having the extension "bin" (which do not exist)
}

TEST_F(CRegexPathMatchTest, Navigation_Pattern)
{
    std::filesystem::path rpathBase = GetExecDirectory() / "path_match_test";

    // Current and higher directory in pattern
    EXPECT_EQ(CollectWildcardPath(rpathBase, "\\./.*file50a\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match "file50a.bin" in the "." directory or any of its sub-directories
    EXPECT_EQ(CollectWildcardPath(rpathBase, "\\.\\./.*file50a\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match "file50a.bin" in the ".." directory or any of its sub-directories
    EXPECT_EQ(CollectWildcardPath(rpathBase, "\\.\\./.*/subdir1/file10\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match "subdir1/file10.bin" in any directory following the ".." directory (but not from the subdirectories)
    EXPECT_EQ(CollectWildcardPath(rpathBase, "[^/]+/\\.\\./subdir1/file10\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match "subdir1/file10.bin" in the parent directory ".." of any top directory (but not from the subdirectories)
    EXPECT_EQ(CollectWildcardPath(rpathBase, ".*/\\.\\./subdir5/file50a\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match "subdir5/file50a.bin" in the parent directory ".." of any diretory or subdirectory
    EXPECT_EQ(CollectWildcardPath(rpathBase, ".*/\\.\\./\\.\\./subdir5/file50a\\.bin", EPathMatchAlgorithm::regex).size(), 1); // Match "subdir5/file50a.bin" in the parent directory ".." of the parent directory ".." of any directory or subdirectory
    EXPECT_EQ(CollectWildcardPath(rpathBase, ".*/\\.\\./\\.\\./subdir1/file10\\.bin", EPathMatchAlgorithm::regex).size(), 0); // Match "subdir1/file10.bin" in the parent directory ".." of the parent directory ".." of any directory or subdirectory (which do not exist)
}
