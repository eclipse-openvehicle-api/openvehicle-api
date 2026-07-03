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

#ifndef PARSER_NODE_TOML_H
#define PARSER_NODE_TOML_H

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <map>

#include <support/interface_ptr.h>
#include "miscellaneous.h"
#include "code_snippet.h"
#include "parser_node_indexer.h"

/// The TOML parser namespace
namespace toml_parser
{
    // Forward declaration
    class CArray;
    class CTable;
    class CTableArray;
    class CParser;
    class CNode;
    class CNodeCollection;

    /**
     * @brief TOML generation options.
     */
    enum class EGenerateOptions : uint32_t
    {
        inline_when_possible   = 0x01,  ///< Try to generate as much as possible as inline nodes.
        explicit_when_possible = 0x02,  ///< Try to generate as much as possible as explicit nodes.
        no_comments            = 0x10,  ///< Do not include comments.
        reduce_whitespace      = 0x20,  ///< Add comments, but reduce extra newlines before and after the node.
        full_header            = 0x40,  ///< When generating tables or table arrays, include the header in generated code.
    };

    /**
     * @brief Generation context information, allowing the generation to take place from the root node as well as from any node
     * collection that contains sub nodes.
     */
    class CGenContext
    {
    public:
        /**
         * @brief Default constructor.
         * @param[in] rssPrefixKey Reference to the prefix key that is used as a parent during the generation.
         * @param[in] uiOptions The initial options to set.
         */
        CGenContext(const std::string& rssPrefixKey = std::string(), uint32_t uiOptions = 0);

        /**
         * @brief Called by the node that is generating the TOML. If not initialized before, extract the context from the node and
         * assign this node as top node for the code generation.
         * @param[in] rptrNode Reference to the node that could be used for initialization as top most node.
         */
        void InitTopMostNode(const std::shared_ptr<const CNode>& rptrNode);

        /**
         * @brief A node is part of the view if either it is an inline node, or it is a standard node and the current parent is the
         * root view.
         * @param[in] rptrNode Reference to the smart pointer to the node to check for.
         * @return Returns whether the node is part of the view.
         */
        bool IsPartOfView(const std::shared_ptr<const CNode>& rptrNode) const;

        /**
         * @brief Create a copy of the context class with a new key context.
         * @param[in] rssNewKeyContext Reference to the string containing the new key context.
         * @param[in] rptrNode Reference to the node pointer to extract the context from.
         * @param[in] bLastNode When set, this is the last node in the current view.
         * @return The copy of the contetx class.
         */
        CGenContext CopyWithContext(const std::string& rssNewKeyContext, const std::shared_ptr<CNode>& rptrNode,
            bool bLastNode) const;

        /**
         * @brief Get the stored prefix key that should be used for the TOML code generation.
         * @return Reference to the prefix key string.
         */
        const std::string& PrefixKey() const;

        /**
         * @brief Get the stored key context.
         * @return Reference to the key context string.
         */
        const std::string& KeyContext() const;

        /**
         * @brief The key path composed of the prefix and the relative key path.
         * @return Reference to the key path string.
         */
        const std::string& KeyPath() const;

        /**
         * @brief The key path composed of the key kontext and the relative key path.
         * @return Reference to the key path string.
         */
        const std::string& FullKeyPath() const;

        /**
         * @brief The relative key path, relative to the current context.
         * @return Reference to the key path string.
         */
        const std::string& RelKeyPath() const;

        /**
         * @brief Is this the top most node?
         * @return Returns when the node is the top most node.
         */
        bool TopMostNode() const;

        /**
         * @brief Set a generation option.
         * @param[in] eOption Option to set.
         */
        void SetOption(EGenerateOptions eOption);

        /**
         * @brief Check whether a generation option has been set.
         * @param[in] eOption Option to set.
         * @return Returns whether the option was set.
         */
        bool CheckOption(EGenerateOptions eOption) const;

        /**
         * @brief Is the last-node-flag set?
         * @return Returns whether the last-node-flag has been set indicating the node using this context to be the last node within
         * the current view.
         */
        bool LastNode() const;

        /**
         * @brief Node presentation form.
         */
        enum class EPresentation
        {
            standard,        ///< Standard presentation (root or within table/table-array)
            standard_inline, ///< Inline presentation (root or within table/table-array)
            embedded,        ///< Embedded presentation (within array or inline-table)
        };

        /**
         * @brief Get the presentation form of the node.
         * @return The node presentation extracted from the node and the generation context.
         */
        EPresentation Presentation() const;

        /**
         * @brief Is the node a standard node?
         * @return Returns 'true' when the node is a standard node.
         */
        bool Standard() const;

        /**
         * @brief Is the node an inline node?
         * @remarks Embedded nodes are also inline.
         * @return Returns 'true' when the node is an inline node.
         */
        bool Inline() const;

        /**
         * @brief Is the node an embedded node (within an inline table or array)?
         * @return Returns 'true' when the node is an embedded node.
         */
        bool Embedded() const;

        /**
         * @brief Does the node need an assignment (key and when inline, equal sign)?
         * @remarks Embedded nodes within an array do not need an assignment.
         * @return Returns 'true' when the node needs an assignment.
         */
        bool Assignment() const;

        /**
         * @brief For an embedded node, is a comma indicating the next node needed?
         * @remarks Some inline arrays can have a final comma behind the last embedded node.
         * @return Returns 'true' when a comma is needed.
         */
        bool CommaNeeded() const;

        /**
         * @brief Are comments and newlines allowed? For an embedded node, this might be prohibited. But can also explicitly be
         * defined in the context.
         * @remarks Inline tables require a one line definition for the embedded nodes.
         * @return Returns 'true' if comments and newlines are allowed.
         */
        bool CommentAndNewlineAllowed() const;

        /**
         * @brief Are newlines allowed? For an embedded node, this might be prohibited. But can also explicitly be defined in the
         * context.
         * @remarks Inline tables require a one line definition for the embedded nodes.
         * @return Returns 'true' if comments and newlines are allowed.
         */
        bool NewlineAllowed() const;

        /**
         * @brief For a standard (inline) node, is a newline required at the end of the node definition?
         * @returns Returns'true' if a newline is required behind the node definition.
         */
        bool FinalNewline() const;

    private:
        void ExtractContext(const std::shared_ptr<const CNode>& rptrNode);

        std::shared_ptr<const CNode> m_ptrTopMostNode;                  ///< Top most node that is used for the generation. The
                                                                        ///< parent nodes of the top most node will not be part of
                                                                        ///< the node generation and if they contain child nodes in
                                                                        ///< their view, the nodes are printed by their parent and
                                                                        ///< not by their view.
        std::string         m_ssPrefixKey;                              ///< Prefix key to be used during the generation of the TOML
                                                                        ///< not by their view.
        std::string         m_ssKeyContext2;
        std::string         m_ssKeyContext;                             ///< String containing the current context. The string must
                                                                        ///< follow the key rules for separation with bare, literal
                                                                        ///< and quoted keys.
        std::string         m_ssKeyPath;                                ///< The key path composed of the prefix and the relative
                                                                        ///< key path.
        std::string         m_ssFullKeyPath;                            ///< The full key path composed of the key context and the
                                                                        ///< relative key path.
        std::string         m_ssRelKeyPath;                             ///< The relative key path, relative to the current context.
        uint32_t            m_uiOptions = 0;                            ///< Zero or more options to take into account when creating
                                                                        ///< the text to the TOML nodes.
        bool                m_bTopMost = true;                          ///< Set when this context is the top most context.
        bool                m_bLastNode = false;                        ///< Is this the last node in the current view?
        bool                m_bFinalLastNode = false;                   ///< When set, this is the last (top level) node of the node
                                                                        ///< hierarchy. Only child nodes can still follow.
        EPresentation       m_ePresentation = EPresentation::standard;  ///< Presentation of the node.
        bool                m_bOneLine = false;                         ///< Set when the node is not allowed to cover more than
                                                                        ///< one line (except when using multi-line strings).
        bool                m_bAssignment = false;                      ///< Does the node need an assignment.
        bool                m_bCommaNeeded = false;                     ///< Is a comma needed following the node definition?
        bool                m_bFinalNewline = false;                    ///< Is a final newline behind the definition required?
    };

    /**
     * @brief Node to build up the parse tree
     */
    class CNode :
        public std::enable_shared_from_this<CNode>, public sdv::IInterfaceAccess, public sdv::toml::INodeInfo,
        public sdv::toml::INodeUpdate
    {
    protected:
        /**
         * @brief Constructs a new generic node object.
         * @param[in] rparser Reference to the TOML parser.
         * @param[in] rssName Reference to the name of the node.
         * @param[in] rssRawName Reference to the raw name of the node.
         */
        CNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName);

    public:
        /**
         * @brief Deleted since Nodes should only be handled via smart-pointer
         * @{
         */
        CNode(const CNode&) = delete;
        CNode& operator=(const CNode&) = delete;
        CNode(const CNode&&) = delete;
        CNode& operator=(const CNode&&) = delete;
        /**
         * @}
         */

        /**
         * @brief Destroy the node object
         */
        ~CNode();

        // Interface map
        BEGIN_SDV_INTERFACE_MAP()
            SDV_INTERFACE_ENTRY(sdv::toml::INodeInfo)
            SDV_INTERFACE_ENTRY(sdv::toml::INodeUpdate)
        END_SDV_INTERFACE_MAP()

