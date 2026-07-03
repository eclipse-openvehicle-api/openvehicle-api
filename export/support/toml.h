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
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#ifndef SDV_TOML_H
#define SDV_TOML_H

#include <charconv>
#include "../interfaces/toml.h"
#include "interface_ptr.h"
#include "local_service_access.h"
#include "interface_ptr.h"

namespace sdv::toml
{
    /**
     * @brief Node class allowing access to the type and value.
     */
    class CNode
    {
    public:
        /**
         * @brief Default constructor.
         */
        CNode() = default;

        /**
         * @brief Node assignment constructor.
         * @param[in] rptrNode Reference to the node interface.
         */
        CNode(const TInterfaceAccessPtr& rptrNode);

        /**
         * @brief Node assignment operator.
         * @param[in] rptrNode Reference to the node interface.
         * @return Reference to this class.
        */
        CNode& operator=(const TInterfaceAccessPtr& rptrNode);

        /**
         * @brief Does the class contain a valid node?
         * @return Returns 'true' if the class contains a valid node; 'false' otherwise.
         */
        virtual bool IsValid() const;

        /**
         * @brief Does the class contain a valid node?
         * @return Returns 'true' if the class contains a valid node; 'false' otherwise.
         */
        virtual operator bool() const;

        /**
         * @brief Get the underlying interface.
         * @return The interface pointer.
         */
        TInterfaceAccessPtr GetInterface();

        /**
         * @brief Return the node name.
         * @return String containing the node name.
         */
        sdv::u8string GetName() const;

        /**
         * @brief Retrurn the node qualified path including the parent path.
         * @details The qualified path is a path composed through all parent nodes containing quoted names where needed. The path
         * to the node can be used to directly access the node.
         * @return String containing the qualified path to the node.
         */
        sdv::u8string GetQualifiedPath() const;

        /**
         * @brief Get the node type.
         * @return The node type.
         */
        ENodeType GetType() const;

        /**
         * @brief Get the index of this node within the parent collection.
         * @return The index of the node within the parent collection node or npos when no parent is available.
         */
        uint32_t GetIndex() const;

        /**
         * @brief Is the node defined as inline node?
         * @return The inline flag.
         */
        bool IsInline() const;

        /**
         * @brief Return any associated comment text for this node.
         * @return The node comment or an empty string when there is not comment for this node.
         */
        std::string GetComment() const;

        /**
         * @brief Set comment for this node. Comments are placed before the node if this node is a table or an array and placed
         * behind the node if this node is a value node.
         * @param rssComment Reference to the comment string.
         */
        void SetComment(const std::string& rssComment);

        /**
         * @brief Get the node value if the node contains a value.
         * @return The node value or empty when the node doesn't have a value.
         */
        sdv::any_t GetValue() const;

        /**
         * @brief Automatically cast the value to a string.
         * @return String to return or an empty string.
         */
        std::string GetValueAsString() const;

        /**
         * @brief Automatically cast the value to a path (only for string values).
         * @return Path to return or an empty path.
         */
        std::filesystem::path GetValueAsPath() const;

        /**
         * @brief Set the node value if the node is a value node.
         * @remarks The value type depends on the node type and will be converted if necessary.
         * @param[in] ranyValue Reference to the value to set.
         * @return Returns 'true' on successful setting; 'false' otherwise.
         */
        bool SetValue(const sdv::any_t& ranyValue);

        /**
         * @brief Delete the node including all child nodes.
         * @return Returns 'true' on successful setting; 'false' otherwise.
         */
        bool Delete();

        /**
         * @brief Clear the node class.
         */
        virtual void Clear();

        /**
         * @brief Format the node automatically, remove redundant whitespace.
         * @param[in] bRemoveComments When set, the comments are removed from the node.
         */
        void AutomaticFormat(bool bRemoveComments);

        /**
         * @brief Get the TOML string from this node including all children.
         * @return The TOML string.
         */
        sdv::u8string GetTOML() const;

    protected:
        TInterfaceAccessPtr     m_ptrNode;              ///< Pointer to the node interface.
        INodeInfo*              m_pNodeInfo = nullptr;  ///< Node information interface.
    };

