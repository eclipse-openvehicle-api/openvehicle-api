#ifndef PARSER_NODE_TOML_H
#define PARSER_NODE_TOML_H

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <map>

#include <interfaces/toml.h>
#include <support/interface_ptr.h>
#include "lexer_toml.h"
#include "miscellaneous.h"

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
        inline_when_possible   = 0x01, ///< Try to generate as much as possible as inline nodes.
        explicit_when_possible = 0x02, ///< Try to generate as much as possible as explicit nodes.
        no_comments            = 0x10, ///< Do not include comments
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
         * @brief Called by the node that is generating the TOML. If not initialized before, initializes with the provided node.
         * @param[in] rptrNode Reference to the node that could be used for initialization as top most node.
         */
        void InitTopMostNode(const std::shared_ptr<const CNode>& rptrNode);

        /**
         * @brief Check whether the provided node is a parent of the top most node.
         * @param[in] rptrNode Reference to the node to use for the checking.
         * @return Returns true if the node is a parent of the top most node, false otherwise.
         */
        bool PartOfExcludedParents(const std::shared_ptr<const CNode>& rptrNode) const;

        /**
         * @brief Create a copy of the context class with a new key context.
         * @param[in] rssNewKeyContext Reference to the string containing the new key context.
         * @return The copy of the contetx class.
         */
        CGenContext CopyWithContext(const std::string& rssNewKeyContext) const;

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

    private:
        std::shared_ptr<const CNode>    m_ptrTopMostNode;   ///< Top most node that is used for the generation. The parent nodes of
                                                            ///< the top most node will not be part of the node generation and if
                                                            ///< they contain child nodes in their view, the nodes are printed by
                                                            ///< their parent and not by their view.
        std::string                     m_ssPrefixKey;      ///< Prefix key to be used during the generation of the TOML code.
        std::string                     m_ssKeyContext;     ///< string containing the current context. The string must follow the
                                                            ///< key rules for separation with bare, literal and quoted keys.
        uint32_t                        m_uiOptions = 0;    ///< Zero or more options to take into account when creating the text to
                                                            ///< the TOML nodes.
        bool                            m_bTopMost = true;  ///< Set when this context is the top most context.
    };

    /**
     * @brief Node to build up the parse tree
     */
    class CNode :
        public std::enable_shared_from_this<CNode>, public sdv::IInterfaceAccess, public sdv::toml::INodeInfo,
        public sdv::toml::INodeDelete, public sdv::toml::INodeUpdate
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
            SDV_INTERFACE_ENTRY(sdv::toml::INodeDelete)
            SDV_INTERFACE_ENTRY(sdv::toml::INodeUpdate)
        END_SDV_INTERFACE_MAP()

        /**
         * @brief Get a reference to the TOML parser that generated this node.
         * @return Reference to the TOML parse.
         */
        CParser& Parser();

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
         * @remarks This function can also be used to insert whitespace (with or without comments) when used in raw mode.
         * Set the comment text for the node. If a comment is proided as text (normal behavior), the comment text will be
         * formatted automatically when generating the TOML text. If the comment is provided as raw comment, the text should
         * contain all whitespace and the comment '#' character before the comment text.
         * Comments inserted before the enode will be inserted on the line before the node uness the comment is provided in raw
         * format and is ended with a newline and optionally whitespace. Comment inserted behind the node will be inserted on
         * the same line as the node.
         * Comments provided as text is automatically wrapped to 80 characters if possible. Newlines in the text will cause a
         * new comment line to start.
         * @param[in] ssComment String containing the comment text or the raw comment string to set.
         * @param[in] uiFlags One or more ECommentFlags flags influencing the behavior of the comment.
         */
        virtual void SetComment(const sdv::u8string& ssComment, uint32_t uiFlags) override;

        /**
         * Get the current comment for the node. Overload of sdv::toml::INodeInfo::GetComment.
         * @remarks To receive the whitespace formatting the node, use this function in raw mode.
         * @param[in] uiFlags One or more ECommentFlags flags identifying the string format of the comment to return.
         * @return String with the comment text or an empty string if no comment is available.
         */
        virtual sdv::u8string GetComment(uint32_t uiFlags) override;

        /**
         * @brief Format the node automatically. This will remove the whitespace between the elements within the node. Comments
         * will not be changed. Overload of sdv::toml::INodeInfo::AutomaticFormat.
         */
        virtual void AutomaticFormat() override;

        /**
         * @brief Update the node with TOML code information. The default implementation takes the comment and whitespace around the
         * node and stores this for node reconstruction.
         * @param[in] rNodeRange Reference to the node range information containing the tokens for the code snippets.
         */
        virtual void UpdateNodeCode(const CNodeTokenRange& rNodeRange);

        /**
         * @brief Delete the current node. Overload of sdv::toml::INodeDelete::DeleteNode.
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
         * @param[in] anyNewValue The value of the node, being either an integer, floating point number, boolean value or a
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
         * @brief Gets the parent node pointer
         * @return Returns the parent node pointer or an empty pointer when no parent was assigned or the stored weak pointer could
         * not be locked.
         */
        std::shared_ptr<CNodeCollection> GetParentPtr() const;

        /**
         * @brief Set the parent node.
         * @param[in] rptrParent Reference to the node to assign to this node as a parent.
         */
        void SetParentPtr(const std::shared_ptr<CNodeCollection>& rptrParent);

        /**
         * @brief Get the parent path of the node.
         * @return Return the parent path if existining and not a root.
         */
        std::string GetParentPath() const;

        /**
         * @brief Set the view definition node. The view definition node is a parent or grand parent that presents the node when
         * generating TOML code. When not set, the parent node is taking over this role.
         * @param[in] rptrView Reference to the node to assign to this node as a parent or grand parent.
         */
        void SetViewPtr(const std::shared_ptr<CNodeCollection>& rptrView);

        /**
         * @brief Checks whether the node is part of the view.
         * @details The node is part of the view if the supplied pointer is identical to the view definition pointer, when the view
         * definition pointer is not part of a parent of the topmost node. In all other cases, the node is part of the view.
         * @param[in] rContext Reference to the context class to use during TOML code generation.
         * @param[in] rptrNode Reference to the node to check whether it registered for a view.
         * @return Returns whether this node is part of the view with the supplied pointer.
         */
        bool IsPartOfView(const CGenContext& rContext, const std::shared_ptr<const CNodeCollection>& rptrNode) const;

        /**
         * @brief Accesses a node by its key in the parse tree.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @attention For an array, when no indexing is supplied, the latest entry will be returned.
         * @param[in] rssPath The path of the node to searched for.
         * @return Returns a shared pointer to the wanted Node if it was found or a node with invalid content if it was not found.
         */
        virtual std::shared_ptr<CNode> Direct(const std::string& rssPath) const = 0;

        /**
         * @brief Create the TOML text based on the content using an optional prefix node.
         * @param[in] rContext Reference to the context class to use during TOML code generation.
         * @return TOML text string.
         */
        virtual std::string GenerateTOML(const CGenContext& rContext = CGenContext()) const = 0;

    protected:
        /**
         * @brief Compose a custom path from the node key path using a key prefix and a context.
         * @param[in] rssPrefixKey The prefix to insert at as a base to the key tree.
         * @param[in] rssContext The context that is used to define the relative portion of the key. To determine the relative
         * portion, the context string contains the same prefix as is supplied in rssPrefixKey.
         * @return Returns the custom path composed of the prefix and the relative portion of the original path.
         */
        std::string GetCustomPath(const std::string& rssPrefixKey, const std::string& rssContext) const;

        /**
         * @brief Comment or code snippet structure.
         * @details Each node has multiple code snippets used to reproduce the exact code. For example with an assignment:
         * @code
         *
         * # This is out of scope comment before
         *
         * # This is comment before
         *     var_a   =     "abc"  # comment behind
         *                          # more comment behind
         *
         * # This is out of scope comment behind
         *
         * @endcode
         *
         * The code snippets are identified as follows:
         * @code
         * <out of scope comment before>
         * <comment before>
         * <space before><key><space>=<space><value><comment behind>
         * <out of scope comment behind>
         * @endcode
         */
        class CCodeSnippet
        {
        public:
            /**
             * @brief Access the token list.
             * @return Reference to the list with tokens.
             */
            std::list<CToken>& List();

            /**
             * @brief Access the comment text string.
             * @return Reference to the comment string.
             */
            std::string& Str();

            /**
             * @brief Mode the code snippet composer should run in.
             */
            enum class EComposeMode
            {
                compose_inline,         ///< Compose as inline whitespace and comment. If there is no token list and no comment
                                        ///< string, compose as one space. If there is only a comment string, insert a space, add
                                        ///< the comment followed by an obligatory newline, and insert spaces until the next
                                        ///< provided position. If there are tokens with a comment token, replace the comment. If
                                        ///< there are tokens without comment, add the comment, newline and spaces.
                compose_before,         ///< Compose as comment assigned to and located before the node. If there is no token list
                                        ///< and no comment string, doesn't add anything. If there is only a comment string, adds
                                        ///< the comment followed by the obligatory newline. If there are tokens with a comment
                                        ///< token, replace the comment. If there are tokens without the comment, place the comment
                                        ///< before the last newline or when not available, at the end of the tokens followed by a
                                        ///< new newline.
                compose_behind,         ///< Compose as comment assigned to and located behind the node. If there is no token list
                                        ///< and no comment string, add a newline. If there is a comment string and no tokens,
                                        ///< add a space, the comment string followed by the obligatory newline. If there is a token
                                        ///< list without comment, add a comment before the newline or at the end with an additional
                                        ///< newline.
                compose_standalone,     ///< Compose as stand-alone comment. Replace any token list if a comment string is
                                        ///< available.
            };

            /**
             * @brief Compose a conde string from the stored tokens and/or string.
             * @param[in] eMode The mode the composer should run in.
             * @param[in] nAssignmentOffset The offset for a next assignent; only used for inline composition.
             * @param[in] nCommentOffset The offset to insert a multi-line comment; only used for inline and behind composition.
             * @return The composed code string.
             */
            std::string Compose(EComposeMode eMode, size_t nAssignmentOffset = 0, size_t nCommentOffset = 0) const;

        private:
            std::list<CToken>   m_lstTokens;    ///< Token list for the code snippet in raw format.
            std::string         m_ssComment;    ///< The comment text for the code snippet in text format.
        };

        // White space and comment preservation indices for code generation.
        const size_t m_nPreNodeCode = 0;             ///< Code snippet before the node. Corresponds to
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
         * @param[in] nIndex The comment type index to get the comment for.
         * @param[in] rssKey Reference to the key to be used for code snippet identification.
         * @return Reference to the comment structure of the comment. If the provided index is not available in the vector,
         * returns an empty code snippet.
         */
        const CCodeSnippet& CodeSnippet(size_t nIndex, const std::string& rssKey = std::string()) const;

        /**
         * @brief Get the code snippet (write access). This allows moving the snippet from one node to the another node.
         * @remarks Since the request to the code snippet could change the location of the vector allocation, access to the code
         * snippet is valid until the next code snippet is requested.
         * @param[in] nIndex The comment type index to get the comment for.
         * @param[in] rssKey Reference to the key to be used for code snippet identification.
         * @return Reference to the comment structure of the comment.
         */
        CCodeSnippet& CodeSnippet(size_t nIndex, const std::string& rssKey = std::string());

    private:
        std::weak_ptr<CNodeCollection>      m_ptrParent;            ///< Weak pointer to the parent node (if existing).
        std::weak_ptr<CNodeCollection>      m_ptrView;              ///< Weak pointer to the view node (if existing and explicitly set).
        std::string                         m_ssName;               ///< Name of the node.
        std::string                         m_ssRawName;            ///< Raw name of the node.
        bool                                m_bDeleted = false;     ///< Enabled when the node was marked for deletion.
        CParser&                            m_rParser;              ///< Reference to the TOML parser.
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
         * @param[in] bInline When set, try to switch to inline. Otherwise try to switch to normal.
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline) = 0;

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
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline) override;

        /**
         * @brief Accesses a node by its key in the parse tree. Overload of CNode::Direct.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @attention For an array, when no indexing is supplied, the latest entry will be returned.
         * @param[in] rssPath The path of the node to searched for.
         * @return Returns a shared pointer to the wanted Node if it was found or a node with invalid content if it was not found.
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
         * @param[in] anyNewValue The value of the node, being either an integer, floating point number, boolean value or a
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
        bool    m_bVal = false;     ///< Value in case of boolean node.
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
         * @param[in] anyNewValue The value of the node, being either an integer, floating point number, boolean value or a
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
         * @param[in] anyNewValue The value of the node, being either an integer, floating point number, boolean value or a
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
         * @param[in] anyNewValue The value of the node, being either an integer, floating point number, boolean value or a
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
    class CNodeCollection : public CNode, public sdv::toml::INodeCollection, public sdv::toml::INodeCollectionInsert
    {
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
            SDV_INTERFACE_CHAIN_BASE(CNode)
        END_SDV_INTERFACE_MAP()

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
        std::shared_ptr<CNode> Get(uint32_t uiIndex) const;

        /**
         * @brief Accesses a node by its key in the parse tree. Overload of CNode::Direct.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @attention For an array, when no indexing is supplied, the latest entry will be returned.
         * @param[in] rssPath The path of the node to searched for.
         * @return Returns a shared pointer to the wanted Node if it was found or a node with invalid content if it was not found.
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
         * @brief Insert a value into the collection at the location before the supplied index. Overload of
         * sdv::toml::INodeCollectionInsert::InsertValue.
         * @param[in] uiIndex The insertion location to insert the node before. Can be npos or any value larger than the
         * collection count to insert the node at the end of the collection. Value nodes cannot be inserted behind external
         * tables and table arrays. If the index is referencing a position behind an external table or a table array, the index
         * is automatically corrected.
         * @param[in] ssName Name of the node to insert. Will be ignored for an array collection. The name must adhere to the
         * key names defined by the TOML specification. Defining the key multiple times is not allowed. Quotation of key names
         * is done automatically; the parser decides itself whether the key is bare-key, a literal key or a quoted key.
         * @param[in] anyValue The value of the node, being either an integer, floating point number, boolean value or a string.
         * Conversion is automatically done to int64, double float, bool or u8string.
         * @return On success the interface to the newly inserted node is returned or NULL otherwise.
         */
        virtual sdv::IInterfaceAccess* InsertValue(uint32_t uiIndex, const sdv::u8string& ssName, sdv::any_t anyValue) override;

        /**
         * @brief Insert an array into the collection at the location before the supplied index. Overload of
         * sdv::toml::INodeCollectionInsert::InsertArray.
         * @param[in] uiIndex The insertion location to insert the node before. Can be npos or any value larger than the
         * collection count to insert the node at the end of the collection. Array nodes cannot be inserted behind external
         * tables and table arrays. If the index is referencing a position behind an external table or a table array, the index
         * is automatically corrected.
         * @param[in] ssName Name of the array node to insert. Will be ignored if the current node is also an array collection.
         * The name must adhere to the key names defined by the TOML specification. Defining the key multiple times is not
         * allowed. Quotation of key names is done automatically; the parser decides itself whether the key is bare-key, a
         * literal key or a quoted key.
         * @return On success the interface to the newly inserted node is returned or NULL otherwise.
         */
        virtual sdv::IInterfaceAccess* InsertArray(uint32_t uiIndex, const sdv::u8string& ssName) override;

        /**
         * @brief Insert a table into the collection at the location before the supplied index. Overload of
         * sdv::toml::INodeCollectionInsert::InsertTable.
         * @param[in] uiIndex The insertion location to insert the node before. Can be npos or any value larger than the
         * collection count to insert the node at the end of the collection. Table nodes cannot be inserted before value nodes
         * or arrays. If the index is referencing a position before a value node or an array, the index is automatically
         * corrected.
         * @param[in] ssKeyName Name of the table node to insert. Will be ignored if the current node is an array collection.
         * The name must adhere to the key names defined by the TOML specification. Defining the key multiple times is not
         * allowed. Quotation of key names is done automatically; the parser decides itself whether the key is bare-key, a
         * literal key or a quoted key.
         * @return On success the interface to the newly inserted node is returned or NULL otherwise.
         */
        virtual sdv::IInterfaceAccess* InsertTable(uint32_t uiIndex, const sdv::u8string& ssKeyName) override;

        /**
         * @brief Insert a table array into the collection at the location before the supplied index. Overload of
         * sdv::toml::INodeCollectionInsert::InsertTableArray.
         * @param[in] uiIndex The insertion location to insert the node before. Can be npos or any value larger than the
         * collection count to insert the node at the end of the collection. Table array nodes cannot be inserted before value
         * nodes or arrays. If the index is referencing a position before a value node or an array, the index is automatically
         * corrected.
         * @param[in] ssName Name of the array node to insert. Will be ignored if the current node is also an array collection.
         * The name must adhere to the key names defined by the TOML specification. Defining the key multiple times is not
         * allowed. Quotation of key names is done automatically; the parser decides itself whether the key is bare-key, a
         * literal key or a quoted key.
         * @return On success the interface to the newly inserted node is returned or NULL otherwise.
         */
        virtual sdv::IInterfaceAccess* InsertTableArray(uint32_t uiIndex, const sdv::u8string& ssName) override;

        /**
         * @brief Insert a TOML string as a child of the current collection node. If the collection is a table, the TOML string
         * should contain values and inline/external/array-table nodes with names. If the collection is an array, the TOML
         * string should contain and inline table nodes without names. Overload of sdv::toml::INodeCollectionInsert::InsertTOML.
         * @param[in] ssTOML The TOML string to insert.
         * @param[in] bRollbackOnPartly If only part of the nodes could be inserted, no node will be inserted.
         * @return The result of the insertion.
         */
        virtual sdv::toml::INodeCollectionInsert::EInsertResult InsertTOML(const sdv::u8string& ssTOML,
            bool bRollbackOnPartly) override;

        /**
         * @brief Delete the current node. Overload of sdv::toml::INodeDelete::DeleteNode.
         * @attention A successful deletion will cause all interfaces to the current node to become inoperable.
         * @return Returns whether the deletion was successful.
         */
        virtual bool DeleteNode() override;

