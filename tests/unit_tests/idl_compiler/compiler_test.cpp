#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"
#include "../../../sdv_executables/sdv_idl_compiler/generator/context.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/generator/ps_class_generator_base.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/generator/definition_generator.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/generator/proxy_generator.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/generator/stub_generator.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/generator/serdes_generator.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/generator/cmake_generator.cpp"

/**
 * @brief IDL file management class.
*/
class CIdlFile
{
public:
    /**
     * @brief Constructor
     * @param[in] rssIdlCode Reference to the string containing the code to store in the IDL file.
     */
    CIdlFile(const std::string& rssIdlCode) : m_pathIdl(std::filesystem::current_path() / "test.idl")
    {
        // Start clean
        CleanUp();

        // Create a stream and write the code.
        std::ofstream fstreamIdlFile(m_pathIdl.c_str());
        fstreamIdlFile << rssIdlCode;
        fstreamIdlFile.close();
    }

    /**
     * @brief Destructor
     */
    ~CIdlFile()
    {
        CleanUp();
    }

    /**
     * @brief Get the full path to the IDL file.
     * @return The path.
     */
    std::filesystem::path PathIdl() const { return m_pathIdl; }

    /**
     * @brief Get the header file content (if one was created).
     * @return The string containing the header file content.
     */
    std::string Header() const
    {
        std::filesystem::path pathHdr = m_pathIdl;
        pathHdr.replace_extension(".h");
        std::ifstream fstream(pathHdr.c_str());
        std::stringstream sstreamContent;
        sstreamContent << fstream.rdbuf();
        return sstreamContent.str();
    }

private:
    /**
     * @brief Clean up before and after the compilation.
     */
    void CleanUp()
    {
        try
        {
            if (std::filesystem::exists(m_pathIdl))
                std::filesystem::remove(m_pathIdl);
        } catch (const std::filesystem::filesystem_error&)
        {}
        std::filesystem::path pathHdr = m_pathIdl;
        pathHdr.replace_extension(".h");
        try
        {
            if (std::filesystem::exists(pathHdr))
                std::filesystem::remove(pathHdr);
        } catch (const std::filesystem::filesystem_error&)
        {}
        std::filesystem::path pathPSDir = m_pathIdl;
        pathPSDir.remove_filename();
        pathPSDir /= "ps";
        if (std::filesystem::exists(pathPSDir))
        {
            try
            {
                std::filesystem::remove_all(pathPSDir);
            } catch (const std::filesystem::filesystem_error&)
            {}
        }
        std::filesystem::path pathSerDesDir = m_pathIdl;
        pathSerDesDir.remove_filename();
        pathSerDesDir /= "serdes";
        if (std::filesystem::exists(pathSerDesDir))
        {
            try
            {
                std::filesystem::remove_all(pathSerDesDir);
            } catch (const std::filesystem::filesystem_error&)
            {}
        }
    }

    std::filesystem::path       m_pathIdl;      ///< Path to the IDL file
};

/**
 * @brief Class managing a test compilation.
 */
class CTestCompile : public CIdlFile
{
public:
    /**
     * @brief Constructor triggering a test compilation using the provided code.
     * @param[in] rssIdlCode Reference to the IDL code to compile.
     */
    CTestCompile(const std::string& rssIdlCode) : CIdlFile(rssIdlCode)
    {
        CIdlCompilerEnvironment environment;
        try
        {
            // Parse file
            CParser parser(PathIdl(), environment);
            parser.Parse();

            // Generate definition
            CDefinitionGenerator defgen(&parser);
            defgen.Generate();

            // Proxy code
            CProxyGenerator proxygen(&parser);
            proxygen.Generate();

            // Stub code
            CStubGenerator stubgen(&parser);
            stubgen.Generate();

            // Serdes code
            CSerdesGenerator serdesgen(&parser);
            serdesgen.Generate();

            // CMake code generation
            CIdlCompilerCMakeGenerator cmakegen(&parser);
            cmakegen.Generate("proxystub");
        }
        catch (const CCompileException& rexcept)
        {
            std::stringstream sstreamError;
            if (!rexcept.GetPath().empty())
                std::cout << rexcept.GetPath() << "(line=" << rexcept.GetLineNo() << ", col=" << rexcept.GetColNo() <<
                ") error: " << rexcept.GetReason() << std::endl;
            else
                std::cout << "error: " << rexcept.GetReason() << std::endl;
            return;
        }

        // Compilation was successful
        m_bResult = true;
    }

