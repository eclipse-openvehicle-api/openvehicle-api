#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/parsecontext.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/entity_base.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/entity_value.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/root_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/entities/hash_calc.cpp"

void CParserTest::SetUpTestCase()
{
    ASSERT_TRUE(true);
}

void CParserTest::TearDownTestCase()
{}

void CParserTest::SetUp()
{}

void CParserTest::TearDown()
{}

TEST_F(CParserTest, Instantiate)
{
    // Instantiate parser without code
    EXPECT_THROW(CParser(nullptr).Parse(), CCompileException);
}

TEST_F(CParserTest, ParsingEmptyString)
{
    // Parse empty string
    EXPECT_NO_THROW(CParser("").Parse());
}

TEST_F(CParserTest, ParsingFindNonExistent)
{
    EXPECT_FALSE(CParser("").Parse().Root()->Find("Test"));
}

TEST_F(CParserTest, ParsingPreceedingCComments)
{
    // Standard C-style comment
    const char* szCode1 = R"code(
    /* Comments */
    struct S;
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style));

    // Multi-line C-style comments
    const char* szCode2 = R"code(
    /* Comments
       More comments
    */
    struct S;
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style));

    // C-style comments
    const char* szCode6 = R"code(
    /*Comments*/
    struct S;
)code";
    CParser parser6(szCode6);
    ptrEntity = parser6.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style));

    // Same line comments
    const char* szCode7 = R"code(
    /* Comments */ struct S;
)code";
    CParser parser7(szCode7);
    ptrEntity = parser7.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style));

    // Multiple separate comment blocks
    const char* szCode8 = R"code(
    /* Not included comments */

    /* Included comments */
    struct S;
)code";
    CParser parser8(szCode8);
    ptrEntity = parser8.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Included comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style));

    // Multiple adjacent comment blocks
    const char* szCode9 = R"code(
    /* Comments */
    /* More comments */ struct S;
)code";
    CParser parser9(szCode9);
    ptrEntity = parser9.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style));
}

TEST_F(CParserTest, ParsingPreceedingCJavaDocComments)
{
	// Illformed Javadoc comment
    const char* szCode3 = R"code(
    /** Comments **/
    struct S;
)code";
    CParser parser3(szCode3);
    uint32_t uiFlags = 0;
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments *");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));

    // Javadoc comment
    const char* szCode4 = R"code(
    /**
     * Comments
     */
    struct S;
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));

    // Javadoc multi-line comment
    const char* szCode5 = R"code(
    /**
     * Comments
     * More comments
     */
    struct S;
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));

    // Multiple lines javadoc comments on same line
    const char* szCode10 = R"code(
    /** Comments
     * More comments */ struct S;
)code";
    CParser parser10(szCode10);
    ptrEntity = parser10.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));

}

TEST_F(CParserTest, ParsingPreceedingCQTComments)
{
    // Illformed QT comment
    const char* szCode3 = R"code(
    /*! Comments **/
    struct S;
)code";
    CParser parser3(szCode3);
    uint32_t uiFlags = 0;
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments *");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt));

    // QT comment
	const char* szCode4 = R"code(
    /*!
     * Comments
     */
    struct S;
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt));

    // QT multi-line comment
	const char* szCode5 = R"code(
    /*!
     * Comments
     * More comments
     */
    struct S;
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt));

    // Multiple lines QT comments on same line
	const char* szCode10 = R"code(
    /*! Comments
     * More comments */ struct S;
)code";
    CParser parser10(szCode10);
    ptrEntity = parser10.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt));

}

