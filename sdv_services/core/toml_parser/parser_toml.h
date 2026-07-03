/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Martin Stimpfl - initial API and implementation
 *   Erik Verhoeven - writing TOML and whitespace preservation
 ********************************************************************************/

#ifndef PARSER_TOML_H
#define PARSER_TOML_H

#include "lexer_toml.h"
#include "parser_node_toml.h"
#include "miscellaneous.h"
#include "parser_node_indexer.h"
#include <stack>
#include <memory>
#include <string>

/// The TOML parser namespace
namespace toml_parser
{
    // Forward declarations
    class CNode;
    class CNodeCollection;

    /**
     * @brief Creates a tree structure from input of UTF-8 encoded TOML source data
     */
    class CParser : public sdv::IInterfaceAccess, public sdv::toml::ITOMLParser
    {
    public:
        // Forward declaration
        class CLockRebuild;
        friend CLockRebuild;        ///< Friend class can trigger manage rebuild lock counter.

        /**
         * @brief Construct a new Parser object
         * @param[in] rssString UTF-8 encoded data of a TOML source
         */
        CParser(const std::string& rssString = std::string());

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
         * @brief Get the indexer object managing the overall order of the nodes.
         * @return Reference to the index object.
        */
        CNodeIndexer& Indexer();

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

        /**
         * @brief Lock rebuild object preventing rebuilding the node order of all the tables.
         */
        class CLockRebuild
        {
            friend CParser;     ///< Parser can access the constructor

            /**
             * @brief Constructor
             * @param[in] rParser Reference to the parser object
             */
            CLockRebuild(CParser& rParser);

        public:
            /**
             * @brief Copy constructor
             * @param[in] rLockRebuild Reference to another rebuild lock object.
             */
            CLockRebuild(const CLockRebuild& rLockRebuild);

            /**
             * @brief Move constructor
             * @param[in] rLockRebuild Reference to another rebuild lock object.
             */
            CLockRebuild(CLockRebuild&& rLockRebuild);

            /**
             * @brief Destructor
             */
            ~CLockRebuild();

        private:
            CParser&    m_rParser;  ///< Reference to the parser object
        };

        /**
         * @brief Create a rebuild lock object. During the lifetime of the object rebuilding the node order is locked using the lock
         * counter method.
         * @return An instance of the rebuild lock object.
         */
        CLockRebuild CreateRebuildLockObject();

        /**
         * @brief Returns whether rebuild is locked at the moment.
         * @return Set when rebuild is locked.
         */
        bool RebuildLocked() const;

    private:
        /**
         * @brief Increase the rebuild lock counter. A lock count larger than 0 will prevent a rebuild.
         */
        void IncrRebuildLockCnt();

        /**
         * @brief Decrease the rebuild lock counter. A lock count of 0 will trigger the rebuild.
         */
        void DecrRebuildLockCnt();

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
            none,               ///< No nested environment (used as default environment).
            array,              ///< Environment for an array
            inline_table        ///< Environment for a table
        };

        enum_stack<EEnvironment, EEnvironment::none> m_stackEnvironment;    ///< Tracking of environments in nested structures.
        std::shared_ptr<CRootTable>             m_ptrRoot;                  ///< The one root node.
        std::shared_ptr<CNodeCollection>        m_ptrCurrentCollection;     ///< The current collection node.
        CLexer                                  m_lexer;                    ///< Lexer object user for lexing the TOML code.
        CNodeIndexer                            m_indexer;                  ///< Indexer managing the oberall order of the nodes.
        size_t                                  m_nRebuildLockCnt = 0;      ///< A lock counter > 0 will prevent the node order
                                                                            ///< rebuild.
    };
} // namespace toml_parser

#endif // PARSER_TOML_H