    /**
     * @brief Return the result.
     */
    operator bool() const
    {
        return m_bResult;
    }

private:
    bool            m_bResult = false;  ///< The result value of the test.
};

using CCompilerTest = CParserTest;

TEST_F(CCompilerTest, EmptyFile)
{
    // Tests bug fix: #380794
    EXPECT_TRUE(CTestCompile(""));
    EXPECT_TRUE(CTestCompile("/// @file Empty file without any definitions"));
}

TEST_F(CCompilerTest, SkipCodePreCompiled)
{
    // Partially skip code
    EXPECT_TRUE(CTestCompile(R"code(/// @file Skip code by using a conditional pre-compile statement

struct S1
{};

#if 0

module mod
{};

#endif

struct S2
{};)code"));

    // Tests bug fix: #380794
    // Completely skip code
    EXPECT_TRUE(CTestCompile(R"code(/// @file Skip complete file by using a conditional pre-compile statement

#if 0

module mod
{};

#endif)code"));
}

TEST_F(CCompilerTest, SkipCodeCommenting)
{
    // Partially skip code
    EXPECT_TRUE(CTestCompile(R"code(/// @file Skip code by using a commented out code

struct S1
{};

//module mod
//{};

struct S2
{};)code"));

    // Tests bug fix: #380794
    // Completely skip code
    EXPECT_TRUE(CTestCompile(R"code(/// @file Skip code by using a commented out code

//struct S1
//{};

//module mod
//{};

//struct S2
//{};)code"));

    // Tests bug fix: #380790
    // Skip large chunk
    std::stringstream sstream;
    sstream << R"code(/// @file Skip code by using a commented out code

struct S1
{};
)code";
    for (size_t n = 0; n < 20000; n++)
    {
        sstream << R"code(// struct STest {
// this is a test with lots of comments
// which should be skipped and not be processed
// and should not lead to any unexpected behaviour
// like crashes or giant comment blocks for the definition
// following...
// };
)code";
    }

    sstream << R"code(
struct S2
{};)code";
    EXPECT_TRUE(CTestCompile(sstream.str()));
}

TEST_F(CCompilerTest, GeneratedCodeWithTypedefOfArray)
{
    const char szIdl[] = R"code(struct LidarPointField {};
const int32 maxLidarPoints = 32;
typedef LidarPointField LidarPointsField_t;
typedef LidarPointField LidarPointsAry[maxLidarPoints];

struct LidarPoints
{
    uint32 pointNumber;
    LidarPointsField_t point;
    LidarPointsAry lidarPointsList;
};)code";

    const char szHdr[] = R"code(struct LidarPointField{};
    static const int32_t maxLidarPoints = 32;
    typedef LidarPointField LidarPointsField_t;
    typedef LidarPointField LidarPointsAry[maxLidarPoints];
    struct LidarPoints
    {
        uint32_t pointNumber;
        LidarPointsField_t point;
        LidarPointsAry lidarPointsList;
    };)code";

    // Tests bug fix: #385982
    CTestCompile test(szIdl);
    EXPECT_TRUE(test);
    EXPECT_CPPEQ(test.Header(), szHdr);
}

TEST_F(CCompilerTest, ExplicitSpecifyModule)
{
    const char szIdl[] = R"code(module MyAPI
{
    struct VehSpd {};
    module interfaces
    {
        struct VehSpd
        {
            MyAPI::VehSpd notifyStatus;
        };
    };
};)code";

    const char szHdr[] = R"code(namespace MyAPI
{
    struct VehSpd
    {};
    namespace interfaces
    {
        struct VehSpd
        {
            MyAPI::VehSpd notifyStatus;
        };
    }
}
)code";

    // Tests bug fix: #384857
    CTestCompile test(szIdl);
    EXPECT_TRUE(test);
    EXPECT_CPPEQ(test.Header(), szHdr);
}

// NOTE 11.04.2025: The C++ comparison function is not working correctly.
TEST_F(CCompilerTest, DISABLED_ForwardDeclUnion)
{
    const char szIdlTypeBased[] = R"code(union UTest;
/**
 * @brief Standard union based on a integer.
 */
union UTest switch (uint32)
{
case 10:        boolean bVal;  ///< Bool value
case 20:        uint64 uiVal;  ///< 64-bit int value
case 30:        float fVal;    ///< Float value
default:        string ssVal;  ///< String value
};)code";

    const char szHdrTypeBased[] = R"code(struct UTest;
struct UTest;

/**
 * @brief Standard union based on a integer.
 */
