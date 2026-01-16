#ifndef PARSER_TOML_H
#define PARSER_TOML_H

#include "lexer_toml.h"
#include "parser_node_toml.h"
#include "miscellaneous.h"
#include <stack>
#include <memory>
#include <string>

/// The TOML parser namespace
namespace toml_parser
{
    class CNode;

    /**
     * @brief Creates a tree structure from input of UTF-8 encoded TOML source data
     */
    class CParser : public sdv::IInterfaceAccess, public sdv::toml::ITOMLParser
    {
    public:
        /**
         * @brief Default constructor
         */
        CParser() = default;

        /**
         * @brief Construct a new Parser object
         * @param[in] rssString UTF-8 encoded data of a TOML source
         */
        CParser(const std::string& rssString);

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
         * @brief Get the lexer containing the token list.
         * @return A reference to the lexer containing the token list.
         */
        CLexer& Lexer();

        /**
         * @{
         * @brief Return the root node.
         * @return Reference to the root node collection.
         */
        const CNodeCollection& Root() const;
        CNodeCollection& Root();
        /**
         * @}
         */

        /**
         * @brief Get the TOML text based on the content.
         * @param[in] rssPrefixKey When present, uses the prefix node into the TOML text generation. The string must follow the key
         * rules for separation with bare, literal and quoted keys.
         * @return The string containing the TOML text.
         */
        std::string GenerateTOML(const std::string& rssPrefixKey = std::string()) const;

    private:
        /**
         * @brief Process a table declaration.
         * @param[in, out] rNodeRange Reference to the extended token range of the node.
         */
        void ProcessTable(CNodeTokenRange& rNodeRange);

        /**
         * @brief Process a table array declaration.
         * @param[in, out] rNodeRange Reference to the extended token range of the node.
         */
        void ProcessTableArray(CNodeTokenRange& rNodeRange);

        /**
         * @brief Process the value key.
         * @param[in, out] rNodeRange Reference to the extended token range of the node.
         */
        void ProcessValueKey(CNodeTokenRange& rNodeRange);

        /**
         * @brief Process the value with the supplied key.
         * @param[in] rrangeKeyPath Reference to the key path token range.
         * @param[in, out] rNodeRange Reference to the extended token range of the node.
         */
        void ProcessValue(const CTokenRange& rrangeKeyPath, CNodeTokenRange& rNodeRange);

        /**
         * @brief Process the array value with the supplied key.
         * @param[in, out] rNodeRange Reference to the extended token range of the node. The second main range will be added.
         */
        void ProcessArray(CNodeTokenRange& rNodeRange);

        /**
         * @brief Process the inline table value with the supplied key.
         * @param[in, out] rNodeRange Reference to the extended token range of the node. The second main range will be added.
         */
        void ProcessInlineTable(CNodeTokenRange& rNodeRange);

        /**
         * @brief Compose a path from lexer tokens. A path is composed of table and array elements separated with a dot.
         * @return The token range of the key path.
         */
        CTokenRange ProcessKeyPath();

        /**
         * @brief Enum for differentiating between an array environment and an inline table environment for syntax checks.
         */
        enum class EEnvironment
        {
            env_array,       ///< Environment for an array
            env_inline_table ///< Environment for a table
        };

        std::stack<EEnvironment>            m_stackEnvironment;     ///< Tracking of environments in nested structures.
        std::shared_ptr<CRootTable>         m_ptrRoot;              ///< The one root node.
        std::shared_ptr<CNodeCollection>    m_ptrCurrentCollection; ///< The current collection node.
        CLexer                              m_lexer;                ///< Lexer.
    };
} // namespace toml_parser

#endif // PARSER_TOML_H