        /**
         * @{ 
         * @brief Get a reference to the TOML parser that generated this node.
         * @return Reference to the TOML parse.
         */
        CParser& Parser();
        const CParser& Parser() const;
        /**
         * @}
         */

        /**
         * @{
         * @brief Return the index object of the node.
         * @return Reference to the node index object.
         */
        const CNodeIndex& NodeIndex() const;
        CNodeIndex& NodeIndex();
        /**
         * @}
         */

        /**
         * @brief Compare the index position of this node with the position of another node and return whether this node occurs
         * before the other node.
         * @param[in] rNode Reference to the node to compare the position with.
         * @return Returns whether this node occurs before the other node.
         */
        bool operator<(const CNode& rNode) const;

        /**
         * @brief Compare the index position of this node with the position of another node and return whether this node occurs
         * before the other node.
         * @param[in] rptrNode Reference to the node to compare the position with.
         * @return Returns whether this node occurs before the other node.
         */
        bool operator<(const std::shared_ptr<CNode>& rptrNode) const;

        /**
         * @brief Get the node name (no conversion to a literal or quoted key is made). Overload of sdv::toml::INodeInfo::GetName.
         * @return String containing the name of the node.
         */
        virtual sdv::u8string GetName() const override;

        /**
         * @brief Get the node path following the key rules for bar, literal and quoted keys. Overload of
         * sdv::toml::INodeInfo::GetPath.
         * @param[in] bResolveArrays When set, include array indices in the path. The path returned without array indices is
         * identical to the code in the TOML file. The path returned with array indices is identical to the direct access of
         * nodes within the parser.
         * @return String containing the path of the node.
         */
        virtual sdv::u8string GetPath(bool bResolveArrays) const override;

        /**
         * @brief Get the raw node path using the original keys if available.
         * @param[in] bResolveArrays When set, include array indices in the path. The path returned without array indices is
         * identical to the code in the TOML file. The path returned with array indices is identical to the direct access of
         * nodes within the parser.
         * @return List containing the keys pairs for each parent until this node. Each key pair contain the name (first) and the
         * raw name as it was defined (second).
         */
        std::list<std::pair<std::string, std::string>> GetRawPath(bool bResolveArrays) const;

        /*
         * @brief The node value. Overload of sdv::toml::INodeInfo::GetValue.
         * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
         * returns empty.
         */
        virtual sdv::any_t GetValue() const override;

        /**
         * @brief Get the index of this node within the parent collection. Overload of sdv::toml::INodeInfo::GetIndex.
         * @return The index of the node within the parent collection node or npos when no parent is available.
         */
        virtual uint32_t GetIndex() const override;

        /**
         * @brief Get the parent collection node. Overload of sdv::toml::INodeInfo::GetParent.
         * @return Returns the parent collection node or NULL when there is no parent collection node.
         */
        virtual sdv::IInterfaceAccess* GetParent() const override;

        /**
         * @brief The node value. Overload of sdv::toml::INodeInfo::GetTOML.
         * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
         * returns empty.
         */
        virtual sdv::u8string GetTOML() const override;

        /**
         * @brief Set or replace a comment for the node. Overload of sdv::toml::INodeInfo::SetComment.
         * @details Set the comment text for the node. If a comment is provided as text (normal behavior), the comment text will
         * be formatted automatically when generating the TOML text. If the comment text should not contain the comment
         * character '#' before the comment text.
         * Comments inserted before the node will be inserted on the line before the node unless the comment is provided in raw
         * format and is ended with a line-break and optionally whitespace. Comment inserted behind the node will be inserted on
         * the same line as the node.
         * Comments provided as text is automatically wrapped to 132 characters if possible. Line-breaks in the text will cause
         * a new comment line to start.
         * @param[in] eType The comment type to set the comment text for.
         * @param[in] ssComment String containing the comment text to set.
         */
        virtual void SetComment(sdv::toml::INodeInfo::ECommentType eType, const sdv::u8string& ssComment) override;

        /**
         * Get the current comment for the node. Overload of sdv::toml::INodeInfo::GetComment.
         * @param[in] eType The comment type to get the comment text of.
         * @return String with the comment text or an empty string if no comment is available.
         */
        virtual sdv::u8string GetComment(sdv::toml::INodeInfo::ECommentType eType) override;

        /**
         * @brief Format the node automatically, remove redundant whitespace. Overload of sdv::toml::INodeInfo::AutomaticFormat.
         * @param[in] bRemoveComments When set, the comments are removed from the node.
         */
        virtual void AutomaticFormat(/*in*/ bool bRemoveComments) override;

        /**
         * @brief Is the node inline? Overload of sdv::toml::INodeInfo::IsInline.
         * @return Returns whether the node is defined as inline node.
         */
        virtual bool IsInline() const override;

        /**
         * @brief Is the node defined as standard node? Overload of sdv::toml::INodeInfo::IsStandard.
         * @return Returns whether the node is defined as standard node.
         */
        virtual bool IsStandard() const override;

        /**
         * @brief Update the node with TOML code information. The default implementation takes the comment and whitespace around the
         * node and stores this for node reconstruction.
         * @param[in] rNodeRange Reference to the node range information containing the tokens for the code snippets.
         */
        virtual void UpdateNodeCode(const CNodeTokenRange& rNodeRange);

        /**
         * @brief Change the key name of the node (if the node is not a value node of an array). Overload of
         * sdv::toml::INodeUpdate::ChangeName.
         * @param[in] ssNewName The name to assign to the node. The name must adhere to the key names defined by the TOML
         * specification. Defining the key multiple times is not allowed. Quotation of key names is done automatically; the
         * parser decides itself whether the key is bare-key, a literal key or a quoted key.
         * @return Returns whether the name change was successful.
         */
        virtual bool ChangeName(const sdv::u8string& ssNewName) override;

        /**
         * @brief Change the value of the node. Overload of sdv::toml::INodeUpdate::ChangeValue.
         * @remarks Only valid for value nodes. Changing the value type is not supported.
         * @param[in] anyNewValue The value of the node, being either an integer, floating point number, virtual bool value or a
         * string. Conversion is automatically done to int64, double float, bool or u8string.
         * @return Returns whether the value change was successful.
         */
        virtual bool ChangeValue(sdv::any_t anyNewValue) override;

        /**
         * @brief Move up the node in the collection. Overload of sdv::toml::INodeUpdate::MoveUp.
         * @remarks External tables or table arrays cannot be moved before value nodes.
         * @remarks Moving if the node is the first node is not possible.
         * @return Returns whether the move was successful.
         */
        virtual bool MoveUp() override;

        /**
         * @brief Move down the node in the collection. Overload of sdv::toml::INodeUpdate::MoveDown.
         * @remarks Value nodes cannot be moved behind external tables or table arrays.
         * @remarks Moving if the node is the last node is not possible.
         * @return Returns whether the move was successful.
         */
        virtual bool MoveDown() override;

        /**
         * @brief Delete the current node. Overload of sdv::toml::INodeUpdate::DeleteNode.
         * @attention A successful deletion will cause all interfaces to the current node to become inoperable.
         * @return Returns whether the deletion was successful.
         */
        virtual bool DeleteNode() override;

        /**
         * @brief Is this node marked as deleted?
         * @return Returns whether this node has been deleted.
         */
        bool IsDeleted() const;

        /**
         * @brief Do a dynamic cast to one of the base types of the node.
         * @return Casted shared pointer to the base type if the type is valid, or an empty pointer if not.
         */
        template <typename TNodeType>
        std::shared_ptr<TNodeType> Cast();

        /**
         * @brief Do a dynamic cast to one of the base types of the node.
         * @return Casted shared pointer to the base type if the type is valid, or an empty pointer if not.
         */
        template <typename TNodeType>
        std::shared_ptr<const TNodeType> Cast() const;

        /**
         * @brief Reassign the parent node and if necessary the parser reference.
         * @details This function allows shifting nodes from one parser to another.
         * @param[in] rptrParent Reference to the node to assign to this node as a parent.
         */
        virtual void SetParentPtr(const std::shared_ptr<CNodeCollection>& rptrParent);

        /**
         * @brief Gets the parent node pointer.
         * @return Returns the parent node pointer or an empty pointer when no parent was assigned or the stored weak pointer could
         * not be locked.
         */
        std::shared_ptr<CNodeCollection> GetParentPtr() const;

        /**
         * @brief Get the parent path of the node.
         * @return Return the parent path if existining and not a root.
         */
        std::string GetParentPath() const;

        /**
         * @brief Accesses a node by its key in the parse tree.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @attention For an array, when no indexing is supplied, the latest entry will be returned.
         * @param[in] rssPath The path of the node to searched for.
         * @return Returns a shared pointer to the wanted node if it was found or a node with invalid content if it was not found.
         */
        virtual std::shared_ptr<CNode> Direct(const std::string& rssPath) const = 0;

        /**
         * @brief Create the TOML text based on the content using an optional prefix node.
         * @param[in] rContext Reference to the context class to use during TOML code generation.
         * @return TOML text string.
         */
        virtual std::string GenerateTOML(const CGenContext& rContext = CGenContext()) const = 0;