struct UTest
{
    /** Constructor */
    UTest()
    {
        construct_switch_value();
    }

    /** Destructor */
    ~UTest()
    {
        destruct_switch_value();
    }

private:
    /** Constructor helper function for switch_value */
    void construct_switch_value(uint32_t val = uint32_t{})
    {
        switch_value = val;
        switch (val)
        {
        case 10:
            new (&bVal)  bool;
            break;
        case 20:
            new (&uiVal)  uint64_t;
            break;
        case 30:
            new (&fVal)  float;
            break;
        default:
            new (&ssVal) sdv::string;
            break;
        }
    }

    /** Destructor helper function for switch_value */
    void destruct_switch_value()
    {
        switch (switch_value)
        {
        case 10:
            break;
        case 20:
            break;
        case 30:
            break;
        default:
            ssVal.~string_base();
            break;
        }
    }

public:
    /** Set the switch type for the union UTest */
    void switch_to(uint32_t val)
    {
        // Anything to do?
        if (switch_value == val) return;

        // Assign the new value...
        switch_value = val;

        // Destruct and construct switch_value...
        destruct_switch_value();
        construct_switch_value(val);
    }

    /** Get the switch value */
    uint32_t get_switch() const
    {
        return switch_value;
    }

private:
    uint32_t switch_value;     ///< Union switch variable.

public:
    union /*switch(switch_value)*/
    {
        // case 10:
        /// Bool value
        bool bVal;

        // case 20:
        /// 64-bit int value
        uint64_t uiVal;

        // case 30:
        /// Float value
        float fVal;

        // default
        /// String value
        sdv::string ssVal;
    };
};
)code";

    // Tests bug fix: #380792
    CTestCompile testTypeBased(szIdlTypeBased);
    EXPECT_TRUE(testTypeBased);
    EXPECT_CPPEQ(testTypeBased.Header(), szHdrTypeBased);

    const char szIdlVarBased[] = R"code(struct S {
    // Forward declaration
    union UTest;

    int32 iVal;

    /**
     * @brief Standard union based on a integer value.
     */
    union UTest switch (iVal)
    {
    case 10:        boolean bVal;  ///< Bool value
    case 20:        uint64 uiVal;  ///< 64-bit int value
    case 30:        float fVal;    ///< Float value
    default:        char cVal;     ///< Character value
    };
};)code";

    const char szHdrVarBased[] = R"code(struct S {
    union UTest;
    int32_t iVal;
    union UTest
    {
        UTest() {}
        ~UTest() {}
        bool bVal;
        uint64_t uiVal;
        float fVal;
        char cVal;
    };
};
)code";

    // Tests bug fix: #380792
    CTestCompile testVarBased(szIdlVarBased);
    EXPECT_TRUE(testVarBased);
    EXPECT_CPPEQ(testVarBased.Header(), szHdrVarBased);
}

// NOTE 11.04.2025: The C++ comparison function is not working correctly.
TEST_F(CCompilerTest, DISABLED_UnionWithComplexMembers)
{
    const char szIdlTypeBased[] = R"code(/**
 * @brief Standard union based on a integer.
 */
union UTest switch (uint32)
{
case 10:        boolean bVal;  ///< Bool value
case 20:        uint64 uiVal;  ///< 64-bit int value
case 30:        float fVal;    ///< Float value
default:        string ssVal;  ///< String value
};)code";

    const char szHdrTypeBased[] = R"code(struct UTest
{
    UTest(uint32_t val = 10)
    {
        switch_value = val;
        switch (val)
        {
        case 10: new (&bVal) bool; break;
        case 20: new (&uiVal) uint64_t; break;
        case 30: new (&fVal) float; break;
        default: new (&ssVal) sdv::string; break;
        }
    }
    ~UTest()
    {
        switch (switch_value)
        {
        case 10: break;
        case 20: break;
        case 30: break;
        default: ssVal.~string_base(); break;
        }
    }
    void switch_to(uint32_t val)
    {
        if (val == switch_value) return;
        this->~UTest();
        new (this) UTest(val);
    }

    uint32_t switch_value;
    union
    {
        bool bVal;
        uint64_t uiVal;
        float fVal;
        sdv::string ssVal;
    };
};
)code";

    // Tests bug fix: #380792
    CTestCompile testTypeBased(szIdlTypeBased);
    EXPECT_TRUE(testTypeBased);
    EXPECT_CPPEQ(testTypeBased.Header(), szHdrTypeBased);

}