//protected:
        /**
         * @brief Remove a node from the collection.
         * @remarks The node will not be deleted, but placed in the recycle bin. Deletion will take place at collection destruction.
         * @param[in] rptrNode Reference to the smart pointer pointing to the node to remove.
         * @return Returns whether the removal was successful.
         */
        bool RemoveNode(const std::shared_ptr<CNode>& rptrNode);

        /**
         * @brief Remove a node from a view.
         * @param[in] rptrNode Reference to the smart pointer pointing to the node to remove.
         * @return Returns whether the removal was successful.
         */
        bool RemoveFromView(const std::shared_ptr<CNode>& rptrNode);

        /**
         * @brief Check whether this node is the last node in the collection.
         * @param[in] rptrNode Reference to the smart pointer pointing to the node to check for.
         * @return Returns whether the provided node is the last node in the collection.
         */
        bool CheckLast(const std::shared_ptr<CNode>& rptrNode);

    public:
        /**
         * @brief Find the index belonging to the provided node.
         * @param[in] rptrNode Reference to the smart pointer holding the node to return the index for.
         * @return Return the node index. Returns npos if the node could not be found.
         */
        uint32_t FindIndex(const std::shared_ptr<CNode>& rptrNode);

        /**
         * @brief Generic inserting function for nodes.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @attention Array indexing starts with 0!
         * @attention For an array, when no indexing is supplied, the latest entry will be returned.
         * @remarks If the node to insert exists already, but is marked implicit, the node will be returned and made explicit. In
         * all other cases the an error will occur that the node already exists.
         * @param[in] uiIndex The insertion location to insert the node before. Can be npos or any value larger than the
         * collection count to insert the node at the end of the collection. Table array nodes cannot be inserted before value
         * nodes or arrays. If the index is referencing a position before a value node or an array, the index is automatically
         * corrected.
         * @param[in] rrangeKeyPath Reference to the token range containing the path to the node to insert.
         * @param[in] rtArgs Zero or more references to arguments passed to the constructor of the node classes being created by
         * this function.
         * @return Returns a shared pointer to the inserted node. This node is of the requested type, except with table arrays;
         * there the returned node is a table within the table array.
         */
        template <typename TNodeType, typename... TArgs>
        std::shared_ptr<CNode> Insert(uint32_t uiIndex, const CTokenRange& rrangeKeyPath, const TArgs&... rtArgs);

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
                
        std::vector<std::shared_ptr<CNode>> m_vecNodeOrder;     ///< Vector holding the child nodes (could contain grand children as
                                                                ///< well).
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
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline) override;

        /**
         * @brief Checks whether the table was explicitly defined. Overload of CNodeCollection::ExplicitlyDefined.
         * @return Returns the explicit definition flag.
         */
        virtual bool ExplicitlyDefined() const override;

        /**
         * @brief If the table was an implicit definition, make it explicit. Overload of CNodeCollection::MakeExplicit.
         */
        virtual void MakeExplicit() override;


        //bool m_bOpenToAddChildren = true;   ///< If internal table, the table can be extended until the table is closed.

    private:
        bool m_bDefinedExplicitly = true;   ///< When set, the table is defined explicitly.
        bool m_bInline = false;             ///< Flag determining whether the table is inline or not.
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
         * @return Returns a shared pointer to the wanted Node if it was found or a node with invalid content if it was not found.
         */
        virtual std::shared_ptr<CNode> Direct(const std::string& rssPath) const override;

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
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline) override;

    private:
        bool m_bDefinedExplicitly = true;   ///< When set, the array is defined explicitly.
        bool m_bInline = false;             ///< Flag determining whether the table is inline or not.
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
        CRootTable(CParser& rparser) : CTable(rparser, "root", "", false)
        {}

        /**
         * @brief Delete the current node. Overload of sdv::toml::INodeDelete::DeleteNode.
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
            return false;
        }

        /**
         * @brief With some node collections it is possible to switch between inline and normal. Overload of
         * CNodeCollection::Inline.
         * @remarks Additional node composition information will be removed and the order within the parent node might be changed.
         * @param[in] bInline When set, try to switch to inline. Otherwise try to switch to normal.
         * @return Returns whether the switch was successful. A switch to the same type (normal to normal or inline to inline is
         * always successful). When returning false, the switching might not be supported for this type.
         */
        virtual bool Inline(bool bInline) override
        {
            return bInline == false;
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
    inline std::shared_ptr<CNode> CNodeCollection::Insert(uint32_t uiIndex, const CTokenRange& rrangeKeyPath, const TArgs&... rtArgs)
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
                    ptrTableArray->SetParentPtr(Cast<CNodeCollection>());

                    // Add to the list
                    m_lstNodes.push_back(ptrTableArray);
                }
                else
                    ptrTableArray = (*itNode)->template Cast<CTableArray>();
                if (!ptrTableArray)
                    throw XTOMLParseException("The node '" + prKey.first.get().StringValue() +
                        "' exists already, but is not a table array.");

                // Create the table.
                ptrNode = ptrTableArray->Insert<CTable>(sdv::toml::npos, CTokenRange(prKey.first, prKey.first.get().Next()),
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
                ptrNode->SetParentPtr(Cast<CNodeCollection>());

                // Add to the list
                m_lstNodes.push_back(ptrNode);

                // If the current node is implicit, take over the inline flag (this determines whether a sub-table definition is
                // allowed or not).
                if (!ExplicitlyDefined())
                    Inline(ptrNode->Inline());
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
                m_lstNodes.push_back(ptrNode);
            }

            // Insert the node in the child node
            std::shared_ptr<CNodeCollection> ptrNodeCollection = ptrNode->Cast<CNodeCollection>();
            if (!ptrNodeCollection)
                throw XTOMLParseException("Parent node is not an array or table '" + ptrNode->GetPath(true) + "'.");
            ptrNode = ptrNodeCollection->Insert<TNodeType>(sdv::toml::npos, prKey.second, rtArgs...);
            if (!ptrNode)
                throw XTOMLParseException("Could not create the node '" + prKey.first.get().StringValue() + "'.");

            // This node is not the parent, but still presents of the created node. Add the view pointer that they are linked.
            ptrNode->SetViewPtr(Cast<CNodeCollection>());
        }

        // Insert the node at the requested location if this node is inline or this is the view of the node.
        auto itPos = (static_cast<size_t>(uiIndex) >= m_vecNodeOrder.size()) ? m_vecNodeOrder.end() :
            m_vecNodeOrder.begin() + static_cast<size_t>(uiIndex);
        m_vecNodeOrder.insert(itPos, ptrNode);

        // Return the result
        return ptrNode;
    }


} // namespace toml_parser

#endif // !defined PARSER_NODE_TOML_H