    /**
     * @brief Node collection class representing a table or array node.
     */
    class CNodeCollection : public CNode
    {
    public:
        /**
         * @brief Default constructor.
         */
        CNodeCollection() = default;

        /**
         * @brief Node assignment constructor.
         * @param[in] rptrNode Reference to the node interface.
         */
        CNodeCollection(const TInterfaceAccessPtr& rptrNode);

        /**
         * @brief Node assignment constructor.
         * @param[in] rNode Reference to the node class.
         */
        CNodeCollection(const CNode& rNode);

        /**
         * @brief Node assignment operator.
         * @param[in] rptrNode Reference to the node interface.
         * @return Reference to this class.
         */
        CNodeCollection& operator=(const TInterfaceAccessPtr& rptrNode);

        /**
         * @brief Node assignment operator.
         * @param[in] rNode Reference to the node class.
         * @return Reference to this class.
         */
        CNodeCollection& operator=(const CNode& rNode);

        /**
        * @brief Does the class contain a valid node?
        * @return Returns 'true' if the class contains a valid node; 'false' otherwise.
        */
        virtual bool IsValid() const override;

        /**
        * @brief Does the class contain a valid node?
        * @return Returns 'true' if the class contains a valid node; 'false' otherwise.
        */
        virtual operator bool() const override;

        /**
         * @brief Return the amount of nodes in this collection.
         * @return The amount of nodes.
         */
        size_t GetCount() const;

        /**
         * @brief Get the node at the provided index.
         * @param[in] nIndex The index number.
         * @return Returns the node if available or an empty node when not.
         */
        CNode Get(size_t nIndex) const;

        /**
         * @brief Get the node at the provided index.
         * @param[in] nIndex The index number.
         * @return Returns the node if available or an empty node when not.
         */
        CNode operator[](size_t nIndex) const;

        /**
         * @brief Clear the node class.
         */
        virtual void Clear() override;

        /**
         * @brief Get direct access to a node.
         * @details Elements of tables can be accessed and traversed by using '.' to separated the parent name from child name.
         * E.g. 'parent.child' would access the 'child' element of the 'parent' table. Elements of arrays can be accessed and
         * traversed by using the index number in brackets. E.g. 'array[3]' would access the fourth element of the array 'array'.
         * These access conventions can also be chained like 'table.array[2][1].subtable.integerElement'.
         * @param[in] rssNode Reference to the node string.
         * @return Returns the node when available or an empty node when not.
         */
        CNode GetDirect(const sdv::u8string& rssNode) const;

        /**
         * @brief Return the node name for a node in the collection. If the collection is an array, provides the index between
         * square brackets.
         * @param[in] nIndex Index to return the node name for.
         * @return Name of the node or an empty string if the index is larger than the amount of nodes in the collection.
         */
        std::string GetNodeNameByIndex(size_t nIndex) const;

        /**
         * @brief Insert a value node before the provided position.
         * @remarks In TOML, inline nodes are located before standard nodes. Since values are presented as inline node, they
         * will be inserted before any standard node (table or table array if defined as standard node).
         * @param[in] rssInsertBefore Name of the node to insert the value before. In case of an array, can be an index between
         * square brackets. Can be empty, causing the node to be inserted at the end.
         * @param[in] rssName Reference to the name of the new value node. If this collection is an array, the name is ignored.
         * Otherwise the name must be unique within this collection.
         * @param[in] ranyValue The value to assign to the node. The value also determines the type of value node.
         * @return Returns the node when successfully inserted or an empty node when not.
         */
        CNode InsertValue(const std::string& rssInsertBefore, const std::string& rssName, const sdv::any_t& ranyValue);

        /**
         * @brief Add a value node to the collection.
         * @param[in] rssName Reference to the name of the new array collection node. If this collection is an array, the name is
         * ignored. Otherwise the name must be unique within this collection.
         * @param[in] ranyValue The value to assign to the node. The value also determines the type of value node.
         * @return Returns the node when successfully added or an empty node when not.
         */
        CNode AddValue(const std::string& rssName, const sdv::any_t& ranyValue);