        // White space and comment preservation indices for code generation.
        const size_t m_nPreNodeCode = 0;            ///< Code snippet before the node. Corresponds to
                                                    ///< sdv::toml::INodeInfo::ECommentFlags::comment_before.
        const size_t m_nPostNodeCode = 1;           ///< Comment behind the node. Corresponds to
                                                    ///< sdv::toml::INodeInfo::ECommentFlags::comment_behind.
        const size_t m_nOutOfScopeCodeBefore = 2;   ///< Out of scope comment before the node. Corresponds to
                                                    ///< sdv::toml::INodeInfo::ECommentFlags::out_of_scope_comment_before.
        const size_t m_nOutOfScopeCodeBehind = 3;   ///< Out of scope comment behind the node. Corresponds to
                                                    ///< sdv::toml::INodeInfo::ECommentFlags::out_of_scope_comment_behind.
        const size_t m_nPreKeyCode = 4;             ///< Code snippet before the key. Is stored in combination with the
                                                    ///< corresponding key.
        const size_t m_nPostKeyCode = 5;            ///< Code Snippet behind the key. Is stored in combination with the
                                                    ///< corresponding key.
        const size_t m_nPreValueCode = 6;           ///< Code snippet before the value.
        const size_t m_nPostValueCode = 7;          ///< Code snippet behind the value.
        const size_t m_nPostValuesArray = 8;        ///< Code snippet at the end of the array values, before the closing character.

        /**
         * @brief Get the code snippet.
         * @param[in] nIndex The comment type index to get the code for.
         * @param[in] rssKey Reference to the key to be used for code snippet identification.
         * @return Reference to the comment structure of the comment. If the provided index is not available in the vector,
         * returns an empty code snippet.
         */
        const CCodeSnippet& CodeSnippet(size_t nIndex, const std::string& rssKey = std::string()) const;

        /**
         * @brief Get the code snippet (write access).
         * @param[in] nIndex The comment type index to get the code for.
         * @param[in] rssKey Reference to the key to be used for code snippet identification.
         * @return Reference to the comment structure of the comment.
         */
        CCodeSnippet& CodeSnippet(size_t nIndex, const std::string& rssKey = std::string());

        /**
         * @brief Get the code snippet using the comment type.
         * @param[in] eType The comment type to get the code for.
         * @return Reference to the comment structure of the comment. If the provided index is not available in the vector,
         * returns an empty code snippet.
         */
        const CCodeSnippet& CodeSnippet(sdv::toml::INodeInfo::ECommentType eType) const;

        /**
         * @brief Get the code snippet (write access) using the comment type.
         * @param[in] eType The comment type to get the code for.
         * @return Reference to the comment structure of the comment. If the provided index is not available in the vector,
         * returns an empty code snippet.
         */
        CCodeSnippet& CodeSnippet(sdv::toml::INodeInfo::ECommentType eType);

        /**
         * @brief Compose a custom path from the node key path using a key prefix and a context.
         * @param[in] rssPrefixKey The prefix to insert at as a base to the key tree.
         * @param[in] rssContext The context that is used to define the relative portion of the key. To determine the relative
         * portion, the context string contains the same prefix as is supplied in rssPrefixKey.
         * @return Returns the custom path composed of the prefix and the relative portion of the original path.
         */
        std::string GetCustomPath(const std::string& rssPrefixKey, const std::string& rssContext) const;

        /**
         * @brief When the parent changes (e.g. when moving items from one parser to the other), the items and all its sub-items
         * need to reasign the parser.
         * @param[in] rParser Reference to the parser to assign.
         */
        virtual void ReassignParser(CParser& rParser);

    private:
        CNodeIndex                          m_index;            ///< Node index object holding the overall node position.
        std::weak_ptr<CNodeCollection>      m_ptrParent;        ///< Weak pointer to the parent node (if existing).
        std::string                         m_ssName;           ///< Name of the node.
        std::string                         m_ssRawName;        ///< Raw name of the node.
        bool                                m_bDeleted = false; ///< Enabled when the node was marked for deletion.
        std::reference_wrapper<CParser>     m_refParser;        ///< Reference to the TOML parser.
        std::vector<std::map<std::string, CCodeSnippet>> m_vecCodeSnippets; ///< Vector with comments/code snippets.

    public:
        /**
         * @brief The derived class from the node collection can be inline or not.
         * @return Returns whether the node is an inline node.
         */
        virtual bool Inline() const = 0;

        /**
         * @brief With some node collections it is possible to switch between inline and normal.
         * @remarks Additional node composition information will be removed and the order within the parent node might be changed.
         * @remarks When made inline, all child nodes must be made inline as well. When made standard, only this node is made
         * standard.
         * @remarks Making this node a standard node, this is only possible when the parent is not an inline mode.
         * @param[in] bInline When set, try to switch to inline. Otherwise try to switch to normal.
         * @param[in] bIncludeChildren When set and bInline is not set, applicable child nodes are converted as well (only tables
         * and table-arrays can be defined as standard). Making a node inline is always including the children.
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline, bool bIncludeChildren = true) = 0;

        /**
         * @brief Checks whether the table was explicitly defined.
         * @return Returns the explicit definition flag.
         */
        virtual bool ExplicitlyDefined() const;

