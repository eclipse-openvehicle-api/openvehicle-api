#ifndef CONTEXT_H
#define CONTEXT_H

#include "../includes.h"
#include <filesystem>
#include <map>

/**
 * @brief Generator context
 */
class CGenContext
{
public:
    /**
     * @brief Constructor
     * @param[in] pParser Pointer to the parser object.
     */
    CGenContext(sdv::IInterfaceAccess* pParser);

    /**
     * @brief Destructor
     */
    virtual ~CGenContext();

    /**
     *
     * @brief Get the parser interface.
     * @tparam TInterface The interface to request the interface from.
     * @param[in] pObject Pointer to the object to request the interface from or NULL when the parser should be asked.
     * @return Returns a pointer to the interface if available.
     */
    template <typename TInterface>
    TInterface* GetInterface(sdv::IInterfaceAccess* pObject) const;

    /**
     * @brief Return the interface to the parser.
     * @return The interface to the parser or NULL when the parser is not available.
     */
    sdv::IInterfaceAccess* GetParser() const { return m_pParser; }

    /**
     * @brief Get source path.
     * @return Returns the source path.
     */
    std::filesystem::path GetSource() const;

    /**
     * @brief Get the output directory.
     * @remarks If there is no output directory defined, takes the parent directory of the source.
     * @return Returns the output directory path.
     */
    std::filesystem::path GetOutputDir() const;

    /**
     * @brief File header generation using JavaDoc C-style comments.
     * @param[in] rpathFile Reference to the path to the file to generate the header for.
     * @param[in] rssDescription Optional description to add to the file header.
     * @return Returns the file header string.
     */
    std::string Header(const std::filesystem::path& rpathFile, const std::string& rssDescription = std::string()) const;

    /**
     * @brief Creates safeguard C++ string that can be used to safeguard a C++ header file.
     * @param[in] rpathFile Reference to the path to the file to use for safeguarding.
     * @param[in] bInitial When set, creates initial lines, otherwise closing lines.
     * @return Returns the safeguard string composed front the path.
     */
    static std::string Safeguard(const std::filesystem::path& rpathFile, bool bInitial);

    /**
     * @brief Insert indentation before each text within a (multi-line) string.
     * @details If the line doesn't start with a number sign insert an indentation before each line. Also a line-concatinating
     * character (back-slash before end of line) will be removed. Independent of the number sign, any whitespace at the end of each
     * line will be removed.
     * @param[in] rssStr Reference to the string to adapt.
     * @param[in] rssIndent Reference to the indentation string to insert.
     * @return Returns the string with inserted indentations
     */
    static std::string SmartIndent(const std::string& rssStr, const std::string& rssIndent);

    /**
     * @brief Make a qualified identifier from a fully scoped name with array brackets.
     * @details A fully scoped name contains all the namespace and struct definitions that define the context of the supplied name.
     * The names of each level is separated by the scope separator (::). Also any member declaration is separated by the member
     * separator (.). The name could also contain square brackets to identify an array. This function replaces the scope operator by
     * a double underscore (__), the array operator by a single underscore and the array brackets by a single underscore. In the
     * end, the name results in a qualified C++ name.
     * @param[in] rssName Reference to the name string to qualify.
     * @return Returns the qualified name sstring.
     */
    static std::string QualifyName(const std::string& rssName);

    /**
     * @brief Keyword map for keyword replacement in a string.
     */
    typedef std::map<std::string, std::string> CKeywordMap;

    /**
     * @brief Vector containing the exceptions that might be thrown by the function.
     */
    typedef std::vector<std::string> CExceptionVector;

    /**
     * @brief Replace keywords in a string.
     * @param[in] rssStr Reference to the string containing the keywords.
     * @param[in] rmapKeywords Map with keywords to replace.
     * @param[in] cMarker Character to identify the keyword with (placed before and after the keyword; e.g. %keyword%).
     * @return Returns the string with replacements.
     */
    static std::string ReplaceKeywords(const std::string& rssStr, const CKeywordMap& rmapKeywords, char cMarker = '%');

    /**
     * @brief Get indentation string (represents one tab).
     * @return Reference to the string with the indentation.
     */
    static std::string GetIndentChars();

    /**
     * @brief Declaration information.
     */
    struct SCDeclInfo
    {
		sdv::idl::EDeclType	eBaseType = sdv::idl::EDeclType::decltype_unknown;    ///< Base type
        std::string                 ssDeclType;             ///< Declaration type (incl. pointer addition for every array extend).
        bool                        bIsPointer = false;     ///< Type is represented as a pointer to dynamic data or an interface.
		bool                        bIsComplex = false;     ///< Complex data type; use by-ref for parameters.
        bool                        bTemplated = false;     ///< Type has template parameters
        bool                        bIsInterface = false;   ///< Type is an interface pointer.
		bool                        bIsString = false;      ///< Set when the type represents a string object.
        bool                        bIsDynamic = false;     ///< Type is dynamic
        bool                        bValidType = false;     ///< Type is not void
    };

    /**
     * @brief Get the C++ declaration type as string.
     * @attention Does not check for anonymous types.
     * @param[in] pDeclTypeObj Pointer to the IInterfaceAccess interface of the declaration type object.
     * @param[in] rssScope Reference to the string containing the current scope.
     * @param[in] bScopedName When set, return the scoped name.
     * @return The declaration information (or empty types when not available).
     */
    SCDeclInfo GetCDeclTypeStr(sdv::IInterfaceAccess* pDeclTypeObj, const std::string& rssScope /*= std::string()*/, bool bScopedName = false) const;

    /**
     * @brief Map the IDL type to the C type (if possible).
     * @param[in] eEntityType The entity type to map.
     * @return Returns a string representing the C type or empty if there is no C type or "invalid" if the type is invalid.
     */
    static std::string MapEntityType2CType(sdv::idl::EEntityType eEntityType);

    /**
    * @brief Map the IDL type to the C type (if possible).
    * @param[in] eDeclType The declaration type to map.
    * @return Returns a string representing the C type or empty if there is no C type or "invalid" if the type is invalid.
    */
    static std::string MapDeclType2CType(sdv::idl::EDeclType eDeclType);

    /**
     * @brief Get a relative scoped name based on the provided scope entity.
     * @param[in] ssScopedName Reference to the fully scoped name of the entity.
     * @param[in] rssScope Reference to the current scope.
     * @return String with the relatively scoped name.
     */
    static std::string GetRelativeScopedName(const std::string& ssScopedName, const std::string& rssScope);

private:
    sdv::IInterfaceAccess*        m_pParser = nullptr;        ///< Parse tree instance.
    sdv::idl::ICompilerInfo*      m_pCompilerInfo = nullptr;  ///< Compiler information interface.
    sdv::idl::ICompilerOption*    m_pOption = nullptr;        ///< Program options interface.
};

template <typename TInterface>
inline TInterface* CGenContext::GetInterface(sdv::IInterfaceAccess* pObject) const
{
    if (!pObject) return nullptr;
    return pObject->GetInterface(sdv::GetInterfaceId<TInterface>()).template get<TInterface>();
}

#ifndef DOXYGEN_IGNORE
template <>
inline sdv::IInterfaceAccess* CGenContext::GetInterface(sdv::IInterfaceAccess* pObject) const
{
    return pObject;
}
#endif

#endif // !defined(CONTEXT_H)