TEST_F(CParserTest, ParsingSucceedingCComments)
{
    // Standard C-style comment
	const char* szCode1 = R"code(
    struct S;       /* Comments */
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multi-line C-style comments
	const char* szCode2 = R"code(
    struct S;      /* Comments
                      More comments
                   */
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // C-style comments
	const char* szCode6 = R"code(
    struct S;   /*Comments*/
)code";
    CParser parser6(szCode6);
    ptrEntity = parser6.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Same line comments
	const char* szCode7 = R"code(
    struct S;/* Comments */
)code";
    CParser parser7(szCode7);
    ptrEntity = parser7.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple separate comment blocks
	const char* szCode8 = R"code(
    struct S; /* Included comments */

    /* Not included comments */
)code";
    CParser parser8(szCode8);
    ptrEntity = parser8.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Included comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple adjacent comment blocks
	const char* szCode9 = R"code(
    struct S;   /* Comments */
                /* More comments */
)code";
    CParser parser9(szCode9);
    ptrEntity = parser9.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingSucceedingCJavaDocComments)
{
    // Illformed Javadoc comment
	const char* szCode3 = R"code(
    struct S;   /** Comments **/
)code";
    CParser parser3(szCode3);
    uint32_t uiFlags = 0;
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments *");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Javadoc comment
	const char* szCode4 = R"code(
    struct S;       /**
                     * Comments
                     */
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Javadoc multi-line comment
	const char* szCode5 = R"code(
    struct S;       /**
                     * Comments
                     * More comments
                     */
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple lines javadoc comments on same line
	const char* szCode10 = R"code(
    struct S;       /**< Comments
                    * More comments */
)code";
    CParser parser10(szCode10);
    ptrEntity = parser10.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingSucceedingCQTComments)
{
    // Illformed QT comment
	const char* szCode3 = R"code(
    struct S;   /*! Comments **/
)code";
    CParser parser3(szCode3);
    uint32_t uiFlags = 0;
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments *");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // QT comment
	const char* szCode4 = R"code(
    struct S;       /*!
                     * Comments
                     */
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // QT multi-line comment
	const char* szCode5 = R"code(
    struct S;       /*!
                     * Comments
                     * More comments
                     */
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple lines QT comments on same line
	const char* szCode10 = R"code(
    struct S;       /*!< Comments
                    * More comments */
)code";
    CParser parser10(szCode10);
    ptrEntity = parser10.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingCompatitivePreAndSucceedingCComments)
{
    // Standard C-style comment
	const char* szCode1 = R"code(
    /* Precomments */
    struct S;       /* Postcomments */
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Postcomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multi-line C-style comments
	const char* szCode2 = R"code(
    /* Precomments
       More precomments
    */
    struct S;      /* Postcomments
                      More postcomments
                   */
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Postcomments\nMore postcomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
    // C-style comments
	const char* szCode6 = R"code(
    /*Precomments*/
    struct S;   /*Postcomments*/
)code";
    CParser parser6(szCode6);
    ptrEntity = parser6.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Postcomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Same line comments
	const char* szCode7 = R"code(
    /* Precomments */ struct S;/* Postcomments */
)code";
    CParser parser7(szCode7);
    ptrEntity = parser7.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Postcomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple separate comment blocks
	const char* szCode8 = R"code(
    /* Not included precomments */

    /* Included precomments */
    struct S; /* Included postcomments */

    /* Not included postcomments */
)code";
    CParser parser8(szCode8);
    ptrEntity = parser8.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Included postcomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple adjacent comment blocks
	const char* szCode9 = R"code(
    /* Precomments */
    /* More precomments */ struct S;   /* Postcomments */
                /* More postcomments */
)code";
    CParser parser9(szCode9);
    ptrEntity = parser9.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Postcomments\nMore postcomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingCompatitivePreAndSucceedingCJavaDocComments)
{
    // Illformed Javadoc comment
	const char* szCode3 = R"code(
    /** Precomments **/
    struct S;   /** Postcomments **/
)code";
    CParser parser3(szCode3);
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments *");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Javadoc comment
	const char* szCode4 = R"code(
    /**
     * Precomments
     */
    struct S;       /**
                     * Postcomments
                     */
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Javadoc multi-line comment
	const char* szCode5 = R"code(
    /**
     * Precomments
     * More precomments
     */
    struct S;       /**
                     * Postcomments
                     * More postcomments
                     */
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments\nMore precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple lines javadoc comments on same line
	const char* szCode10 = R"code(
    /** Precomments
     * More precomments */ struct S;       /**!< Postcomments
                    * More postcomments */
)code";
    CParser parser10(szCode10);
    ptrEntity = parser10.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments\nMore precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingCompatitivePreAndSucceedingCQTComments)
{
    // Illformed QT comment
	const char* szCode3 = R"code(
    /*! Precomments **/
    struct S;   /*! Postcomments **/
)code";
    CParser parser3(szCode3);
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments *");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // QT comment
	const char* szCode4 = R"code(
    /*!
     * Precomments
     */
    struct S;       /*!
                     * Postcomments
                     */
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // QT multi-line comment
	const char* szCode5 = R"code(
    /*!
     * Precomments
     * More precomments
     */
    struct S;       /*!
                     * Postcomments
                     * More postcomments
                     */
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments\nMore precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple lines QT comments on same line
	const char* szCode10 = R"code(
    /*! Precomments
     * More precomments */ struct S;       /*!< Postcomments
                    * More postcomments */
)code";
    CParser parser10(szCode10);
    ptrEntity = parser10.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments\nMore precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingExtendingPreAndSucceedingCComments)
{
    // Standard C-style comment
	const char* szCode1 = R"code(
    /* Precomments */
    const struct SDef {} sDec = {};       /* Postcomments */
)code";
    CParser parser1(szCode1);
    parser1.Parse();
    CEntityPtr ptrEntityDef = parser1.Root()->Find("SDef");
    uint32_t uiFlagsDef = 0;
    std::string ssCommentsDef = ptrEntityDef->GetComments(uiFlagsDef);
    CEntityPtr ptrEntityDec = parser1.Root()->Find("sDec");
    uint32_t uiFlagsDec = 0;
    std::string ssCommentsDec = ptrEntityDec->GetComments(uiFlagsDec);
    EXPECT_EQ(ssCommentsDef, "");
    EXPECT_EQ(uiFlagsDef, 0u);
    EXPECT_EQ(ssCommentsDec, "Precomments");
    EXPECT_EQ(uiFlagsDec, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style));
}