        /**
         * @brief Insert an array collection before the provided position.
         * @remarks In TOML, inline nodes are located before standard nodes. Since arrays are presented as inline node, they
         * will be inserted before any standard node (table or table array if defined as standard node).
         * @param[in] rssInsertBefore Name of the node to insert the value before. In case of an array, can be an index between
         * square brackets. Can be empty, causing the node to be inserted at the end.
         * @param[in] rssName Reference to the name of the new value node. If this collection is an array, the name is ignored.
         * Otherwise the name must be unique within this collection.
         * @return Returns the collection node when successfully inserted or an empty node when not.
         */
        CNodeCollection InsertArray(const std::string& rssInsertBefore, const std::string& rssName);

        /**
         * @brief Add an array collection to this collection.
         * @param[in] rssName Reference to the name of the new array collection node. If this collection is an array, the name is
         * ignored. Otherwise the name must be unique within this collection.
         * @return Returns the collection node when successfully added or an empty node when not.
         */
        CNodeCollection AddArray(const std::string& rssName);

        /**
         * @brief Insert a table collection before the provided position.
         * @remarks In TOML, inline nodes are located before standard nodes. Tables can be inserted as inline node, in which
         * case they will be inserted before any standard node (table or table array if defined as standard node).
         * @param[in] rssInsertBefore Name of the node to insert the value before. In case of an array, can be an index between
         * square brackets. Can be empty, causing the node to be inserted at the end.
         * @param[in] rssName Reference to the name of the new table collection node. If this collection is an array, the name is
         * ignored. Otherwise the name must be unique within this collection.
         * @param[in] bFavorInline When set, the node will be added as inline collection node. When not, the node will be inserted
         * as inline collection if the this collection is also inline, as standard when not.
         * @return Returns the collection node when successfully inserted or an empty node when not.
         */
        CNodeCollection InsertTable(const std::string& rssInsertBefore, const std::string& rssName, bool bFavorInline = false);

        /**
         * @brief Add a table collection to this collection.
         * @param[in] rssName Reference to the name of the new table collection node. If this collection is an array, the name is
         * ignored. Otherwise the name must be unique within this collection.
         * @param[in] bFavorInline When set, the node will be added as inline collection node. When not, the node will be inserted
         * as inline collection if the this collection is also inline, as standard when not.
         * @return Returns the collection node when successfully added or an empty node when not.
         */
        CNodeCollection AddTable(const std::string& rssName, bool bFavorInline = false);

        /**
         * @brief Insert a table array collection before the provided position.
         * @remarks A table array is an array with table inside. Inserting a table array node can also be done by creating an array,
         * if not existing already, and adding a table to the array.
         * @remarks In TOML, inline nodes are located before standard nodes. Table arrays can be inserted as inline node, in
         * which case they will be inserted before any standard node (table or table array if defined as standard node).
         * @param[in] rssInsertBefore Name of the node to insert the value before. In case of an array, can be an index between
         * square brackets. Can be empty, causing the node to be inserted at the end.
         * @param[in] rssName Reference to the name of the new table array node. If this collection is an array, the name is
         * ignored. Otherwise the name must be unique within this collection.
         * @param[in] bFavorInline When set, the node will be added as inline collection node. When not, the node will be inserted
         * as inline collection if the this collection is also inline, as standard when not.
         * @return Returns the collection node when successfully inserted or an empty node when not. The collection node represents
         * a table collection.
         */
        CNodeCollection InsertTableArray(const std::string& rssInsertBefore, const std::string& rssName, bool bFavorInline = false);

        /**
         * @brief Add a table array collection to this collection.
         * @remarks A table array is an array with table inside. Inserting a table array node can also be done by creating an array,
         * if not existing already, and adding a table to the array.
         * @param[in] rssName Reference to the name of the new table array node. If this collection is an array, the name is
         * ignored. Otherwise the name must be unique within this collection.
         * @param[in] bFavorInline When set, the node will be added as inline collection node. When not, the node will be inserted
         * as inline collection if the this collection is also inline, as standard when not.
         * @return Returns the collection node when successfully inserted or an empty node when not. The collection node represents
         * a table collection.
         */
        CNodeCollection AddTableArray(const std::string& rssName, bool bFavorInline = false);

