#ifndef PARSER_TOML_H
#define PARSER_TOML_H

#include "lexer_toml.h"
#include "parser_node_toml.h"

/**
 * @brief Creates a tree structure from input of UTF-8 encoded TOML source data
 */
class CParserTOML : public sdv::IInterfaceAccess, public sdv::toml::ITOMLParser
{
public:
    /**
     * @brief Default constructor
     */
    CParserTOML() = default;

    /**
     * @brief Construct a new Parser object
     * @param[in] rssString UTF-8 encoded data of a TOML source
     */
    CParserTOML(const std::string& rssString);

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::toml::ITOMLParser)
        SDV_INTERFACE_CHAIN_MEMBER(m_ptrRoot)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Clears the current parse result.
     * @attention This will render any pointer invalid!
     */
    void Clear();

    // Ignore cppcheck warning for not using dynamic binding when being called through the constructor.
    // cppcheck-suppress virtualCallInConstructor
    /**
    * @brief Process the configuration from the supplied content string. Overload of sdv::toml::ITOMLParser.
    * @param[in] ssContent Configuration string.
    * @return Returns 'true' when the configuration could be read successfully, false when not.
    */
    virtual bool Process(/*in*/ const sdv::u8string& ssContent) override;

    /**
     * @{
     * @brief Return the root node.
     * @return Reference to the root node collection.
     */
    const CNodeCollection& GetRoot() const;
    CNodeCollection& GetRoot();
    /**
     * @}
     */

    /**
     * @brief Get the TOML text based on the content.
     * @param[in] rssParent When present, uses the parent node into the TOML text generation.
     * @return The string containing the TOML text.
     */
    std::string CreateTOMLText(const std::string& rssParent = std::string()) const;

private:
    /**
     * @brief Add a collection node (table or array).
     * @tparam TCollectionNode The collection node class to add (to create).
     * @param[in] rssPath Reference to the node path.
     * @return Returns whether the node could be added.
     */
    template <class TCollectionNode>
    bool Add(const std::string& rssPath);

    /**
     * @brief Add a boolean value node.
     * @param[in] rssPath Reference to the node path.
     * @param[in] bVal The boolean value.
     * @return Returns whether the node could be added.
     */
    bool Add(const std::string& rssPath, bool bVal);

    /**
     * @brief Add a integer value node.
     * @param[in] rssPath Reference to the node path.
     * @param[in] iVal The integer value.
     * @return Returns whether the node could be added.
     */
    bool Add(const std::string& rssPath, int64_t iVal);

    /**
     * @brief Add a floating point value node.
     * @param[in] rssPath Reference to the node path.
     * @param[in] dVal The floating point value.
     * @return Returns whether the node could be added.
     */
    bool Add(const std::string& rssPath, double dVal);

    /**
     * @brief Add a string value node.
     * @param[in] rssPath Reference to the node path.
     * @param[in] rssVal Reference to the string value.
     * @return Returns whether the node could be added.
     */
    bool Add(const std::string& rssPath, const std::string& rssVal);

    /**
     * @brief Process a table declaration.
     */
    void ProcessTable();

    /**
     * @brief Process a table array declaration.
     */
    void ProcessTableArray();

    /**
     * @brief Process the value key.
    */
    void ProcessValueKey();

    /**
     * @brief Process the value with the supplied key.
     * @param[in] rssKeyPath Reference to the key path string.
     */
    void ProcessValue(const std::string& rssKeyPath);

    /**
     * @brief Process the array value with the supplied key.
     * @param[in] rssKeyPath Reference to the key path string.
     */
    void ProcessArray(const std::string& rssKeyPath);

    /**
     * @brief Process the inline table value with the supplied key.
     * @param[in] rssKeyPath Reference to the key path string.
     */
    void ProcessInlineTable(const std::string& rssKeyPath);

    /**
     * @brief Compose a path from lexer tokens. A path is composed of table and array elements separated with a dot.
     * @return The composed path.
     */
    std::string ComposePath();

    /**
    * @brief Enum for differentiating between an array environment and an inline table environment for syntax checks.
    */
    enum class EEnvironment
    {
        env_array,          //!< Environment for an array
        env_inline_table    //!< Environment for a table
    };
    std::stack<EEnvironment>    m_stackEnvironment;                             ///< Tracking of environments in nested structures.
    std::shared_ptr<CNode>      m_ptrRoot = std::make_shared<CRootTable>();     ///< The one root node.
    std::string                 m_ssCurrentTable;                               ///< Path to the current table.
    CLexerTOML                  m_lexer;                                        ///< Lexer.
};

template <class TCollectionNode>
inline bool CParserTOML::Add(const std::string& rssPath)
{
    size_t nOffset = rssPath.rfind('.');
    if (nOffset == std::string::npos)
        nOffset = 0;
    else
        nOffset++;
    std::string ssName = rssPath.substr(nOffset);

    m_ptrRoot->Add(rssPath, std::make_shared<TCollectionNode>(ssName), true);

    return true;
}

#endif // PARSER_TOML_H
