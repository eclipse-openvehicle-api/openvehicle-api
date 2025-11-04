#ifndef ENV_H
#define ENV_H

#include <filesystem>
#include <set>
#include <stack>

#include "logger.h"
#include "core_idl_backup.h"
#include "../global/cmdlnparser/cmdlnparser.h"
#include "includes.h"
#include "macro.h"
#include "token.h"
#include "exception.h"

/**
 * @brief Parser environment management class.
 */
class CIdlCompilerEnvironment
    : public sdv::idl::ICompilerOption
    , public sdv::IInterfaceAccess
{
public:
    /**
     * @brief Default constructor
     */
    CIdlCompilerEnvironment();

    /**
     * @brief Constructor with program arguments.
     * @param[in] rvecArgs Reference to the vector with program arguments
     */
    CIdlCompilerEnvironment(const std::vector<std::string>& rvecArgs);

    /**
     * @brief Constructor with program arguments.
     * @tparam TCharType Character type.
     * @param[in] nArgs The amount of arguments.
     * @param[in] rgszArgs Array of arguments.
     */
    template <typename TCharType>
    CIdlCompilerEnvironment(size_t nArgs, const TCharType** rgszArgs);

    /**
     * @brief Get access to another interface. Overload of IInterfaceAccess::GetInterface.
     * @param[in] idInterface The interface id to get access to.
     * @return Returns a pointer to the interface or NULL when the interface is not supported.
     */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Get the path of the next file.
     * @return Returns the path to the next file or an empty path.
     */
    std::filesystem::path GetNextFile();

    /**
     * @brief Help was requested on the command line.
     * @return Returns 'true' when help was requested. Otherwise returns 'false'.
     */
    bool Help() const { return m_bCLHelp; }

    /**
     * @brief Show command line help.
     */
    void ShowHelp() const { m_cmdln.PrintHelp(std::cout); }

    /**
     * @brief Version information was requested on the command line.
     * @return Returns 'true' when version info was requested. Otherwise returns 'false'.
     */
    bool Version() const { return m_bCLVersion; }

    /**
     * @brief Get the vector of include directories.
     * @return Reference to the vector of include directories.
     */
    const std::vector<std::filesystem::path>& GetIncludeDirs() const { return m_vecIncludeDirs; }

    /**
     * @brief Get the output directory.
     * @return The output directory.
     */
    std::filesystem::path GetOutputDir() const { return m_pathOutputDir; }

    /**
     * @brief Add macro definition in the form of name, name=value or name(arg1, arg2, arg3)=value.
     * @param[in] szDefinition The definition string
     */
    void AddDefinition(const char* szDefinition);

    /**
     * @brief Add macro definition.
     * @param[in] rtoken Token in the source file of the provided macro.
     * @param[in] rMacro Reference to the macro to add.
     */
    void AddDefinition(const CToken& rtoken, const CMacro& rMacro);

    /**
     * @brief Remove macro definition.
     * @param[in] szMacro The name of the macro.
     * @remarks If the macro doesn#t exists, doesn't do anything.
     */
    void RemoveDefinition(const char* szMacro);

    /**
     * @brief Check whether a macro definition exists.
     * @param[in] szMacro The name of the macro.
     * @returns Returns 'true' when the definition exists; 'false' otherwise.
     */
    bool Defined(const char* szMacro) const;

    /**
     * @brief Test for a macro and replace the code.
     * @details This function will check for the existence of the macro with the supplied name. If it does, it will read any
     * parameter from the code (depends on whether the macro needs parameters) and create a string with the filled in parameters.
     * This string, then, is prepended to the code replacing the identifiert and its optional parameters. Reparsing of the code
     * needs to take place - the return value is 'true'. If there is no macro with the supplied name, the return value is 'false'
     * and the supplied identifier is to be treated as an identifier.
     * Macro parameters can have commas if they are parenthesized before.
     * The value can have the '#' unary operator to stringificate the following parameter and the '##' operator to join the
     * preceding or succeeding operator to the adjacent identifier.
     * @param[in] rssIdentifier Reference to the string object containing the name of the identifier to test for macro definition.
     * @param[in, out] rcode The code holding the potential parameters and to be replaced and prepended with the resolved macro.
     * @param[in] bInMacroExpansion Set 'true' when the identifier was the (part) result of a macro expansion. If this is the
     * case, previously used macros cannot be reused - to prevent circular expansion.
     * @param[in, out] rsetPreviousExpanded Reference to the set of previously expanded macros, preventing circular expansion. If
     * the used macro set is empty, use the default set of the environment. This set will b eextended with macros used within the
     * macro expansion.
     * @return Returns 'true' if macro replacement was successful, or 'false' when the identifier is not a macro.
     */
    bool TestAndExpand(const std::string&   rssIdentifier,
                       CCodePos&            rcode,
                       bool                 bInMacroExpansion,
                       CUsedMacroSet&       rsetPreviousExpanded) const;

    /**
     * @brief Test for a macro and replace the code.
     * @details This function will check for the existence of the macro with the supplied name. If it does, it will read any
     * parameter from the code (depends on whether the macro needs parameters) and create a string with the filled in parameters.
     * This string, then, is prepended to the code replacing the identifiert and its optional parameters. Reparsing of the code
     * needs to take place - the return value is 'true'. If there is no macro with the supplied name, the return value is 'false'
     * and the supplied identifier is to be treated as an identifier.
     * Macro parameters can have commas if they are parenthesized before.
     * The value can have the '#' unary operator to stringificate the following parameter and the '##' operator to join the
     * preceding or succeeding operator to the adjacent identifier.
     * @param[in] rssIdentifier Reference to the string object containing the name of the identifier to test for macro definition.
     * @param[in, out] rcode The code holding the potential parameters and to be replaced and prepended with the resolved macro.
     * @param[in] bInMacroExpansion Set 'true' when the identifier was the (part) result of a macro expansion. If this is the
     * case, previously used macros cannot be reused - to prevent circular expansion.
     * @return Returns 'true' if macro replacement was successful, or 'false' when the identifier is not a macro.
     */
    bool TestAndExpand(const std::string& rssIdentifier, CCodePos& rcode, bool bInMacroExpansion = false) const
    {
        CUsedMacroSet setDummy;
        return TestAndExpand(rssIdentifier, rcode, bInMacroExpansion, setDummy);
    }

    /**
     * @brief Resolve const expressions.
     * @return Returns 'true' when const expressions should be resolved. Otherwise the expression should be exported during code
     * generation.
     */
    bool ResolveConst() const;

    /**
     * @brief Suppress the generation of proxy and stub code.
     * @return Returns 'true' when proxy stub code should not be generated. Otherwise the proxy and stub code should be generated.
     */
    bool NoProxyStub() const;

    /**
     * @brief Get library target name for the proxy stub cmake file.
     * @return Returns a reference to the target name if set in the command line or an empty string otherwise.
     */
    const std::string& GetProxStubCMakeTarget() const;

    /**
     * @brief Get the compiler option. Overload of sdv::idl::ICompilerOption::GetOption.
     * @param[in] rssOption Reference to the string containing the name of the option to retrieve.
     * @return The requested compiler option (if available).
     */
    virtual sdv::u8string GetOption(/*in*/ const sdv::u8string& rssOption) const override;

    /**
     * @brief Get the amount of option values. Overload of sdv::idl::ICompilerOption::GetOptionCnt.
     * @param[in] rssOption Reference to the string containing the name of the option to retrieve.
     * @return The amount of option values.
     */
    virtual uint32_t GetOptionCnt(/*in*/ const sdv::u8string& rssOption) const override;

    /**
     * @brief Get the compiler option. Overload of sdv::idl::ICompilerOption::GetOptionN.
     * @param[in] rssOption Reference to the string containing the name of the option to retrieve.
     * @param[in] uiIndex The index of the option value.
     * @return The requested compiler option value.
     */
    virtual sdv::u8string GetOptionN(/*in*/ const sdv::u8string& rssOption, /*in*/ uint32_t uiIndex) const override;

    /**
     * @{
     * @brief Extensions
     * @return Returns true when the extension is enabled; false when not.
     */
    bool InterfaceTypeExtension() const { return m_bEnableInterfaceTypeExtension; }
    bool ExceptionTypeExtension() const { return m_bEnableExceptionTypeExtension; }
    bool PointerTypeExtension() const { return m_bEnablePointerTypeExtension; }
    bool UnicodeExtension() const { return m_bEnableUnicodeExtension; }
    bool CaseSensitiveTypeExtension() const { return m_bEnableCaseSensitiveNameExtension; }
    bool ContextDependentNamesExtension() const { return m_bEnableContextDependentNamesExtension; }
    bool MultiDimArrayExtension() const { return m_bEnableMultiDimArrayExtension; }
    /**
     * @}
     */

private:
    CCommandLine m_cmdln;                                   ///< Command line parsing class.
    CMacroMap m_mapMacros;                                  ///< Map containing macros.
    std::vector<std::filesystem::path> m_vecIncludeDirs;    ///< Vector containing all search paths for finding files.
    std::vector<std::filesystem::path> m_vecFileNames;      ///< Vector containing file paths of the files to process.
    std::filesystem::path m_pathOutputDir;                  ///< Output directory.
    mutable CUsedMacroSet m_setMacrosUsedInExpansion;       ///< Set of macros used in the expansion to prevent circular expansion
                                                            ///< when the macro being expanded generate a call to itself.
    bool m_bCLHelp       = false;                           ///< Help was requested at the command line.
    bool m_bCLVersion	 = false;                           ///< Version info was request at the command line.
    bool m_bResolveConst = false;                           ///< When set, store the value of a const assignment; otherwise, store
                                                            ///< the expression of the const assignment.
    bool m_bNoPS = false;                                   ///< Doesn't generate proxy/stub code.
    std::string m_ssProxyStubLibName = "proxystub";         ///< Proxy and stub library target name in the generated cmake file.
    std::filesystem::path m_pathCompilerPath;               ///< The path to the compiler.
    int32_t m_iFileIndex = -1;                              ///< The IDL file to process.
    bool m_bEnableInterfaceTypeExtension = true;            ///< Enable the 'inteface_t', 'inteface_id' and 'null' keywords.
    bool m_bEnableExceptionTypeExtension = true;            ///< Enable the 'exception_id' keyword.
    bool m_bEnablePointerTypeExtension = true;              ///< Enable the 'pointer' keyword.
    bool m_bEnableUnicodeExtension = true;                  ///< Enable UTF-8, UTF-16 and UTF-32 character and string extensions.
    bool m_bEnableCaseSensitiveNameExtension = true;        ///< Enable sensitive name restriction.
    bool m_bEnableContextDependentNamesExtension = true;    ///< Enable relaxed uniqueness extension.
    bool m_bEnableMultiDimArrayExtension = true;            ///< Enable multi-dimensional array support.
};