        /**
         * @brief Insert a TOML string to the collection. All nodes specified in the TOML will be added in the collection except
         * when the nodes already exist. Comment and whitespace are preserved when possible.
         * @remarks In TOML, inline nodes are located before standard nodes. Dependable on the nodes defined in the TOML they
         * might be transferred to inline or they might be inserted at a different location.
         * @param[in] rssInsertBefore Name of the node to insert the value before. In case of an array, can be an index between
         * square brackets. Can be empty, causing the node to be inserted at the end.
         * @param[in] rssTOML Reference to the TOML string containing the nodes. The TOML string can be empty, which is not an
         * error. If required the TOML nodes are converted to inline nodes.
         * @param[in] bAllowPartial When set, duplicate nodes (already present in this collection) will be ignored and do not
         * cause an error. When not set, duplicate nodes will cause the function to return without inserting any node.
         * @return Returns 1 if the complete TOMl could be inserted, 0 if no TOML could be inserted or -1 when the TOML could be
         * partially inserted.
         */
        int InsertTOML(const std::string& rssInsertBefore, const std::string& rssTOML, bool bAllowPartial = false);

        /**
         * @brief Add a TOML string to this collection. All nodes specified in the TOML will be added in the collection except
         * when the nodes already exist. Comment and whitespace are preserved when possible.
         * @remarks The actual position depends on the type of node and the order the nodes are stored. Inline nodes come before
         * standard nodes.
         * @param[in] rssTOML Reference to the TOML string containing the nodes. The TOML string can be empty, which is not an
         * error. If required the TOML nodes are converted to inline nodes.
         * @param[in] bAllowPartial When set, duplicate nodes (already present in this collection) will be ignored and do not
         * cause an error. When not set, duplicate nodes will cause the function to return without inserting any node.
         * @return Returns 1 if the complete TOMl could be inserted, 0 if no TOML could be inserted or -1 when the TOML could be
         * partially inserted.
         */
        int AddTOML(const std::string& rssTOML, bool bAllowPartial = false);

    private:
        INodeCollection*    m_pCollection = nullptr;        ///< Pointer to the node collection interface.
    };

    /**
     * @brief TOML parser class
     */
    class CTOMLParser : public CNodeCollection
    {
    public:
        /**
         * @brief Constructor providing automatic processing.
         * @param[in] rssConfig Reference to the configuration.
         */
        CTOMLParser(const std::string& rssConfig = "");

        /**
         * @brief Process a configuration. This will clear any previous configuration.
         * @param[in] rssConfig Reference to the configuration.
         * @return Returns 'true' when the processing was successful; 'false# when not.
         */
        bool Process(const std::string& rssConfig);

        /**
         * @brief Returns whether a valid processed configuration is available.
         * @return Returns 'true' when the configuration is valid; 'false' when not.
         */
        virtual bool IsValid() const override;

        /**
         * @brief Returns whether a valid processed configuration is available.
         * @return Returns 'true' when the configuration is valid; 'false' when not.
         */
        virtual operator bool() const override;

        // Ignore cppcheck warning for not using dynamic binding when being called through the destructor.
        // cppcheck-suppress virtualCallInConstructor
        /**
         * @brief Clear the current configuration.
         */
        virtual void Clear() override;

    private:
        TObjectPtr      m_ptrParserUtil;        ///< TOML parser utility
        ITOMLParser*    m_pParser = nullptr;    ///< Pointer to the parser interface.
    };

    /**
     * @brief Compare flags
     */
    enum class ECompareFlags : uint32_t
    {
        compare_ignore_whitespace = 1,  ///< Compare, but ignore whitespace
        compare_ignore_comments = 2,    ///< Compare, but ignore comments
        compare_ignore_inline = 8,      ///< Compare, but ignore inline or explicit
        compare_ignore_all = 255,       ///< Compare with all ignore flags
    };

    /**
     * @brief Compare result
     */
    enum class ECompareResult : int32_t
    {
        compare_identical = 0,          ///< The comparison resulted into identical TOML strings
        compare_different = 1,          ///< The comparison resulted into different TOML strings
        compare_error = -1              ///< The comparison could not be done due to a failure
    };