TEST_F(CParserTest, ParsingExtendingPreAndSucceedingCCommentsTypedef)
{
    // Standard C-style comment
	const char* szCode1 = R"code(
    /* Precomments */
    typedef struct tagTDec {} TDec;       /* Postcomments */
)code";
    CParser parser1(szCode1);
    parser1.Parse();
    CEntityPtr ptrEntityDec = parser1.Root()->Find("TDec");
    uint32_t uiFlagsDec = 0;
    std::string ssCommentsDec = ptrEntityDec->GetComments(uiFlagsDec);
    EXPECT_EQ(ssCommentsDec, "Precomments");
    EXPECT_EQ(uiFlagsDec, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style));
}

TEST_F(CParserTest, ParsingPreceedingCppComments)
{
    // Standard C++-style comment
	const char* szCode1 = R"code(
    // Comments
    struct S;
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style));

    // Multi-line C-style comments
	const char* szCode2 = R"code(
    // Comments
    // More comments
    struct S;
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style));

    // C-style comments
    const char* szCode6 = R"code(
    //Comments
    struct S;
)code";
    CParser parser6(szCode6);
    ptrEntity = parser6.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style));

    // Multiple separate comment blocks
    const char* szCode8 = R"code(
    // Not included comments

    // Included comments
    struct S;
)code";
    CParser parser8(szCode8);
    ptrEntity = parser8.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Included comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style));
}

TEST_F(CParserTest, ParsingPreceedingCppJavaDocComments)
{
    // Javadoc comment
    const char* szCode3 = R"code(
    /// Comments
    struct S;
)code";
    CParser parser3(szCode3);
    uint32_t uiFlags = 0;
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));

    // Javadoc comment
    const char* szCode4 = R"code(
    ///
    /// Comments
    ///
    struct S;
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));

    // Javadoc multi-line comment
    const char* szCode5 = R"code(
    /// Comments
    /// More comments
    struct S;
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));
}

TEST_F(CParserTest, ParsingPreceedingCppQTComments)
{
    // QT comment
    const char* szCode3 = R"code(
    //! Comments
    struct S;
)code";
    CParser parser3(szCode3);
    uint32_t uiFlags = 0;
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt));

    // QT comment
    const char* szCode4 = R"code(
    //!
    //! Comments
    //!
    struct S;
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt));

    // QT multi-line comment
    const char* szCode5 = R"code(
    //!
    //! Comments
    //! More comments
    //!
    struct S;
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt));
}