        /**
         * @brief If the table was an implicit definition, make it explicit.
         */
        virtual void MakeExplicit();
    };

    /**
     * @brief Class implementing the value node.
     * @details A value always is inline. Two versions exist: the assignment version and the embedded in-an-array-version.
     * Not embedded in an array (with assignment):
     * @code
     *      <whitespace>KEY<whitespace>.<whitespace>KEY<whitespace>=<whitespace>VALUE<whitespace>
     *        ^              ^            ^              ^            ^                ^
     *        pre-node       post_key_ws  pre_key_ws     post_key_ws  pre_value_ws     post-node
     * @endcode
     * Embedded in an array (without assignment):
     * @code
     *      <whitespace>VALUE<whitespace>
     *        ^                ^
     *        pre-node         post-node
     * @endcode
     * The pre- and post-node whitespace areas are covered by the UpdateNodeCode function of the CNode base class.
     * Multiple optional pre- and post-key whitespace areas can be defined.
     */
    class CValueNode : public CNode
    {
    protected:
        /**
         * @brief Constructs a new node object.
         * @param[in] rparser Reference to the TOML parser.
         * @param[in] rssName Reference to the name of the node.
         * @param[in] rssRawName Reference to the raw name of the node.
         * @param[in] rssRawValue Reference to the raw value string.
         */
        CValueNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName, const std::string& rssRawValue);

        /**
         * @brief The derived class from the node collection can be inline or not. Overload of CNode::Inline.
         * @return Returns whether the node is an inline node.
         */
        virtual bool Inline() const override;

        /**
         * @brief With some node collections it is possible to switch between inline and normal. Overload of CNode::Inline.
         * @remarks Additional node composition information will be removed and the order within the parent node might be changed.
         * @param[in] bInline When set, try to switch to inline. Otherwise try to switch to normal.
         * @param[in] bIncludeChildren When set and bInline is not set, applicable child nodes are converted as well (only tables
         * and table-arrays can be defined as standard). Making a node inline is always including the children.
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline, bool bIncludeChildren = true) override;

        /**
         * @brief Accesses a node by its key in the parse tree. Overload of CNode::Direct.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @attention For an array, when no indexing is supplied, the latest entry will be returned.
         * @param[in] rssPath The path of the node to searched for.
         * @return Returns a shared pointer to the wanted node if it was found or a node with invalid content if it was not found.
         */
        virtual std::shared_ptr<CNode> Direct(const std::string& rssPath) const override;

        /**
         * @brief Create the TOML text based on the content using an optional prefix node. Overload of CNode::GenerateTOML.
         * @param[in] rContext Reference to the context class to use during TOML code generation.
         * @return The TOML text string.
         */
        virtual std::string GenerateTOML(const CGenContext& rContext = CGenContext()) const override;

        /**
         * @brief Get the value in text form.
         * @return String with the value in text form.
         */
        virtual std::string ValueText() const = 0;

        /**
         * @brief Update the node with TOML code information. Overload of CNode::UpdateNodeCode.
         * @param[in] rNodeRange Reference to the node range information containing the tokens for the code snippets.
         */
        virtual void UpdateNodeCode(const CNodeTokenRange& rNodeRange) override;

        /**
         * @brief Get the raw value text.
         * @return The raw value text of the original value token. Or if not existing, the value text.
         */
        std::string RawValueText() const;

    protected:
        /**
         * @brief When updating the node, reset the raw value text.
         */
        void ResetRawValueText();

    private:
        std::string m_ssRawValue;                   ///< Raw value string.
    };

    /**
     * @brief Boolean value node.
     */
    class CBooleanNode : public CValueNode
    {
    public:
        /**
         * @brief Constructor
         * @param[in] rparser Reference to the TOML parser.
         * @param[in] rssName Reference to the string containing the name of the node.
         * @param[in] rssRawName Reference to the raw name of the node.
         * @param[in] bVal The value to assign.
         * @param[in] rssRawValue Reference to the raw value string.
         */
        CBooleanNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName, bool bVal,
            const std::string& rssRawValue);

        /**
         * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
         * @return Type of the node.
         */
        virtual sdv::toml::ENodeType GetType() const override;

        /**
         * @brief The node value. Overload of sdv::toml::INodeInfo::GetValue.
         * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
         * returns empty.
         */
        virtual sdv::any_t GetValue() const override;

        /**
         * @brief Change the value of the node. Overload of sdv::toml::INodeUpdate::ChangeValue.
         * @remarks Only valid for value nodes. Changing the value type is not supported.
         * @param[in] anyNewValue The value of the node, being either an integer, floating point number, virtual bool value or a
         * string. Conversion is automatically done to int64, double float, bool or u8string.
         * @return Returns whether the value change was successful.
         */
        virtual bool ChangeValue(sdv::any_t anyNewValue) override;

        /**
         * @brief Get the value in text form. Overload of CNodeValue::ValueText.
         * @return String with the value in text form.
         */
        virtual std::string ValueText() const override;

    private:
        bool    m_bVal = false;     ///< Value in case of virtual bool node.
    };

    /**
     * @brief Integer value node.
     */
    class CIntegerNode : public CValueNode
    {
    public:
        /**
         * @brief Constructor
         * @param[in] rparser Reference to the TOML parser.
         * @param[in] rssName Reference to the string containing the name of the node.
         * @param[in] rssRawName Reference to the raw name of the node.
         * @param[in] iVal The value to assign.
         * @param[in] rssRawValue Reference to the raw value string.
         */
        CIntegerNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName, int64_t iVal,
            const std::string& rssRawValue);

        /**
         * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
         * @return Type of the node.
         */
        virtual sdv::toml::ENodeType GetType() const override;

        /**
         * @brief The node value. Overload of sdv::toml::INodeInfo::GetValue.
         * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
         * returns empty.
         */
        virtual sdv::any_t GetValue() const override;

        /**
         * @brief Change the value of the node. Overload of sdv::toml::INodeUpdate::ChangeValue.
         * @remarks Only valid for value nodes. Changing the value type is not supported.
         * @param[in] anyNewValue The value of the node, being either an integer, floating point number, virtual bool value or a
         * string. Conversion is automatically done to int64, double float, bool or u8string.
         * @return Returns whether the value change was successful.
         */
        virtual bool ChangeValue(sdv::any_t anyNewValue) override;

        /**
         * @brief Get the value in text form. Overload of CNodeValue::ValueText.
         * @return String with the value in text form.
         */
        virtual std::string ValueText() const override;

    private:
        int64_t     m_iVal = 0;     ///< Value in case of integer node.
    };

    /**
     * @brief Floating point value node.
     */
    class CFloatingPointNode : public CValueNode
    {
    public:
        /**
         * @brief Constructor
         * @param[in] rparser Reference to the TOML parser.
         * @param[in] rssName Reference to the string containing the name of the node.
         * @param[in] rssRawName Reference to the raw name of the node.
         * @param[in] dVal The value to assign.
         * @param[in] rssRawValue Reference to the raw value string.
         */
        CFloatingPointNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName, double dVal,
            const std::string& rssRawValue);

        /**
         * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
         * @return Type of the node.
         */
        virtual sdv::toml::ENodeType GetType() const override;

        /**
         * @brief The node value. Overload of sdv::toml::INodeInfo::GetValue.
         * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
         * returns empty.
         */
        virtual sdv::any_t GetValue() const override;

        /**
         * @brief Change the value of the node. Overload of sdv::toml::INodeUpdate::ChangeValue.
         * @remarks Only valid for value nodes. Changing the value type is not supported.
         * @param[in] anyNewValue The value of the node, being either an integer, floating point number, virtual bool value or a
         * string. Conversion is automatically done to int64, double float, bool or u8string.
         * @return Returns whether the value change was successful.
         */
        virtual bool ChangeValue(sdv::any_t anyNewValue) override;

        /**
         * @brief Get the value in text form. Overload of CNodeValue::ValueText.
         * @return String with the value in text form.
         */
        virtual std::string ValueText() const override;

    private:
        double      m_dVal = 0.0;       ///< Value in case of floating point node.
    };

    /**
     * @brief String value node.
     */
    class CStringNode : public CValueNode
    {
    public:
        /**
         * @brief Quotation type used for the string.
         */
        enum EQuotationType
        {
            quoted_string,      ///< String was a quoted string (default)
            literal_string,     ///< String was a literal string.
            multi_line_quoted,  ///< Multiple line quoted string.
            multi_line_literal, ///< Multiple line lteral string.
        };
        
        /**
         * @brief Constructor
         * @param[in] rparser Reference to the TOML parser.
         * @param[in] rssName Reference to the string containing the name of the node.
         * @param[in] rssRawName Reference to the raw name of the node.
         * @param[in] rssVal The value to assign.
         * @param[in] eQuotationType Type of quotation used for the value.
         * @param[in] rssRawValue Reference to the raw value string.
         */
        CStringNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName, const std::string& rssVal,
            EQuotationType eQuotationType, const std::string& rssRawValue);

        /**
         * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
         * @return Type of the node.
         */
        virtual sdv::toml::ENodeType GetType() const override;

        /**
         * @brief The node value. Overload of sdv::toml::INodeInfo::GetValue.
         * @return For boolean, integer, floating point and strings, the function returns a value. Otherwise the function
         * returns empty.
         */
        virtual sdv::any_t GetValue() const override;

        /**
         * @brief Change the value of the node. Overload of sdv::toml::INodeUpdate::ChangeValue.
         * @remarks Only valid for value nodes. Changing the value type is not supported.
         * @param[in] anyNewValue The value of the node, being either an integer, floating point number, virtual bool value or a
         * string. Conversion is automatically done to int64, double float, bool or u8string.
         * @return Returns whether the value change was successful.
         */
        virtual bool ChangeValue(sdv::any_t anyNewValue) override;

        /**
         * @brief Get the value in text form. Overload of CNodeValue::ValueText.
         * @return String with the value in text form.
         */
        virtual std::string ValueText() const override;

    private:
        std::string     m_ssVal;                                            ///< Value in case of string or illegal (error) node.
        EQuotationType  m_eQuotationType = EQuotationType::quoted_string;   ///< Quotation type of the string.
    };

    /**
     * @brief Base structure for arrays and tables.
     */
    class CNodeCollection : public CNode, public sdv::toml::INodeCollection, public sdv::toml::INodeCollectionInsert,
        public sdv::toml::INodeCollectionConvert
    {
        // Friend class CNode.
        friend CNode;
    protected:
        /**
         * @brief Constructor
         * @param[in] rparser Reference to the TOML parser.
         * @param[in] rssName Reference to the name of the node.
         * @param[in] rssRawName Reference to the raw name of the node.
         */
        CNodeCollection(CParser& rparser, const std::string& rssName, const std::string& rssRawName);

    public:
        // Interface map
        BEGIN_SDV_INTERFACE_MAP()
            SDV_INTERFACE_ENTRY(sdv::toml::INodeCollection)
            SDV_INTERFACE_ENTRY(sdv::toml::INodeCollectionInsert)
            SDV_INTERFACE_ENTRY(sdv::toml::INodeCollectionConvert)
            SDV_INTERFACE_CHAIN_BASE(CNode)
        END_SDV_INTERFACE_MAP()

        /**
         * @brief Format the node automatically, remove redundant whitespace. Overload of sdv::toml::INodeInfo::AutomaticFormat.
         * @param[in] bRemoveComments When set, the comments are removed from the node.
         */
        virtual void AutomaticFormat(/*in*/ bool bRemoveComments) override;

        /**
         * @brief Returns the amount of nodes. Overload of sdv::toml::INodeCollection::GetCount.
         * @return The amount of nodes.
         */
        virtual uint32_t GetCount() const override;

        /**
         * @brief Get the node. Overload of sdv::toml::INodeCollection::GetNode.
         * @param[in] uiIndex Index of the node to get.
         * @return Interface to the node object.
         */
        virtual IInterfaceAccess* GetNode(/*in*/ uint32_t uiIndex) const override;

        /**
         * @brief Get the node.
         * @param[in] uiIndex Index of the node to get.
         * @return Smart pointer to the node object.
         */
        virtual std::shared_ptr<CNode> Get(uint32_t uiIndex) const;

        /**
         * @brief After every insert, deletion and shift, the node order of this and all sub- tables need to be rebuild.
         * @param[in] bForce Force rebuild, even if locked.
         */
        virtual void RebuildNodeOrder(bool bForce);

        /**
         * @brief After every insert, deletion and shift, the node order needs to be rebuild.
         * @details Adding is done iteratively through the node list. In case the top level flag is set, all nodes are added that
         * are marked explicit or if a node is implicit, the explicit sub-nodes are added. In case this is root view, all sub-nodes
         * which are standard tables or standard table arrays are added. This flattens the hierarchy. When the root view is not set,
         * only the tables and table arrays are added that are a direct child of the node collection. At the end of building the
         * root view, the nodes are sorted using their index. Furthermore, inline nodes are moved to the beginning of the vector and
         * standard nodes to the end.
         * @param[in, out] rvecNodes Reference to the vector being filled with the nodes for the view.
         * @param[in] bRootView Set when the count is top level.
         * @param[in] bTopLevel Set when this is the top level node for adding sub nodes.
         */
        virtual void FillNodeOrderVector(std::vector<std::shared_ptr<CNode>>& rvecNodes, bool bRootView = true,
            bool bTopLevel = true);

        /**
         * @brief Accesses a node by its key in the parse tree. Overload of CNode::Direct.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @attention For an array, when no indexing is supplied, the latest entry will be returned.
         * @param[in] rssPath The path of the node to searched for.
         * @return Returns a shared pointer to the wanted node if it was found or a node with invalid content if it was not found.
         */
        virtual std::shared_ptr<CNode> Direct(const std::string& rssPath) const override;

        /**
         * @brief Searches a node by its key in the parse tree
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child
         * name. E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be
         * accessed and traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of
         * the array 'array'. These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @param[in] ssPath The path of the node to searched for.
         * @return Returns an interface the requested node if available.
         */
        virtual sdv::IInterfaceAccess* GetNodeDirect(/*in*/ const sdv::u8string& ssPath) const override;

        /**
         * @brief Get or create the parent nodes automotatically from the path.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @attention For an array, when no indexing or a too large index number is supplied, a new entry will be created and
         * returned.
         * @param[in] rssPath The path of the node to searched for.
         * @param[in] bInsertTableArray Since a table array consists of an array and a table, this is different than the other
         * insertions that only insert one element. Some special treatment is needed at certain points.
         * @return Returns a pair with the shared pointer to the parent node and the leftover name. If the creation could not be
         * done, a NULL pointer is returned.
         */
        virtual std::pair<std::shared_ptr<CNodeCollection>, std::string> SmartParentCreate(const std::string& rssPath,
            bool bInsertTableArray = false);

        /**
         * @brief Insert a value into the collection at the location before the supplied index. Overload of
         * sdv::toml::INodeCollectionInsert::InsertValue.
         * @remarks In TOML, inline nodes are located before standard nodes. Since values are presented as inline node, they
         * will be inserted before any standard node (table or table array if defined as standard node).
         * @param[in] ssInsertBefore Name of the node to insert the value before. In case of an array, can be an index between
         * square brackets. Can be empty, causing the node to be inserted at the end.
         * @param[in] ssName Name of the node to insert. This name can contain parent nodes, which are automatically created if
         * not existing. With arrays, since the value in TOML doesn't have a name, the name of the value must be an empty
         * string and any names provided are considered parent nodes. The name must adhere to the key names defined by the TOML
         * specification. Defining the key multiple times is not allowed. Quotation of key names is done automatically; the
         * parser decides itself whether the key is bare-key, a literal key or a quoted key.
         * @param[in] anyValue The value of the node, being either an integer, floating point number, virtual bool value or a
         * string.
         * Conversion is automatically done to int64, double float, bool or u8string.
         * @return On success the interface to the newly inserted node is returned or NULL otherwise.
         */
        virtual sdv::IInterfaceAccess* InsertValue(/*in*/ const sdv::u8string& ssInsertBefore, /*in*/ const sdv::u8string& ssName,
            /*in*/ sdv::any_t anyValue) override;

        /**
         * @brief Insert an array into the collection at the location before the supplied index. Overload of
         * sdv::toml::INodeCollectionInsert::InsertArray.
         * @remarks In TOML, inline nodes are located before standard nodes. Since arrays are presented as inline node, they
         * will be inserted before any standard node (table or table array if defined as standard node).
         * @param[in] ssInsertBefore Name of the node to insert the value before. In case of an array, can be an index between
         * square brackets. Can be empty, causing the node to be inserted at the end.
         * @param[in] ssName Name of the node to insert. This name can contain parent nodes, which are automatically created if
         * not existing. With arrays, since the value in TOML doesn't have a name, the name of the value must be an empty
         * string and any names provided are considered parent nodes. The name must adhere to the key names defined by the TOML
         * specification. Defining the key multiple times is not allowed. Quotation of key names is done automatically; the
         * parser decides itself whether the key is bare-key, a literal key or a quoted key.
         * @return On success the interface to the newly inserted node is returned or NULL otherwise.
         */
        virtual sdv::IInterfaceAccess* InsertArray(/*in*/ const sdv::u8string& ssInsertBefore,
            /*in*/ const sdv::u8string& ssName) override;

        /**
         * @brief Insert a table into the collection at the location before the supplied index. Overload of
         * sdv::toml::INodeCollectionInsert::InsertTable.
         * @remarks In TOML, inline nodes are located before standard nodes. Tables can be inserted as inline node, in which
         * case they will be inserted before any standard node (table or table array if defined as standard node).
         * @param[in] ssInsertBefore Name of the node to insert the value before. In case of an array, can be an index between
         * square brackets. Can be empty, causing the node to be inserted at the end.
         * @param[in] ssName Name of the node to insert. This name can contain parent nodes, which are automatically created if
         * not existing. With arrays, since the value in TOML doesn't have a name, the name of the value must be an empty
         * string and any names provided are considered parent nodes. The name must adhere to the key names defined by the TOML
         * specification. Defining the key multiple times is not allowed. Quotation of key names is done automatically; the
         * parser decides itself whether the key is bare-key, a literal key or a quoted key.
         * @param[in] ePreference The preferred form of the node to be inserted.
         * @return On success the interface to the newly inserted node is returned or NULL otherwise.
         */
        virtual sdv::IInterfaceAccess* InsertTable(/*in*/ const sdv::u8string& ssInsertBefore, /*in*/ const sdv::u8string& ssName,
            /*in*/ sdv::toml::EInsertPreference ePreference) override;

        /**
         * @brief Insert a table array into the collection at the location before the supplied index. Overload of
         * sdv::toml::INodeCollectionInsert::InsertTableArray.
         * @remarks In TOML, inline nodes are located before standard nodes. Table arrays can be inserted as inline node, in
         * which case they will be inserted before any standard node (table or table array if defined as standard node).
         * @param[in] ssInsertBefore Name of the node to insert the value before. In case of an array, can be an index between
         * square brackets. Can be empty, causing the node to be inserted at the end.
         * @param[in] ssName Name of the node to insert. This name can contain parent nodes, which are automatically created if
         * not existing. With arrays, since the value in TOML doesn't have a name, the name of the value must be an empty
         * string and any names provided are considered parent nodes. The name must adhere to the key names defined by the TOML
         * specification. Defining the key multiple times is not allowed. Quotation of key names is done automatically; the
         * parser decides itself whether the key is bare-key, a literal key or a quoted key.
         * @param[in] ePreference The preferred form of the node to be inserted.
         * @return On success the interface to the newly inserted node is returned or NULL otherwise.
         */
        virtual sdv::IInterfaceAccess* InsertTableArray(/*in*/ const sdv::u8string& ssInsertBefore, /*in*/ const sdv::u8string& ssName,
            /*in*/ sdv::toml::EInsertPreference ePreference) override;

        /**
         * @brief Insert a TOML string as a child of the current collection node. If the collection is a table, the TOML string
         * should contain values and inline/external/array-table nodes with names. If the collection is an array, the TOML
         * string should contain and inline table nodes without names. Overload of sdv::toml::INodeCollectionInsert::InsertTOML.
         * @remarks In TOML, inline nodes are located before standard nodes. Dependable on the nodes defined in the TOML they
         * might be transferred to inline or they might be inserted at a different location.
         * @param[in] ssInsertBefore Name of the node to insert the value before. In case of an array, can be an index between
         * square brackets. Can be empty, causing the node to be inserted at the end.
         * @param[in] ssTOML The TOML string to insert.
         * @param[in] bRollbackOnPartly If only part of the nodes could be inserted, no node will be inserted.
         * @return The result of the insertion.
         */
        virtual sdv::toml::INodeCollectionInsert::EInsertResult InsertTOML(/*in*/ const sdv::u8string& ssInsertBefore,
            /*in*/ const sdv::u8string& ssTOML, /*in*/ bool bRollbackOnPartly) override;

        /**
         * @brief Insert a TOML string as a child of the current collection node. If the collection is a table, the TOML string
         * should contain values and inline/external/array-table nodes with names. If the collection is an array, the TOML
         * string should contain and inline table nodes without names.
         * @param[in] rptrInsertBefore The node to insert the TOML nodes before (if possible). Can be NULL, causing the TOML nodes
         * to be inserted at the end.
         * @param[in] ssTOML The TOML string to insert.
         * @param[in] bRollbackOnPartly If only part of the nodes could be inserted, no node will be inserted.
         * @return A pair structure with the result of the insertion and a vector of all the inserted nodes.
         */
        std::pair<sdv::toml::INodeCollectionInsert::EInsertResult, std::vector<std::shared_ptr<CNode>>> InsertTOML(
            const std::shared_ptr<CNode>& rptrInsertBefore, const sdv::u8string& ssTOML, bool bRollbackOnPartly);

        /**
         * @brief Delete the current node. Overload of sdv::toml::INodeUpdate::DeleteNode.
         * @attention A successful deletion will cause all interfaces to the current node to become inoperable.
         * @return Returns whether the deletion was successful.
         */
        virtual bool DeleteNode() override;

        /**
         * @brief The derived class from the node collection can be inline or not. Overload of CNode::Inline.
         * @return Returns whether the node is an inline node.
         */
        virtual bool Inline() const override;

        /**
         * @brief With some node collections it is possible to switch between inline and normal. Overload of CNode::Inline.
         * @remarks Additional node composition information will be removed and the order within the parent node might be changed.
         * @param[in] bInline When set, try to switch to inline. Otherwise try to switch to normal.
         * @param[in] bIncludeChildren When set and bInline is not set, applicable child nodes are converted as well (only tables
         * and table-arrays can be defined as standard). Making a node inline is always including the children.
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline, bool bIncludeChildren = true) override;

        /**
         * @brief Can the node convert to an inline definition? Overload of sdv::toml::INodeCollectionConvert::CanMakeInline.
         * @return Returns whether the conversion to inline is possible. Returns 'true' when the node is already inline.
         */
        virtual bool CanMakeInline() const override;

        /**
         * @brief Convert the node to an inline node. Overload of sdv::toml::INodeCollectionConvert::MakeInline.
         * @return Returns whether the conversion was successful. Returns 'true' when the node was already inline.
         */
        virtual bool MakeInline() override;

        /**
         * @brief Can the node convert to a standard definition? Overload of sdv::toml::INodeCollectionConvert::CanMakeStandard.
         * @return Returns whether the conversion to standard is possible. Returns 'true' when the node is already defined as
         * standard node.
         */
        virtual bool CanMakeStandard() const override;

        /**
         * @brief Convert the node to a standard node. Overload of sdv::toml::INodeCollectionConvert::MakeStandard.
         * @param[in] bIncludeChildren When set, applicable child nodes are made are converted to standard nodes as well (only
         * tables and table-arrays can be defined as standard).
         * @return Returns whether the conversion was successful. Returns 'true' when the node was already defined as standard
         * node.
         */
        virtual bool MakeStandard(/*in*/ bool bIncludeChildren) override;

        /**
         * @brief Delete a node from the collection.
         * @remarks The node will not be deleted, but placed in the recycle bin. Deletion will take place at collection destruction.
         * @param[in] rptrNode Reference to the smart pointer pointing to the node to remove.
         * @return Returns whether the removal was successful.
         */
        bool DeleteNode(const std::shared_ptr<CNode>& rptrNode);

        /**
         * @brief Find the index belonging to the provided node.
         * @param[in] rptrNode Reference to the smart pointer holding the node to return the index for.
         * @return Return the node index. Returns npos if the node could not be found.
         */
        uint32_t FindIndex(const std::shared_ptr<CNode>& rptrNode) const;

        /**
         * @brief Is the provided child node a direct or indirect child node?
         * @param[in] rptrNode Reference to the smart pointer of the potential descendant node.
         * @return Returns whether the provided node is a descendant of the this node.
         */
        bool IsDescendant(const std::shared_ptr<CNode>& rptrNode) const;

        /**
         * @brief Generic add function for nodes.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @attention For an array, when no indexing is supplied, the latest entry will be returned.
         * @remarks If the node to insert exists already, but is marked implicit, the node will be returned and made explicit. In
         * all other cases the an error will occur that the node already exists.
         * @param[in] rrangeKeyPath Reference to the token range containing the path to the node to insert.
         * @param[in] rtArgs Zero or more references to arguments passed to the constructor of the node classes being created by
         * this function.
         * @return Returns a shared pointer to the inserted node. This node is of the requested type, except with table arrays;
         * there the returned node is a table within the table array.
         */
        template <typename TNodeType, typename... TArgs>
        std::shared_ptr<CNode> AddNodeFromRange(const CTokenRange& rrangeKeyPath, const TArgs&... rtArgs);

        /**
         * @brief Combine the collection with the provided content (mathematical union).
         * @details Update the child nodes with the child nodes of the provided collection. Extend the collection with nodes that do
         * not exist and update the existing nodes with new values.
         * @param[in] rptrCollection Reference to the collection being used for this operation.
         * @return Returns whether the combination was successful.
         */
        virtual bool Combine(const std::shared_ptr<CNodeCollection>& rptrCollection) = 0;

        /**
         * @brief Reduce the collection with by the provided content (mathematical difference).
         * @details Reduce the child node with the child nodes already defined and identical in the provided collection. Different
         * nodes or nodes that are not present in the collection remain.
         * @param[in] rptrCollection Reference to the collection being used for this operation.
         * @return Returns whether the reduction was successful.
         */
        virtual bool Reduce(const std::shared_ptr<CNodeCollection>& rptrCollection) = 0;

        /**
         * @brief When the parent changes (e.g. when moving items from one parser to the other), the items and all its sub-items
         * need to reasign the parser. Overload of CNode::ReassignParser.
         * @param[in] rParser Reference to the parser to assign.
         */
        virtual void ReassignParser(CParser& rParser) override;

    private:
        /**
         * @brief When set, the child nodes need grouping (values following each other, tables and table arrays at the end).
         */
        virtual bool GroupChildNodes() const { return Inline(); }

        // A table could contain nodes that are child notes of (implicitly) generated tables. For example:
        // [MyTable]
        // normal_value1 = "normal1"
        // child_table.my_value1 = "hello1"
        // normal_value2 = "normal2"
        // child_table.my_value2 = "hello2"
        // normal_value2 = "normal3"
        //
        // The implicit table MyTable.child_table has two values: my_value1 and my_value2
        //
        // Making an explicit table out of it would theoretically be possible, but the insertion of values according to the position
        // will become extremely difficult.
        //
        // [MyTable.child_table]     # This will cause an explicit table
        // 
        // The result will be, that the table is explicitly defined, causing the original table from above to not print the implicit
        // table entries any more:
        // [MyTable]
        // normal_value1 = "normal1"
        // normal_value2 = "normal2"
        // normal_value2 = "normal3"
        // 
        // [MyTable.child_table]     # This will cause an explicit table
        // child_table.my_value1 = "hello1"
        // child_table.my_value2 = "hello2"
        //
        // The node collection uses the vector to include the nodes by order.
        // The node collection uses the node list to include child nodes.
        // The child node uses the parent node pointer to indicate which node holds node.
        // The child node uses the view node pointer to indicate which node displays the node content.
                
        std::list<std::shared_ptr<CNode>>   m_lstNodes;         ///< List holding the direct child nodes.
        std::list<std::shared_ptr<CNode>>   m_lstRecycleBin;    ///< List holding the child elements that were deleted. This will
                                                                ///< prevent destruction of the node class, which would otherwise
                                                                ///< lead to unstable behavior when interfaces of the node are still
                                                                ///< being accessed. 
    };

    /**
     * @brief A dynamic table structure that allows mixed data in form of key value pairs.
     * @details A table can be explicit and inline. One version of the explicit table exists. Two versions of the inline
     * table exist: the assignment version and the embedded in-an-array-version.
     * Explicit table:
     * @code
     *      <whitespace>[<whitespace]KEY<whitespace>]<whitespace>
     *        ^           ^               ^            ^
     *        pre-node    pre_key_ws      post_key_ws  post_node
     * @endcode
     * Not embedded in an array, but inline:
     * @code
     *      <whitespace>KEY<whitespace>.<whitespace>KEY<whitespace>=<whitespace>{ZERO_OR_MORE_VALUES}<whitespace>
     *        ^              ^            ^              ^            ^                                ^
     *        pre-node       post_key_ws  pre_key_ws     post_key_ws  pre_value_ws                     post-node
     * @endcode
     * Embedded in an array:
     * @code
     *      <whitespace>{ZERO_OR_MORE_VALUES}<whitespace>
     *        ^                                ^
     *        pre-node                         post-node
     * @endcode
     * The pre- and post-node whitespace areas are covered by the UpdateNodeCode function of the CNode base class.
     * Multiple optional pre- and post-separator whitespace areas can be defined.
     */
    class CTable : public CNodeCollection
    {
    public:
        /**
         * @brief Constructor
         * @param[in] rparser Reference to the TOML parser.
         * @param[in] rssName Reference to the name of the node.
         * @param[in] rssRawName Reference to the raw name of the node.
         * @param[in] bDefaultInline The default value for the inline flag.
         * @param[in] bExplicit When set, the table is defined explicit. Otherwise the table is an implicit table and allows
         * additional elements to be added.
         */
        CTable(CParser& rparser, const std::string& rssName, const std::string& rssRawName, bool bDefaultInline,
            bool bExplicit = true);

        /**
         * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
         * @return Type of the node.
         */
        virtual sdv::toml::ENodeType GetType() const override;

        /**
         * @brief Returns the amount of nodes. Overload of sdv::toml::INodeCollection::GetCount.
         * @return The amount of nodes.
         */
        virtual uint32_t GetCount() const override;

        /**
         * @brief Get the node. Overload of CNodeCollection::Get.
         * @param[in] uiIndex Index of the node to get.
         * @return Smart pointer to the node object.
         */
        virtual std::shared_ptr<CNode> Get(uint32_t uiIndex) const override;

        /**
         * @brief Delete the current node. Overload of sdv::toml::INodeUpdate::DeleteNode.
         * @attention A successful deletion will cause all interfaces to the current node to become inoperable.
         * @return Returns whether the deletion was successful.
         */
        virtual bool DeleteNode() override;

        /**
         * @brief Create the TOML text based on the content using an optional prefix node. Overload of CNode::GenerateTOML.
         * @param[in] rContext Reference to the context class to use during TOML code generation.
         * @return The TOML text string.
         */
        virtual std::string GenerateTOML(const CGenContext& rContext = CGenContext()) const override;

        /**
         * @brief Update the node with TOML code information. Overload of CNode::UpdateNodeCode.
         * @param[in] rNodeRange Reference to the node range information containing the tokens for the code snippets.
         */
        virtual void UpdateNodeCode(const CNodeTokenRange& rNodeRange) override;

        /**
         * @brief The derived class from the node collection can be inline or not. Overload of CNodeCollection::Inline.
         * @return Returns whether the node is an inline node.
         */
        virtual bool Inline() const override;

        /**
         * @brief Switch between inline and explicit table definition. Overload of CNodeCollection::Inline.
         * @attention It is not possible to switch to an explicit table definition if the table is part of an array, since the
         * table doesn't have a name.
         * @remarks Additional node composition information will be removed and the order within the parent node might be changed.
         * @param[in] bInline When set, try to switch to inline. Otherwise try to switch to normal.
         * @param[in] bIncludeChildren When set and bInline is not set, applicable child nodes are converted as well (only tables
         * and table-arrays can be defined as standard). Making a node inline is always including the children.
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline, bool bIncludeChildren = true) override;

        /**
         * @brief Checks whether the table was explicitly defined. Overload of CNodeCollection::ExplicitlyDefined.
         * @return Returns the explicit definition flag.
         */
        virtual bool ExplicitlyDefined() const override;

        /**
         * @brief If the table was an implicit definition, make it explicit. Overload of CNodeCollection::MakeExplicit.
         */
        virtual void MakeExplicit() override;

        /**
         * @brief Combine the collection with the provided content (mathematical union). Overload of CNodeCollection::Combine.
         * @details Update the child nodes with the child nodes of the provided collection. Extend the collection with nodes that do
         * not exist and update the existing nodes with new values.
         * @param[in] rptrCollection Reference to the collection being used for this operation.
         * @return Returns whether the combination was successful.
         */
        virtual bool Combine(const std::shared_ptr<CNodeCollection>& rptrCollection) override;

        /**
         * @brief Reduce the collection with by the provided content (mathematical difference). Overload of CNodeCollection::Reduce.
         * @details Reduce the child node with the child nodes already defined and identical in the provided collection. Different
         * nodes or nodes that are not present in the collection remain.
         * @param[in] rptrCollection Reference to the collection being used for this operation.
         * @return Returns whether the reduction was successful.
         */
        virtual bool Reduce(const std::shared_ptr<CNodeCollection>& rptrCollection) override;

        /**
         * @brief After every insert, deletion and shift, the node order of this and all sub- tables need to be rebuild.
         * @param[in] bForce Force rebuild, even if locked.
         */
        virtual void RebuildNodeOrder(bool bForce) override;

    private:
        bool    m_bDefinedExplicitly = true;                ///< When set, the table is defined explicitly.
        bool    m_bInline = false;                          ///< Flag determining whether the table is inline or not.
        std::vector<std::shared_ptr<CNode>> m_vecNodeOrder; ///< Vector holding the child nodes (could contain grand children as
                                                            ///< well).
    };

    /**
     * @brief A dynamic array structure that allows mixed data of multiple values to be assigned to one key.
     * @details In most cases an array is inline and is defined similar to value assignments. The table array (an array consisting)
     * only of tables can also be defined as an explicit array as well. Two versions of the inline array exist: the assignment
     * version and the embedded in-an-array-version.
     * Explicit table array:
     * @code
     *      <whitespace>[[<whitespace>KEY<whitespace>]]<whitespace>
     *        ^           ^               ^            ^
     *        pre-node    pre_key_ws      post_key_ws  post_node
     * @endcode
     * Not embedded in an array, but inline:
     * @code
     *      <whitespace>KEY<whitespace>.<whitespace>KEY<whitespace>=<whitespace>[ZERO_OR_MORE_VALUES,<whitespace>]<whitespace>
     *        ^              ^            ^              ^            ^                                ^            ^
     *        pre-node       post_key_ws  pre_key_ws     post_key_ws  pre_value_ws                     post_values  post-node
     * @endcode
     * Embedded in an array:
     * @code
     *      <whitespace>[ZERO_OR_MORE_VALUES,<whitespace>]<whitespace>
     *        ^                                ^            ^
     *        pre-node                         post_values  post-node
     * @endcode
     * The pre- and post-node whitespace areas are covered by the UpdateNodeCode function of the CNode base class.
     * Multiple optional pre- and post-separator whitespace areas can be defined.
     */
    class CArray : public CNodeCollection
    {
    public:
        /**
         * @brief Constructor
         * @remarks If the array is not defined as explicit table array, the array will be inline.
         * @param[in] rparser Reference to the TOML parser.
         * @param[in] rssName Reference to the name of the node.
         * @param[in] rssRawName Reference to the raw name of the node.
         * @param[in] bExplicitTableArray When set, the array is defined as explicit table array. In this case, the array will not
         * be inline and needs at least one child-table.
         */
        CArray(CParser& rparser, const std::string& rssName, const std::string& rssRawName, bool bExplicitTableArray = false);

        /**
         * @brief Get the node type. Overload of sdv::toml::INodeInfo::GetType.
         * @return Type of the node.
         */
        virtual sdv::toml::ENodeType GetType() const override;

        /**
         * @brief Accesses a node by its key in the parse tree. Overload of CNode::Direct.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @remarks The definition of an array in TOML differentiate from the syntax to access the elements. For example an array in
         * TOML could be defined by:
         * @code
         * integers = [ 1, 2, 3 ]
         * nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
         * [[products]]
         * name = "Hammer"
         * sku = 738594937
         * @endcode
         * The first two examples define the complete array at once. The third example defines one element to be added to an array.
         * Random access to previous definitions is not required. The access functions need random access to each element. The Direct
         * function uses the syntax similar to C++:
         * @code
         * integers[1]                  --> gives: 2
         * nested_mixed_array[1][2]     --> gives: "c"
         * products[0].sku              --> gives: 738594937
         * @endcode
         * To find array elements, the path names are composed of elements separated by a dot. The Add and Find functions use the
         * following syntax:
         * @code
         * integers.1                   --> stores: 2
         * nested_mixed_array.1.2       --> stores: "c"
         * products.0.sku               --> stores: 738594937
         * @endcode
         * @attention Array element access indices starts with 0!
         * @attention For an array element inserting, when no indexing is supplied, the latest entry will be returned.
         * @param[in] rssPath Reference to the path of the node to searched for.
         * @return Returns a shared pointer to the wanted node if it was found or a node with invalid content if it was not found.
         */
        virtual std::shared_ptr<CNode> Direct(const std::string& rssPath) const override;

        /**
         * @brief Get or create the parent nodes automotatically from the path. Overload of CNodeCollection::SmartParentCreate.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @attention For an array, when no indexing or a too large index number is supplied, a new entry will be created and
         * returned.
         * @param[in] rssPath The path of the node to searched for.
         * @param[in] bInsertTableArray Since a table array consists of an array and a table, this is different than the other
         * insertions that only insert one element. Some special treatment is needed at certain points.
         * @return Returns a pair with the shared pointer to the parent node and the leftover name. If the creation could not be
         * done, a NULL pointer is returned.
         */
        virtual std::pair<std::shared_ptr<CNodeCollection>, std::string> SmartParentCreate(const std::string& rssPath,
            bool bInsertTableArray = false) override;

        /**
         * @brief Create the TOML text based on the content using an optional prefix node. Overload of CNode::GenerateTOML.
         * @param[in] rContext Reference to the context class to use during TOML code generation.
         * @return Return the TOML text string.
         */
        virtual std::string GenerateTOML(const CGenContext& rContext = CGenContext()) const override;
    
        /**
         * @brief Update the node with TOML code information. Overload of CNode::UpdateNodeCode.
         * @param[in] rNodeRange Reference to the node range information containing the tokens for the code snippets.
         */
        virtual void UpdateNodeCode(const CNodeTokenRange& rNodeRange) override;

        /**
         * @brief Returns whether the array is defined as a table array (array with only tables and at least one table).
         * @remarks Table arrays can still be inline.
         * @return Returns whether the array is a table array.
         */
        bool TableArray() const;

        /**
         * @brief The derived class from the node collection can be inline or not. Overload of CNode::Inline.
         * @return Returns whether the node is an inline node.
         */
        virtual bool Inline() const override;

        /**
         * @brief Allow switching between the inline array and a table array. Overload of CNode::Inline.
         * @details Most array definitions are inline. An exception to the rule is a table array, which is allowed to be explicit
         * as well. To switch from inline array to an explicit array, at least one member needs to be present and all members need
         * to be tables.
         * @attention It is not possible to switch to an explicit array when the array is part of another array, since the atray
         * doesn't have a name that can be used to define the explicit table array.
         * @remarks Additional node composition information will be removed and the order within the parent node might be changed.
         * @param[in] bInline When set, try to switch to inline. Otherwise try to switch to normal.
         * @param[in] bIncludeChildren When set and bInline is not set, applicable child nodes are converted as well (only tables
         * and table-arrays can be defined as standard). Making a node inline is always including the children.
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline, bool bIncludeChildren = true) override;

        /**
         * @brief Can the node convert to a standard definition? Overload of sdv::toml::INodeCollectionConvert::CanMakeStandard.
         * @return Returns whether the conversion to standard is possible. Returns 'true' when the node is already defined as
         * standard node.
         */
        virtual bool CanMakeStandard() const override;

        /**
         * @brief Does the last child node need a comma following the node?
         * @return Returns whether the array was defined with the last node requiring a comma following the node.
         */
        bool LastNodeWithSucceedingComma() const;

        /**
         * @brief Combine the collection with the provided content (mathematical union). Overload of CNodeCollection::Combine.
         * @details Update the child nodes with the child nodes of the provided collection. Extend the collection with nodes that do
         * not exist and update the existing nodes with new values.
         * @param[in] rptrCollection Reference to the collection being used for this operation.
         * @return Returns whether the combination was successful.
         */
        virtual bool Combine(const std::shared_ptr<CNodeCollection>& rptrCollection) override;

        /**
         * @brief Reduce the collection with by the provided content (mathematical difference). Overload of CNodeCollection::Reduce.
         * @details Reduce the child node with the child nodes already defined and identical in the provided collection. Different
         * nodes or nodes that are not present in the collection remain.
         * @param[in] rptrCollection Reference to the collection being used for this operation.
         * @return Returns whether the reduction was successful.
         */
        virtual bool Reduce(const std::shared_ptr<CNodeCollection>& rptrCollection) override;

    private:
        bool m_bDefinedExplicitly = true;       ///< When set, the array is defined explicitly.
        bool m_bInline = false;                 ///< Flag determining whether the table is inline or not.
        bool m_bLastChildNodeWithComma = false; ///< Set when the last child node of the array initially has a comma behind the node.
    };

    /**
     * @brief Array of tables
     */
    class CTableArray : public CArray
    {
    public:
        /**
         * @brief Constructor
         * @param[in] rparser Reference to the TOML parser.
         * @param[in] rssName Reference to the name of the node.
         * @param[in] rssRawName Reference to the raw name of the node.
         */
        CTableArray(CParser& rparser, const std::string& rssName, const std::string& rssRawName) :
            CArray(rparser, rssName, rssRawName, true)
        {}
    };

    /**
     * @brief Root table
     */
    class CRootTable : public CTable
    {
    public:
        /**
         * @brief Constructor
         * @param[in] rparser Reference to the TOML parser.
         */
        CRootTable(CParser& rparser) : CTable(rparser, "root", "", false, true)
        {}

        /**
         * @brief Delete the current node. Overload of sdv::toml::INodeUpdate::DeleteNode.
         * @attention A successful deletion will cause all interfaces to the current node to become inoperable.
         * @return Returns whether the deletion was successful.
         */
        virtual bool DeleteNode() override;

        /**
         * @brief The derived class from the node collection can be inline or not. Overload of CNodeCollection::Inline.
         * @return Returns whether the node is an inline node.
         */
        virtual bool Inline() const override
        {
            // The root node can never be inline.
            return false;
        }

        /**
         * @brief Switch between inline and explicit table definition. Overload of CNodeCollection::Inline.
         * @attention It is not possible to switch to an explicit table definition if the table is part of an array, since the
         * table doesn't have a name.
         * @remarks Additional node composition information will be removed and the order within the parent node might be changed.
         * @param[in] bInline When set, try to switch to inline. Otherwise try to switch to normal.
         * @param[in] bIncludeChildren When set and bInline is not set, applicable child nodes are converted as well (only tables
         * and table-arrays can be defined as standard). Making a node inline is always including the children.
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline, bool bIncludeChildren = true) override
        {
            // Pass call to table implementation.
            return CTable::Inline(bInline, bIncludeChildren);
        }
    };

    template <typename TNodeType>
    std::shared_ptr<TNodeType> CNode::Cast()
    {
        return std::dynamic_pointer_cast<TNodeType>(shared_from_this());
    }

    template <typename TNodeType>
    std::shared_ptr<const TNodeType> CNode::Cast() const
    {
        return std::dynamic_pointer_cast<const TNodeType>(shared_from_this());
    }

    template <typename TNodeType, typename... TArgs>
    inline std::shared_ptr<CNode> CNodeCollection::AddNodeFromRange(const CTokenRange& rrangeKeyPath, const TArgs&... rtArgs)
    {
        // Get the first part of the node
        auto prKey = SplitNodeKey(rrangeKeyPath);

        // Find the node if it exists.
        auto itNode = std::find_if(m_lstNodes.begin(),
            m_lstNodes.end(),
            [&](const std::shared_ptr<CNode>& rptrNode) { return rptrNode->GetName() == prKey.first.get().StringValue(); });

        // Is this the target node?
        std::shared_ptr<CNode> ptrNode;
        if (!prKey.second)
        {
            if (std::is_same_v<TNodeType, CTableArray>)
            {
                // If this is a table array, create if not existing and then create a table.
                std::shared_ptr<CTableArray> ptrTableArray;
                if (itNode == m_lstNodes.end())
                {
                    ptrTableArray = std::make_shared<CTableArray>(Parser(), prKey.first.get().StringValue(),
                        prKey.first.get().RawString());

                    // Set the parent pointer; this will add the node to the list.
                    ptrTableArray->SetParentPtr(Cast<CNodeCollection>());
                }
                else
                    ptrTableArray = (*itNode)->template Cast<CTableArray>();
                if (!ptrTableArray)
                    throw XTOMLParseException("The node '" + prKey.first.get().StringValue() +
                        "' exists already, but is not a table array.");

                // Create the table.
                ptrNode = ptrTableArray->AddNodeFromRange<CTable>(CTokenRange(prKey.first, prKey.first.get().Next()),
                    false, true);
            } else if (!Cast<CArray>() && itNode != m_lstNodes.end())
            {
                // If existing... this might be a duplicate if not explicitly defined before.
                // But only when the parent is not an array.
                ptrNode = *itNode;

                // Explicitly defined nodes cannot be defined once more
                if (ptrNode->ExplicitlyDefined())
                    throw XTOMLParseException("The node '" + prKey.first.get().StringValue() + "' exists already.");

                // Inline nodes cannot be made explicit
                if (ptrNode->Inline())
                    throw XTOMLParseException("The node '" + prKey.first.get().StringValue() + "' exists already as inline node.");

                // Make the note explicit. This will remove the view pointer if it was set before.
                ptrNode->MakeExplicit();
            } else
            {
                // Create the target node.
                ptrNode = std::make_shared<TNodeType>(Parser(), prKey.first.get().StringValue(), prKey.first.get().RawString(),
                    rtArgs...);

                // Set the parent pointer; this will add the node to the list.
                ptrNode->SetParentPtr(Cast<CNodeCollection>());

                // If the current node is implicit, take over the inline flag (this determines whether a sub-table definition is
                // allowed or not).
                if (!ExplicitlyDefined())
                    Inline(ptrNode->Inline(), false);
            }
        }
        else // Intermediate node
        {
            // This must be a non-inline table node
            if (itNode != m_lstNodes.end())
            {
                ptrNode = *itNode;

                // Process the nodes dependable on the type
                if (ptrNode->Cast<CTableArray>())
                {
                    // In case the node is a table array, check for the next key part. If this is a number, then the current node
                    // can be an array, which will be indexed by the next key part. If this is not a number, the last table node
                    // is automatically assumed.
                    auto prNextKey = SplitNodeKey(prKey.second);
                    if (prNextKey.first.get().Category() != ETokenCategory::token_integer)
                    {
                        // The table array should have an ordered vector. This is not the case yet. Rebuild the table array.
                        ptrNode->Cast<CTableArray>()->RebuildNodeOrder(true);

                        // Get the last table
                        if (!ptrNode->Cast<CTableArray>()->GetCount())
                            throw XTOMLParseException("The parent table array node '" + prKey.first.get().StringValue() +
                                "' doesn't have any tables assigned.");
                        ptrNode = ptrNode->Cast<CTableArray>()->Get(ptrNode->Cast<CTableArray>()->GetCount() - 1);
                        if (ptrNode->Inline() || !ptrNode->Cast<CTable>())
                            throw XTOMLParseException("The parent node '" + prKey.first.get().StringValue() +
                                "' is of unexpected type.");
                    }
                }
                else if ((!ptrNode->Cast<CTable>() && !ptrNode->Cast<CArray>()) || (ptrNode->ExplicitlyDefined() && ptrNode->Inline()))
                {
                    // This is allowed:
                    // [fruit]
                    // apple.color = "red"
                    // apple.taste.sweet = true
                    // [fruit.apple.texture]  # you can add sub-tables
                    // --> fuit is explicitly defined, but not inline. apple is not explicitly defined
                    //
                    // This not:
                    // type = { name = "Nail" }
                    // type.edible = false  # INVALID
                    // --> type is explicitly defined
                    throw XTOMLParseException("The parent node '" + prKey.first.get().StringValue() + "' is of unexpected type.");
                }
            }
            else
            {
                // Create an implicit table node.
                ptrNode = std::make_shared<CTable>(Parser(), prKey.first.get().StringValue(), prKey.first.get().RawString(), false, false);
                ptrNode->SetParentPtr(Cast<CNodeCollection>());
            }

            // Insert the node in the child node
            std::shared_ptr<CNodeCollection> ptrNodeCollection = ptrNode->Cast<CNodeCollection>();
            if (!ptrNodeCollection)
                throw XTOMLParseException("Parent node is not an array or table '" + ptrNode->GetPath(true) + "'.");
            ptrNode = ptrNodeCollection->AddNodeFromRange<TNodeType>(prKey.second, rtArgs...);
            if (!ptrNode)
                throw XTOMLParseException("Could not create the node '" + prKey.first.get().StringValue() + "'.");
        }

        // Return the result
        return ptrNode;
    }
} // namespace toml_parser

#endif // !defined PARSER_NODE_TOML_H