    /// Internal namespace
    namespace internal
    {
        /**
         * @brief Compare the content of the node with the content of another node.
         * @attention This function changes the nodes dependable on the provided flags.
         * @param[in] rnode1 First node interface to compare with the second node.
         * @param[in] rnode2 Second node interface to compare with the first node.
         * @param[in] uiCompareFlags Zero or more flags from ECompareFlags.
         * @return The comparison result.
         */
        ECompareResult CompareNodes(CNode& rnode1, CNode& rnode2,
            uint32_t uiCompareFlags = static_cast<uint32_t>(ECompareFlags::compare_ignore_all));
    } // namespace internal;

    /**
     * @brief Compare the content of TOML string with the content of another TOML string.
     * @param[in] rssToml1 Reference to the first TOML string to compare with the second TOML string.
     * @param[in] rssToml2 Reference to the second TOML string to compare with the first TOML string.
     * @param[in] uiCompareFlags Zero or more flags from ECompareFlags.
     * @return The comparison result.
     */
    ECompareResult Compare(const std::string& rssToml1, const std::string& rssToml2,
        uint32_t uiCompareFlags = static_cast<uint32_t>(ECompareFlags::compare_ignore_all));

    inline CNode::CNode(const TInterfaceAccessPtr& rptrNode)
    {
        m_pNodeInfo = rptrNode.GetInterface<INodeInfo>();
        if (!m_pNodeInfo) return;
        m_ptrNode = rptrNode;
    }

    inline CNode& CNode::operator=(const TInterfaceAccessPtr& rptrNode)
    {
        CNode::Clear();
        m_pNodeInfo = rptrNode.GetInterface<INodeInfo>();
        if (!m_pNodeInfo)
            return *this;
        m_ptrNode = rptrNode;
        return *this;
    }

    inline bool CNode::IsValid() const
    {
        return m_pNodeInfo ? true : false;
    }

    inline CNode::operator bool() const
    {
        return m_pNodeInfo ? true : false;
    }

    inline TInterfaceAccessPtr CNode::GetInterface()
    {
        return m_ptrNode;
    }

    inline sdv::u8string CNode::GetName() const
    {
        return m_pNodeInfo ? m_pNodeInfo->GetName() : sdv::u8string();
    }

    inline sdv::u8string CNode::GetQualifiedPath() const
    {
        return m_pNodeInfo ? m_pNodeInfo->GetPath(true) : sdv::u8string();
    }

    inline ENodeType CNode::GetType() const
    {
        return m_pNodeInfo ? m_pNodeInfo->GetType() : ENodeType::node_invalid;
    }

    inline uint32_t CNode::GetIndex() const
    {
        return m_pNodeInfo ? m_pNodeInfo->GetIndex() : npos;
    }

    inline bool CNode::IsInline() const
    {
        return m_pNodeInfo ? m_pNodeInfo->IsInline() : true;
    }

    inline std::string CNode::GetComment() const
    {
        if (!m_pNodeInfo) return {};
        std::string ssComment;
        switch (GetType())
        {
        case ENodeType::node_boolean:
        case ENodeType::node_integer:
        case ENodeType::node_floating_point:
        case ENodeType::node_string:
            ssComment = m_pNodeInfo->GetComment(INodeInfo::ECommentType::comment_behind);
            if (ssComment.empty())
                ssComment = m_pNodeInfo->GetComment(INodeInfo::ECommentType::comment_before);
            break;
        default:
            ssComment = m_pNodeInfo->GetComment(INodeInfo::ECommentType::comment_before);
            if (ssComment.empty())
                ssComment = m_pNodeInfo->GetComment(INodeInfo::ECommentType::comment_behind);
            break;
        }
        return ssComment;
    }

    inline void CNode::SetComment(const std::string& rssComment)
    {
        if (!m_pNodeInfo) return;
        switch (GetType())
        {
        case ENodeType::node_boolean:
        case ENodeType::node_integer:
        case ENodeType::node_floating_point:
        case ENodeType::node_string:
            m_pNodeInfo->SetComment(INodeInfo::ECommentType::comment_behind, rssComment);
            break;
        default:
            m_pNodeInfo->SetComment(INodeInfo::ECommentType::comment_before, rssComment);
            break;
        }
    }

    inline sdv::any_t CNode::GetValue() const
    {
        return m_pNodeInfo ? m_pNodeInfo->GetValue() : sdv::any_t();
    }

    inline std::string CNode::GetValueAsString() const
    {
        return m_pNodeInfo ? m_pNodeInfo->GetValue().get<std::string>() : std::string();
    }

