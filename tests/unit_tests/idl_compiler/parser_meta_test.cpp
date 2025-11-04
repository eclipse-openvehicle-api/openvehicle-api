#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/meta_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include <fstream>

using CParserMetaTest = CParserTest;

TEST_F(CParserMetaTest, CheckIncludeLocal)
{
    // Create a dummy file
    std::ofstream fstreamTest("include_test.idl", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamTest.is_open());
    fstreamTest << "const int32 i = 10;";
    fstreamTest.close();

    // Parse the statement
    CParser parser("#include \"include_test.idl\"");
    parser.Parse();
    const CEntityList& rlstMeta = parser.Root()->Get<CRootEntity>()->GetMeta();
    ASSERT_EQ(rlstMeta.size(), 1);

    // Check for the include meta (first entry)
    ASSERT_TRUE(rlstMeta.front() != nullptr);
    CEntityPtr ptrMeta = rlstMeta.front();
    EXPECT_EQ(ptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
    const CMetaEntity* pMeta = ptrMeta->Get<CMetaEntity>();
    ASSERT_TRUE(pMeta != nullptr);
    EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::include_local);
    EXPECT_EQ(pMeta->GetContent(), "\"include_test.idl\"");

    fstreamTest.close();
    try
    {
        std::filesystem::remove("include_test.idl");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CParserMetaTest, CheckIncludeLocalAbsolute)
{
    // Create a dummy file
    std::filesystem::path pathCurrent = std::filesystem::current_path();
    std::ofstream fstreamTest("include_test.idl", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamTest.is_open());
    fstreamTest << "const int32 i = 10;";
    fstreamTest.close();

    // Parse the statement
    std::string ssCode = "#include \"";
    ssCode += (pathCurrent / "include_test.idl").generic_u8string();
    ssCode += "\"";
    CParser parser(ssCode.c_str());
    parser.Parse();
    const CEntityList& rlstMeta = parser.Root()->Get<CRootEntity>()->GetMeta();
    ASSERT_EQ(rlstMeta.size(), 1);

    // Check for the include meta (first entry)
    ASSERT_TRUE(rlstMeta.front() != nullptr);
    CEntityPtr ptrMeta = rlstMeta.front();
    EXPECT_EQ(ptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
    const CMetaEntity* pMeta = ptrMeta->Get<CMetaEntity>();
    ASSERT_TRUE(pMeta != nullptr);
    EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::include_local);
    EXPECT_EQ(pMeta->GetContent(), std::string("\"") + (pathCurrent / "include_test.idl").generic_u8string() + "\"");

    fstreamTest.close();
    try
    {
        std::filesystem::remove("include_test.idl");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CParserMetaTest, CheckIncludeGlobal)
{
    // Create a dummy file
    std::filesystem::create_directory("dummy1");
    std::filesystem::path path = "dummy1/include_test.idl";
    std::ofstream fstreamTest(path.generic_u8string().c_str(), std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamTest.is_open());
    fstreamTest << "const int32 i = 20;";
    fstreamTest.close();

    // Make include dirs (1st argument is the executable file path).
    std::vector<std::string> vecArgs = {"idl_compiler_test.exe", "-Idummy1"};
    CIdlCompilerEnvironment environment(vecArgs);

    // Parse the statement
    CParser parser("#include <include_test.idl>", environment);
    parser.Parse();
    const CEntityList& rlstMeta = parser.Root()->Get<CRootEntity>()->GetMeta();
    ASSERT_EQ(rlstMeta.size(), 1);

    // Check for the include meta (first entry)
    ASSERT_TRUE(rlstMeta.front() != nullptr);
    CEntityPtr ptrMeta = rlstMeta.front();
    EXPECT_EQ(ptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
    const CMetaEntity* pMeta = ptrMeta->Get<CMetaEntity>();
    ASSERT_TRUE(pMeta != nullptr);
    EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::include_global);
    EXPECT_EQ(pMeta->GetContent(), "<include_test.idl>");

    fstreamTest.close();
    try
    {
        std::filesystem::remove(path);
        std::filesystem::remove_all("dummy1");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CParserMetaTest, CheckDefine)
{
    // Parse the statement
    CParser parser("#define def1\n#define def2 10");
    parser.Parse();
    const CEntityList& rlstMeta = parser.Root()->Get<CRootEntity>()->GetMeta();
    ASSERT_EQ(rlstMeta.size(), 2);

    // Check for the 1st define meta
    ASSERT_TRUE(rlstMeta.front() != nullptr);
    CEntityPtr ptrMeta = rlstMeta.front();
    EXPECT_EQ(ptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
    const CMetaEntity* pMeta = ptrMeta->Get<CMetaEntity>();
    ASSERT_TRUE(pMeta != nullptr);
    EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::define);
    EXPECT_EQ(pMeta->GetContent(), "def1");

    // Check for the 2nd define meta
    ASSERT_TRUE(rlstMeta.back() != nullptr);
    ptrMeta = rlstMeta.back();
    EXPECT_EQ(ptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
    pMeta = ptrMeta->Get<CMetaEntity>();
    ASSERT_TRUE(pMeta != nullptr);
    EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::define);
    EXPECT_EQ(pMeta->GetContent(), "def2 10");
}

TEST_F(CParserMetaTest, CheckUndef)
{
    // Parse the statement
    CParser parser("#define def1 10\n#undef def1");
    parser.Parse();
    const CEntityList& rlstMeta = parser.Root()->Get<CRootEntity>()->GetMeta();
    ASSERT_EQ(rlstMeta.size(), 2);

    // Check for the define meta (first entry)
    ASSERT_TRUE(rlstMeta.front() != nullptr);
    CEntityPtr ptrMeta = rlstMeta.front();
    EXPECT_EQ(ptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
    const CMetaEntity* pMeta = ptrMeta->Get<CMetaEntity>();
    ASSERT_TRUE(pMeta != nullptr);
    EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::define);
    EXPECT_EQ(pMeta->GetContent(), "def1 10");

    // Check for the undef meta (second entry)
    ASSERT_TRUE(rlstMeta.back() != nullptr);
    ptrMeta = rlstMeta.back();
    EXPECT_EQ(ptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
    pMeta = ptrMeta->Get<CMetaEntity>();
    ASSERT_TRUE(pMeta != nullptr);
    EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::undef);
    EXPECT_EQ(pMeta->GetContent(), "def1");
}

TEST_F(CParserMetaTest, CheckIncludeLocalDefineCombi)
{
    // Create a dummy file
    std::ofstream fstreamTest("include_test.idl", std::ios_base::out | std::ios_base::trunc);
    ASSERT_TRUE(fstreamTest.is_open());
    fstreamTest << "#define HELLO";
    fstreamTest.close();

    // Parse the statement
    CParser parser("#include \"include_test.idl\"");
    parser.Parse();
    const CEntityList& rlstMeta = parser.Root()->Get<CRootEntity>()->GetMeta();
    ASSERT_EQ(rlstMeta.size(), 2);

    // Check for the include meta (first entry)
    ASSERT_TRUE(rlstMeta.front() != nullptr);
    CEntityPtr ptrMeta = rlstMeta.front();
    EXPECT_EQ(ptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
    const CMetaEntity* pMeta = ptrMeta->Get<CMetaEntity>();
    ASSERT_TRUE(pMeta != nullptr);
    EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::include_local);
    EXPECT_EQ(pMeta->GetContent(), "\"include_test.idl\"");

    // Check for the define meta (second entry)
    ASSERT_TRUE(rlstMeta.back() != nullptr);
    ptrMeta = rlstMeta.back();
    EXPECT_EQ(ptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
    pMeta = ptrMeta->Get<CMetaEntity>();
    ASSERT_TRUE(pMeta != nullptr);
    EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::define);
    EXPECT_EQ(pMeta->GetContent(), "HELLO");

    fstreamTest.close();
    try
    {
        std::filesystem::remove("include_test.idl");
    } catch (const std::filesystem::filesystem_error&)
    {}
}

TEST_F(CParserMetaTest, CheckVerbatim)
{
    // Parse the statement
    CParser parser(
        "#verbatim const int32 i = 10\n"
        "#verbatim const int32 j = 10\n"
        "#verbatim const int32 k = 10\\\n"
        "const int32 l = 10\n"
        "// Definition of M\n"
        "#verbatim #define M\n"
        "#verbatim #include <string>");
    parser.Parse();
    const CEntityList& rlstMeta = parser.Root()->Get<CRootEntity>()->GetMeta();
    ASSERT_EQ(rlstMeta.size(), 5);

    size_t nIndex = 0;
    for (const CEntityPtr& rptrMeta : rlstMeta)
    {
        EXPECT_EQ(rptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
        const CMetaEntity* pMeta = rptrMeta->Get<CMetaEntity>();
        ASSERT_TRUE(pMeta != nullptr);
        EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::verbatim);
        switch (nIndex)
        {
        case 0:
            EXPECT_EQ(pMeta->GetContent(), "const int32 i = 10");
            break;
        case 1:
            EXPECT_EQ(pMeta->GetContent(), "const int32 j = 10");
            break;
        case 2:
            EXPECT_EQ(pMeta->GetContent(), "const int32 k = 10\\\nconst int32 l = 10");
            break;
        case 3:
        {
            EXPECT_EQ(pMeta->GetContent(), "#define M");
            uint32_t uiFlags = 0;
            EXPECT_EQ(pMeta->GetComments(uiFlags), "Definition of M");
            break;
        }
        case 4:
            EXPECT_EQ(pMeta->GetContent(), "#include <string>");
            break;
        }
        nIndex++;
    }
}

TEST_F(CParserMetaTest, CheckVerbatimBlock)
{
    // Parse the statement
    CParser parser(
        "#verbatim_begin\n"
        "const int32 i = 10\n"
        "const int32 j = 10\n"
        "const int32 k = 10\\\n"
        "const int32 l = 10\n"
        "// Definition of M\n"
        "#define M\n"
        "#include <string>\n"
        "#verbatim_end\n"
        "#define N\n");

    parser.Parse();
    const CEntityList& rlstMeta = parser.Root()->Get<CRootEntity>()->GetMeta();
    ASSERT_EQ(rlstMeta.size(), 2);

    size_t nIndex = 0;
    for (const CEntityPtr& rptrMeta : rlstMeta)
    {
        EXPECT_EQ(rptrMeta->GetType(), sdv::idl::EEntityType::type_meta);
        const CMetaEntity* pMeta = rptrMeta->Get<CMetaEntity>();
        ASSERT_TRUE(pMeta != nullptr);
        switch (nIndex)
        {
        case 0:
            EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::verbatim);
            EXPECT_EQ(pMeta->GetContent(),
                "const int32 i = 10\n"
                "const int32 j = 10\n"
                "const int32 k = 10\\\n"
                "const int32 l = 10\n"
                "// Definition of M\n"
                "#define M\n"
                "#include <string>\n" );
            break;
        case 1:
            EXPECT_EQ(pMeta->GetMetaType(), sdv::idl::IMetaEntity::EType::define);
            EXPECT_EQ(pMeta->GetContent(), "N");
            break;
        }
        nIndex++;
    }
}

