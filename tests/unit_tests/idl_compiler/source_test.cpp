#include "includes.h"
#include "lexer_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/source.cpp"

using CSourceTest = CLexerTest;

TEST_F(CSourceTest, SourceCodeDirect)
{
    // Open a sourcefile without code
    EXPECT_THROW(CSource(nullptr), CCompileException);
    EXPECT_NO_THROW(CSource(""));
}

TEST_F(CSourceTest, SourceFileOpenNonExisting)
{
    // Open non existing files
    EXPECT_THROW(CSource(std::filesystem::path("DummyFile")), CCompileException);
}

TEST_F(CSourceTest, SourceFileReadAnsi)
{
    // Create a dummy file
    std::ofstream fstream("dummy_source_test_ansi.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstream.is_open());
    fstream << "Hello, this is a text!";
    fstream.close();

    CSource source(std::filesystem::path("dummy_source_test_ansi.tmp"));
    EXPECT_EQ(source.GetCodeRef(), "Hello, this is a text!");

    try
    {
        std::filesystem::remove("dummy_source_test_ansi.tmp");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CSourceTest, SourceFileReadUTF8)
{
    // Create a dummy file
    std::ofstream fstream("dummy_source_test_utf8.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstream.is_open());
    fstream.put(static_cast<char>(0xEF));
	fstream.put(static_cast<char>(0xBB));
    fstream.put(static_cast<char>(0xBF));
    fstream << "Hello, this is a text!";
    fstream.close();

    CSource source(std::filesystem::path("dummy_source_test_utf8.tmp"));
    EXPECT_EQ(source.GetCodeRef(), "Hello, this is a text!");

    try
    {
        std::filesystem::remove("dummy_source_test_utf8.tmp");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CSourceTest, SourceFileReadUTF16LE)
{
    // Create a dummy file
    std::ofstream fstream("dummy_source_test_utf16le.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstream.is_open());
	fstream.put(static_cast<char>(0xFF));
	fstream.put(static_cast<char>(0xFE));
    char16_t szText[] = u"Hello, this is a text!";
    for (char16_t c16 : szText)
    {
        if (!c16) break;
        char* rgc = reinterpret_cast<char*>(&c16);
        fstream.put(rgc[0]);
        fstream.put(rgc[1]);
    }
    fstream.close();

    CSource source(std::filesystem::path("dummy_source_test_utf16le.tmp"));
    EXPECT_EQ(source.GetCodeRef(), "Hello, this is a text!");

    try
    {
        std::filesystem::remove("dummy_source_test_utf16le.tmp");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CSourceTest, SourceFileReadUTF16BE)
{
    // Create a dummy file
    std::ofstream fstream("dummy_source_test_utf16be.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstream.is_open());
    fstream.put(static_cast<char>(0xFE));
    fstream.put(static_cast<char>(0xFF));
    char16_t szText[] = u"Hello, this is a text!";
    for (char16_t c16 : szText)
    {
        if (!c16) break;
        char* rgc = reinterpret_cast<char*>(&c16);
        fstream.put(rgc[1]);
        fstream.put(rgc[0]);
    }
    fstream.close();

    CSource source(std::filesystem::path("dummy_source_test_utf16be.tmp"));
    EXPECT_EQ(source.GetCodeRef(), "Hello, this is a text!");

    try
    {
        std::filesystem::remove("dummy_source_test_utf16be.tmp");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CSourceTest, SourceFileReadUTF32LE)
{
    // Create a dummy file
    std::ofstream fstream("dummy_source_test_utf32le.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstream.is_open());
    fstream.put(static_cast<char>(0xFF));
    fstream.put(static_cast<char>(0xFE));
    fstream.put(0x00);
    fstream.put(0x00);
    char32_t szText[] = U"Hello, this is a text!";
    for (char32_t c32 : szText)
    {
        if (!c32) break;
        char* rgc = reinterpret_cast<char*>(&c32);
        fstream.put(rgc[0]);
        fstream.put(rgc[1]);
        fstream.put(rgc[2]);
        fstream.put(rgc[3]);
    }
    fstream.close();

    CSource source(std::filesystem::path("dummy_source_test_utf32le.tmp"));
    EXPECT_EQ(source.GetCodeRef(), "Hello, this is a text!");

    try
    {
        std::filesystem::remove("dummy_source_test_utf32le.tmp");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CSourceTest, SourceFileReadUTF32BE)
{
    // Create a dummy file
    std::ofstream fstream("dummy_source_test_utf32be.tmp", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstream.is_open());
    fstream.put(0x00);
    fstream.put(0x00);
    fstream.put(static_cast<char>(0xFE));
    fstream.put(static_cast<char>(0xFF));
    char32_t szText[] = U"Hello, this is a text!";
    for (char32_t c32 : szText)
    {
        if (!c32) break;
        char* rgc = reinterpret_cast<char*>(&c32);
        fstream.put(rgc[3]);
        fstream.put(rgc[2]);
        fstream.put(rgc[1]);
        fstream.put(rgc[0]);
    }
    fstream.close();

    CSource source(std::filesystem::path("dummy_source_test_utf32be.tmp"));
    EXPECT_EQ(source.GetCodeRef(), "Hello, this is a text!");

    try
    {
        std::filesystem::remove("dummy_source_test_utf32be.tmp");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