    inline std::filesystem::path CNode::GetValueAsPath() const
    {
        return m_pNodeInfo ? m_pNodeInfo->GetValue().get<std::filesystem::path>() : std::filesystem::path();
    }

    inline bool CNode::SetValue(const sdv::any_t& ranyValue)
    {
        INodeUpdate* pNodeUpdate = m_ptrNode.GetInterface<INodeUpdate>();
        if (!pNodeUpdate) return false;
        return pNodeUpdate->ChangeValue(ranyValue);
    }

    inline bool CNode::Delete()
    {
        INodeUpdate* pNodeUpdate = m_ptrNode.GetInterface<INodeUpdate>();
        if (!pNodeUpdate) return false;
        bool bRet = pNodeUpdate->DeleteNode();
        Clear();    // Not valid any more.
        return bRet;
    }

    inline void CNode::Clear()
    {
        m_ptrNode = nullptr;
        m_pNodeInfo = nullptr;
    }

    inline void CNode::AutomaticFormat(bool bRemoveComments)
    {
        if (m_pNodeInfo) m_pNodeInfo->AutomaticFormat(bRemoveComments);
    }

    inline sdv::u8string CNode::GetTOML() const
    {
        return m_pNodeInfo ? m_pNodeInfo->GetTOML() : sdv::u8string();
    }

    inline CNodeCollection::CNodeCollection(const TInterfaceAccessPtr& rptrNode) : CNode(rptrNode)
    {
        m_pCollection = rptrNode.GetInterface<INodeCollection>();
        if (!m_pCollection) CNode::Clear();
    }

    inline CNodeCollection::CNodeCollection(const CNode& rNode) : CNode(rNode)
    {
        m_pCollection = m_ptrNode.GetInterface<INodeCollection>();
        if (!m_pCollection) CNode::Clear();
    }

    inline CNodeCollection& CNodeCollection::operator=(const TInterfaceAccessPtr& rptrNode)
    {
        CNode::operator=(rptrNode);
        m_pCollection = rptrNode.GetInterface<INodeCollection>();
        if (!m_pCollection) CNode::Clear();
        return *this;
    }

    inline CNodeCollection& CNodeCollection::operator=(const CNode& rNode)
    {
        CNode::operator=(rNode);
        m_pCollection = m_ptrNode.GetInterface<INodeCollection>();
        if (!m_pCollection) CNode::Clear();
        return *this;
    }

    inline bool CNodeCollection::IsValid() const
    {
        return m_pCollection ? true : false;
    }

    inline CNodeCollection::operator bool() const
    {
        return m_pCollection ? true : false;
    }

    inline size_t CNodeCollection::GetCount() const
    {
        return m_pCollection ? m_pCollection->GetCount() : 0;
    }

    inline CNode CNodeCollection::Get(size_t nIndex) const
    {
        return m_pCollection ? CNode(m_pCollection->GetNode(static_cast<uint32_t>(nIndex))) : CNode();
    }

    inline CNode CNodeCollection::operator[](size_t nIndex) const
    {
        return m_pCollection ? CNode(m_pCollection->GetNode(static_cast<uint32_t>(nIndex))) : CNode();
    }

    inline void CNodeCollection::Clear()
    {
        CNode::Clear();
        m_pCollection = nullptr;
    }

    inline CNode CNodeCollection::GetDirect(const sdv::u8string& rssNode) const
    {
        return m_pCollection ? CNode(m_pCollection->GetNodeDirect(rssNode)) : CNode();
    }

    inline std::string CNodeCollection::GetNodeNameByIndex(size_t nIndex) const
    {
        if (!m_pCollection) return {};
        if (nIndex >= m_pCollection->GetCount()) return {};
        if (GetType() == ENodeType::node_array) return "[" + std::to_string(nIndex) + "]";
        TInterfaceAccessPtr ptrNode = m_pCollection->GetNode(static_cast<uint32_t>(nIndex));
        const auto* pAccess = ptrNode.GetInterface<INodeInfo>();
        if (!pAccess) return {};
        return pAccess->GetName();
    }