template <typename TCharType>
CIdlCompilerEnvironment::CIdlCompilerEnvironment(size_t nArgs, const TCharType** rgszArgs) :
    m_cmdln(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character))
{
    try
    {
        bool bSilent = false;
        bool bVerbose = false;
        bool bStrict = false;
        std::vector<std::string> vecMacros;
        auto& rArgHelpDef = m_cmdln.DefineOption("?", m_bCLHelp, "Show help");
        rArgHelpDef.AddSubOptionName("help");
        auto& rArgSilentDef = m_cmdln.DefineOption("s", bSilent, "Do not show any information on STDOUT. Not compatible with 'verbose'.");
        rArgSilentDef.AddSubOptionName("silent");
        auto& rArgVerboseDef = m_cmdln.DefineOption("v", bVerbose, "Provide verbose information. Not compatible with 'silent'.");
        rArgVerboseDef.AddSubOptionName("verbose");
        m_cmdln.DefineSubOption("version", m_bCLVersion, "Show version information");
        m_cmdln.DefineOption("I", m_vecIncludeDirs, "Set include directory");
        m_cmdln.DefineOption("O", m_pathOutputDir, "Set output directory");
        m_cmdln.DefineOption("D", vecMacros, "Set a macro definition in the form of macro, macro=<value>, macro(arg1,...)=<value>");
        m_cmdln.DefineSubOption("resolve_const", m_bResolveConst, "Use the calculated value for const values, instead of the defined expression.");
        m_cmdln.DefineSubOption("no_ps", m_bNoPS, "Do not create any proxy and stub code (interface definitions only).");
        m_cmdln.DefineSubOption("ps_lib_name", m_ssProxyStubLibName, "Proxy and stub library target name in the generated cmake file (default=\"proxystub\").");

        m_cmdln.DefineGroup("Extensions", "Enable/disable compatibility extensions.");
        m_cmdln.DefineFlagSubOption("interface_type", m_bEnableInterfaceTypeExtension, "Enable/disable support of the interface type extensions 'interface_t' and 'interface_id'.Default enabled.");
        m_cmdln.DefineFlagSubOption("exception_type", m_bEnableExceptionTypeExtension, "Enable/disable support of the exception type extension 'exception_id'. Default enabled.");
        m_cmdln.DefineFlagSubOption("pointer_type", m_bEnablePointerTypeExtension, "Enable/disable support of the pointer type extension 'pointer'. Default enabled.");
        m_cmdln.DefineFlagSubOption("unicode_char", m_bEnableUnicodeExtension, "Enable/disable support of the UTF-8, UTF-16 and UTF-32 Unicode extensions. Default enabled.");
        m_cmdln.DefineFlagSubOption("case_sensitive", m_bEnableCaseSensitiveNameExtension, "Enable/disable case sensitive name restriction extension. Default enabled.");
        m_cmdln.DefineFlagSubOption("context_names", m_bEnableContextDependentNamesExtension, "Enable/disable the use of context dependent names in declarations. Default enabled.");
        m_cmdln.DefineFlagSubOption("multi_dimensional_array", m_bEnableMultiDimArrayExtension, "Enable/disable support of multi-dimensional arrays extension. Default enabled.");
        m_cmdln.DefineSubOption("strict", bStrict, "Strictly maintaining OMG-IDL conformance; disabling extensions.");

        m_cmdln.DefineDefaultArgument(m_vecFileNames, "IDL files");
        m_cmdln.Parse(nArgs, rgszArgs);

        // Add the macros.
        for (const std::string& rssMacro : vecMacros)
        {
            try
            {
                AddDefinition(rssMacro.c_str());
            }
            catch (const sdv::idl::XCompileError&)
            {
                throw CCompileException("Invalid command line option: -D");
            }
        }

        // If strict, disable all extensions.
        if (bStrict)
        {
            m_bEnableInterfaceTypeExtension = false;
            m_bEnableExceptionTypeExtension = false;
            m_bEnablePointerTypeExtension = false;
            m_bEnableUnicodeExtension = false;
            m_bEnableCaseSensitiveNameExtension = false;
            m_bEnableContextDependentNamesExtension = false;
            m_bEnableMultiDimArrayExtension = false;
        }

        // Set the verbosity mode.
        if (bSilent)
            g_log_control.SetVerbosityMode(EVerbosityMode::report_none);
        else if (bVerbose)
            g_log_control.SetVerbosityMode(EVerbosityMode::report_all);
    }
    catch (const SArgumentParseException& rsExcept)
    {
        m_bCLHelp = true;
        throw CCompileException(rsExcept.what());
    }
}

#endif // !defined ENV_H