TEST_F(CParserTest, ParsingSucceedingCppComments)
{
    // Standard C++-style comment
    const char* szCode1 = R"code(
    struct S;       // Comments
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multi-line C-style comments
    const char* szCode2 = R"code(
    struct S;      // Comments
                   // More comments
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // C-style comments
    const char* szCode6 = R"code(
    struct S;   //Comments
)code";
    CParser parser6(szCode6);
    ptrEntity = parser6.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Same line comments
    const char* szCode7 = R"code(
    struct S;// Comments
)code";
    CParser parser7(szCode7);
    ptrEntity = parser7.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple separate comment blocks
    const char* szCode8 = R"code(
    struct S; // Included comments

    // Not included comments
)code";
    CParser parser8(szCode8);
    ptrEntity = parser8.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Included comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingSucceedingCppJavaDocComments)
{
    // Javadoc comment
    const char* szCode3 = R"code(
    struct S;   /// Comments
)code";
    CParser parser3(szCode3);
    uint32_t uiFlags = 0;
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Javadoc comment
    const char* szCode4 = R"code(
    struct S;       ///<
                    ///< Comments
                    ///<
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Javadoc multi-line comment
    const char* szCode5 = R"code(
    struct S;       /// Comments
                    /// More comments
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple lines javadoc comments on same line
    const char* szCode10 = R"code(
    struct S;       ///< Comments
                    ///< More comments
)code";
    CParser parser10(szCode10);
    ptrEntity = parser10.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingSucceedingCppQTComments)
{
    // QT comment
    const char* szCode3 = R"code(
    struct S;   //! Comments
)code";
    CParser parser3(szCode3);
    uint32_t uiFlags = 0;
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // QT comment
    const char* szCode4 = R"code(
    struct S;       //!
                    //! Comments
                    //!
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // QT multi-line comment
    const char* szCode5 = R"code(
    struct S;       //! Comments
                    //! More comments
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple lines QT comments on same line
    const char* szCode10 = R"code(
    struct S;       //!< Comments
                    //!< More comments
)code";
    CParser parser10(szCode10);
    ptrEntity = parser10.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Comments\nMore comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingCompatitivePreAndSucceedingCppComments)
{
    // Standard C++-style comment
    const char* szCode1 = R"code(
    // Precomments
    struct S;       // Postcomments
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Postcomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multi-line C-style comments
    const char* szCode2 = R"code(
    // Precomments
    // More precomments
    struct S;      // Postcomments
                   // More postcomments
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Postcomments\nMore postcomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
    // C-style comments
    const char* szCode6 = R"code(
    //Precomments
    struct S;   //Postcomments
)code";
    CParser parser6(szCode6);
    ptrEntity = parser6.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Postcomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Multiple separate comment blocks
    const char* szCode8 = R"code(
    // Not included precomments

    // Included precomments
    struct S; // Included postcomments

    // Not included postcomments
)code";
    CParser parser8(szCode8);
    ptrEntity = parser8.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Included postcomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingCompatitivePreAndSucceedingCppJavaDocComments)
{
    // Javadoc comment
    const char* szCode3 = R"code(
    /// Precomments
    struct S;   /// Postcomments
)code";
    CParser parser3(szCode3);
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Javadoc comment
    const char* szCode4 = R"code(
    ///
    /// Precomments
    ///
    struct S;       ///
                    /// Postcomments
                    ///
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // Javadoc multi-line comment
    const char* szCode5 = R"code(
    /// Precomments
    /// More precomments
    struct S;       ///< Postcomments
                    ///< More postcomments
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments\nMore precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingCompatitivePreAndSucceedingCppQTComments)
{
    // QT comment
    const char* szCode3 = R"code(
    //! Precomments
    struct S;   //! Postcomments
)code";
    CParser parser3(szCode3);
    CEntityPtr ptrEntity = parser3.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // QT comment
    const char* szCode4 = R"code(
    //!
    //! Precomments
    //!
    struct S;       //!
                    //! Postcomments
                    //!
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // QT multi-line comment
    const char* szCode5 = R"code(
    //! Precomments
    //! More precomments
    struct S;       //!< Postcomments
                    //!< More postcomments
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "Precomments\nMore precomments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}