    inline CNode CNodeCollection::InsertValue(const std::string& rssInsertBefore, const std::string& rssName,
        const sdv::any_t& ranyValue)
    {
        INodeCollectionInsert* pInsert = m_ptrNode.GetInterface<INodeCollectionInsert>();
        if (!pInsert) return {};
        return CNode(pInsert->InsertValue(rssInsertBefore, rssName, ranyValue));
    }

    inline CNode CNodeCollection::AddValue(const std::string& rssName, const sdv::any_t& ranyValue)
    {
        INodeCollectionInsert* pInsert = m_ptrNode.GetInterface<INodeCollectionInsert>();
        if (!pInsert) return {};
        return CNode(pInsert->InsertValue("", rssName, ranyValue));
    }

    inline CNodeCollection CNodeCollection::InsertArray(const std::string& rssInsertBefore, const std::string& rssName)
    {
        INodeCollectionInsert* pInsert = m_ptrNode.GetInterface<INodeCollectionInsert>();
        if (!pInsert) return {};
        return CNodeCollection(pInsert->InsertArray(rssInsertBefore, rssName));
    }

    inline CNodeCollection CNodeCollection::AddArray(const std::string& rssName)
    {
        INodeCollectionInsert* pInsert = m_ptrNode.GetInterface<INodeCollectionInsert>();
        if (!pInsert) return {};
        return CNodeCollection(pInsert->InsertArray("", rssName));
    }

    inline CNodeCollection CNodeCollection::InsertTable(const std::string& rssInsertBefore, const std::string& rssName,
        bool bFavorInline /*= false*/)
    {
        INodeCollectionInsert* pInsert = m_ptrNode.GetInterface<INodeCollectionInsert>();
        if (!pInsert) return {};
        return CNodeCollection(pInsert->InsertTable(rssInsertBefore, rssName, bFavorInline ? EInsertPreference::prefer_inline :
                EInsertPreference::prefer_standard));
    }

    inline CNodeCollection CNodeCollection::AddTable(const std::string& rssName, bool bFavorInline /*= false*/)
    {
        INodeCollectionInsert* pInsert = m_ptrNode.GetInterface<INodeCollectionInsert>();
        if (!pInsert) return {};
        return CNodeCollection(pInsert->InsertTable("", rssName, bFavorInline ? EInsertPreference::prefer_inline :
                EInsertPreference::prefer_standard));
    }

    inline CNodeCollection CNodeCollection::InsertTableArray(const std::string& rssInsertBefore, const std::string& rssName,
        bool bFavorInline /*= false*/)
    {
        INodeCollectionInsert* pInsert = m_ptrNode.GetInterface<INodeCollectionInsert>();
        if (!pInsert) return {};
        return CNodeCollection(pInsert->InsertTableArray(rssInsertBefore, rssName, bFavorInline ? EInsertPreference::prefer_inline :
                EInsertPreference::prefer_standard));
    }

    inline CNodeCollection CNodeCollection::AddTableArray(const std::string& rssName, bool bFavorInline /*= false*/)
    {
        INodeCollectionInsert* pInsert = m_ptrNode.GetInterface<INodeCollectionInsert>();
        if (!pInsert) return {};
        return CNodeCollection(pInsert->InsertTableArray("", rssName, bFavorInline ? EInsertPreference::prefer_inline :
                EInsertPreference::prefer_standard));
    }

    inline int CNodeCollection::InsertTOML(const std::string& rssInsertBefore, const std::string& rssTOML,
        bool bAllowPartial /*= false*/)
    {
        INodeCollectionInsert* pInsert = m_ptrNode.GetInterface<INodeCollectionInsert>();
        if (!pInsert) return 0;
        INodeCollectionInsert::EInsertResult eRet = pInsert->InsertTOML(rssInsertBefore, rssTOML, !bAllowPartial);
        switch (eRet)
        {
        case INodeCollectionInsert::EInsertResult::insert_success:
            return 1;
        case INodeCollectionInsert::EInsertResult::insert_partly_success:
            return -1;
        case INodeCollectionInsert::EInsertResult::insert_fail:
        default:
            return 0;
        }
    }

    inline int CNodeCollection::AddTOML(const std::string& rssTOML, bool bAllowPartial /*= false*/)
    {
        INodeCollectionInsert* pInsert = m_ptrNode.GetInterface<INodeCollectionInsert>();
        if (!pInsert) return 0;
        INodeCollectionInsert::EInsertResult eRet = pInsert->InsertTOML("", rssTOML, !bAllowPartial);
        switch (eRet)
        {
        case INodeCollectionInsert::EInsertResult::insert_success:
            return 1;
        case INodeCollectionInsert::EInsertResult::insert_partly_success:
            return -1;
        case INodeCollectionInsert::EInsertResult::insert_fail:
        default:
            return 0;
        }
    }

    inline CTOMLParser::CTOMLParser(const std::string& rssConfig)
    {
        Process(rssConfig);
    }

    inline bool CTOMLParser::Process(const std::string& rssConfig)
    {
        Clear();
        m_ptrParserUtil = sdv::core::CreateUtility("TOMLParserUtility");
        m_pParser = m_ptrParserUtil.GetInterface<ITOMLParser>();
        if (m_pParser)
        {
            try
            {
                m_pParser->Process(rssConfig);
                CNodeCollection::operator=(m_ptrParserUtil);
            }
            catch (const sdv::toml::XTOMLParseException&)
            {
                Clear();
                return false;
            }
        }
        return IsValid();
    }

    inline bool CTOMLParser::IsValid() const
    {
        return m_pParser ? true : false;
    }

    inline CTOMLParser::operator bool() const
    {
        return m_pParser ? true : false;
    }

    inline void CTOMLParser::Clear()
    {
        m_pParser = nullptr;
        m_ptrParserUtil.Clear();
    }

    namespace internal
    {
        inline ECompareResult CompareNodes(CNode& rnode1, CNode& rnode2,
            uint32_t uiCompareFlags /*= static_cast<uint32_t>(ECompareFlags::compare_ignore_all)*/)
        {
            if (!rnode1 || !rnode2) return ECompareResult::compare_error;

            // Format the nodes when ignore whitespace and/or comments
            bool bIgnoreComments = uiCompareFlags & static_cast<uint32_t>(ECompareFlags::compare_ignore_comments);
            bool bIgnoreWhitespace = bIgnoreComments ||
                (uiCompareFlags & static_cast<uint32_t>(ECompareFlags::compare_ignore_whitespace));
            if (bIgnoreWhitespace)
            {
                INodeInfo* pNodeInfo1 = rnode1.GetInterface().GetInterface<INodeInfo>();
                if (!pNodeInfo1) return ECompareResult::compare_error;
                pNodeInfo1->AutomaticFormat(bIgnoreComments);
                INodeInfo* pNodeInfo2 = rnode2.GetInterface().GetInterface<INodeInfo>();
                if (!pNodeInfo2) return ECompareResult::compare_error;
                pNodeInfo2->AutomaticFormat(bIgnoreComments);
            }
    
            // Convert to standard if ignoring inline
            INodeCollectionConvert* pConvert1 = rnode1.GetInterface().GetInterface<INodeCollectionConvert>();
            INodeCollectionConvert* pConvert2 = rnode2.GetInterface().GetInterface<INodeCollectionConvert>();
            if (pConvert1 && pConvert2 && uiCompareFlags & static_cast<uint32_t>(ECompareFlags::compare_ignore_inline))
            {
                // Making inline nodes as standard, might only change the upper node.
                // Making standard nodes inline, will have all child nodes be made inline as well, because inline nodes can only
                // have inline nodes.
                pConvert1->MakeInline();
                pConvert2->MakeInline();
            }
    
            // Generate the TOMLs and compare
            return rnode1.GetTOML() == rnode2.GetTOML() ? ECompareResult::compare_identical : ECompareResult::compare_different;
        }
    } // namespace internal

    inline ECompareResult Compare(const std::string& rssToml1, const std::string& rssToml2,
        uint32_t uiCompareFlags /*= static_cast<uint32_t>(ECompareFlags::compare_ignore_all)*/)
    {
        try
        {
            CTOMLParser parser1(rssToml1);
            CTOMLParser parser2(rssToml2);
            return internal::CompareNodes(parser1, parser2, uiCompareFlags);
        }
        catch (const sdv::toml::XTOMLParseException&)
        {
            return ECompareResult::compare_error;
        }
    }

}

#endif // !defined SDV_TOML_H