TEST_F(CParserTest, ParsingExtendingPreAndSucceedingCppComments)
{
    // Standard C-style comment
    const char* szCode1 = R"code(
    // Precomments
    const struct SDef {} sDec = {};       // Postcomments
)code";
    CParser parser1(szCode1);
    parser1.Parse();
    CEntityPtr ptrEntityDef = parser1.Root()->Find("SDef");
    uint32_t uiFlagsDef = 0;
    std::string ssCommentsDef = ptrEntityDef->GetComments(uiFlagsDef);
    CEntityPtr ptrEntityDec = parser1.Root()->Find("sDec");
    uint32_t uiFlagsDec = 0;
    std::string ssCommentsDec = ptrEntityDec->GetComments(uiFlagsDec);
    EXPECT_EQ(ssCommentsDef, "");
    EXPECT_EQ(uiFlagsDef, 0u);
    EXPECT_EQ(ssCommentsDec, "Precomments");
    EXPECT_EQ(uiFlagsDec, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style));
}

TEST_F(CParserTest, ParsingExtendingPreAndSucceedingCppCommentsTypedef)
{
    // Standard C-style comment
    const char* szCode1 = R"code(
    // Precomments
    typedef struct tagTDec {} TDec;       // Postcomments
)code";
    CParser parser1(szCode1);
    parser1.Parse();
    CEntityPtr ptrEntityDec = parser1.Root()->Find("TDec");
    uint32_t uiFlagsDec = 0;
    std::string ssCommentsDec = ptrEntityDec->GetComments(uiFlagsDec);
    EXPECT_EQ(ssCommentsDec, "Precomments");
    EXPECT_EQ(uiFlagsDec, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style));
}

TEST_F(CParserTest, ParsingMixedFormatCPreceedingComments)
{
    // C and JavaDoc comments
    const char* szCode1 = R"code(
    /* C-style comments */
    /** JavaDoc comments */
    struct S;
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C-style comments\nJavaDoc comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));

    // Javadoc and C comments
    const char* szCode2 = R"code(
    /** JavaDoc comments */
    /* C-style comments */
    struct S;
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "JavaDoc comments\nC-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));

    // C and QT comments
    const char* szCode3 = R"code(
    /* C-style comments */
    /*! QT comments */
    struct S;
)code";
    CParser parser3(szCode3);
    ptrEntity = parser3.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C-style comments\nQT comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt));

    // QT and C comments
    const char* szCode4 = R"code(
    /*! QT comments */
    /* C-style comments */
    struct S;
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "QT comments\nC-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt));

    // JavaDoc and QT comments
    const char* szCode5 = R"code(
    /** JavaDoc comments */
    /*! QT comments */
    struct S;
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "JavaDoc comments\nQT comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));

    // QT and JavaDoc comments
    const char* szCode6 = R"code(
    /*! QT comments */
    /** JavaDoc comments */
    struct S;
)code";
    CParser parser6(szCode6);
    ptrEntity = parser6.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "QT comments\nJavaDoc comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));
}

TEST_F(CParserTest, ParsingMixedFormatCSucceedingComments)
{
    // C and JavaDoc comments
    const char* szCode1 = R"code(
    struct S; /* C-style comments */
    /** JavaDoc comments */
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C-style comments\nJavaDoc comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));

    // Javadoc and C comments
    const char* szCode2 = R"code(
    struct S; /** JavaDoc comments */
    /* C-style comments */
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "JavaDoc comments\nC-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));

    // C and QT comments
    const char* szCode3 = R"code(
    struct S; /* C-style comments */
    /*! QT comments */
)code";
    CParser parser3(szCode3);
    ptrEntity = parser3.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C-style comments\nQT comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt));

    // QT and C comments
    const char* szCode4 = R"code(
    struct S; /*! QT comments */
    /* C-style comments */
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "QT comments\nC-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_qt));

    // JavaDoc and QT comments
    const char* szCode5 = R"code(
    struct S; /** JavaDoc comments */
    /*! QT comments */
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "JavaDoc comments\nQT comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));

    // QT and JavaDoc comments
    const char* szCode6 = R"code(
    struct S; /*! QT comments */
    /** JavaDoc comments */
)code";
    CParser parser6(szCode6);
    ptrEntity = parser6.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "QT comments\nJavaDoc comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style_javadoc));
}

TEST_F(CParserTest, ParsingMixedFormatCppPreceedingComments)
{
    // C and JavaDoc comments
    const char* szCode1 = R"code(
    // C++-style comments
    /// JavaDoc comments
    struct S;
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C++-style comments\nJavaDoc comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));

    // Javadoc and C comments
    const char* szCode2 = R"code(
    /// JavaDoc comments
    // C++-style comments
    struct S;
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "JavaDoc comments\nC++-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));

    // C and QT comments
    const char* szCode3 = R"code(
    // C++-style comments
    //! QT comments
    struct S;
)code";
    CParser parser3(szCode3);
    ptrEntity = parser3.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C++-style comments\nQT comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt));

    // QT and C comments
    const char* szCode4 = R"code(
    //! QT comments
    // C++-style comments
    struct S;
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "QT comments\nC++-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt));

    // JavaDoc and QT comments
    const char* szCode5 = R"code(
    /// JavaDoc comments
    //! QT comments
    struct S;
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "JavaDoc comments\nQT comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));

    // QT and JavaDoc comments
    const char* szCode6 = R"code(
    //! QT comments
    /// JavaDoc comments
    struct S;
)code";
    CParser parser6(szCode6);
    ptrEntity = parser6.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "QT comments\nJavaDoc comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));
}

TEST_F(CParserTest, ParsingMixedFormatCppSucceedingComments)
{
    // C and JavaDoc comments
    const char* szCode1 = R"code(
    struct S; // C++-style comments
    /// JavaDoc comments
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C++-style comments\nJavaDoc comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));

    // Javadoc and C comments
    const char* szCode2 = R"code(
    struct S; /// JavaDoc comments
    // C++-style comments
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "JavaDoc comments\nC++-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));

    // C and QT comments
    const char* szCode3 = R"code(
    struct S; // C++-style comments
    //! QT comments
)code";
    CParser parser3(szCode3);
    ptrEntity = parser3.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C++-style comments\nQT comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt));

    // QT and C comments
    const char* szCode4 = R"code(
    struct S; //! QT comments
    // C++-style comments
)code";
    CParser parser4(szCode4);
    ptrEntity = parser4.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "QT comments\nC++-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_qt));

    // JavaDoc and QT comments
    const char* szCode5 = R"code(
    struct S; /// JavaDoc comments
    //! QT comments
)code";
    CParser parser5(szCode5);
    ptrEntity = parser5.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "JavaDoc comments\nQT comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));

    // QT and JavaDoc comments
    const char* szCode6 = R"code(
    struct S; //! QT comments
    /// JavaDoc comments
)code";
    CParser parser6(szCode6);
    ptrEntity = parser6.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "QT comments\nJavaDoc comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::cpp_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::format_javadoc));
}

TEST_F(CParserTest, ParsingMixedCAndCppPreceedingComments)
{
    // C and C++ comments
    const char* szCode1 = R"code(
    /* C-style comments */
    // C++-style comments
    struct S;
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C-style comments\nC++-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style));

    // C++ and C comments
    const char* szCode2 = R"code(
    // C++-style comments
    /* C-style comments */
    struct S;
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C++-style comments\nC-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style));
}

TEST_F(CParserTest, ParsingMixedCAndCppSucceedingComments)
{
    // C and C++ comments
    const char* szCode1 = R"code(
    struct S; /* C-style comments */
    // C++-style comments
)code";
    CParser parser1(szCode1);
    CEntityPtr ptrEntity = parser1.Parse().Root()->Find("S");
    uint32_t uiFlags = 0;
    std::string ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C-style comments\nC++-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));

    // C++ and C comments
    const char* szCode2 = R"code(
    struct S; // C++-style comments
    /* C-style comments */
)code";
    CParser parser2(szCode2);
    ptrEntity = parser2.Parse().Root()->Find("S");
    uiFlags = 0;
    ssComments = ptrEntity->GetComments(uiFlags);
    EXPECT_EQ(ssComments, "C++-style comments\nC-style comments");
    EXPECT_EQ(uiFlags, static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::c_style) |
        static_cast<uint32_t>(sdv::idl::IEntityComments::ECommentMask::loc_succeeding));
}
