#ifndef SDV_CONFIG_H
#define SDV_CONFIG_H

#include "../interfaces/toml.h"
#include "interface_ptr.h"
#include "local_service_access.h"

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
         * @brief Return the node name.
         * @return String containing the node name.
         */
        sdv::u8string GetName();

        /**
         * @brief Retrurn the node qualified path including the parent path.
         * @details The qualified path is a path composed through all parent nodes containing quoted names where needed. The path
         * to the node can be used to directly access the node.
         * @return String containing the qualified path to the node.
         */
        sdv::u8string GetQualifiedPath();

        /**
         * @brief Get the node type.
         * @return The node type.
         */
        ENodeType GetType();

        /**
         * @brief Get the node value if the node contains a value.
         * @return The node value.
         */
        sdv::any_t GetValue();

        /**
         * @brief Clear the node class.
         */
        virtual void Clear();

        /**
         * @brief Get the TOML string from this node including all children.
         * @return The TOMl string.
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
         * @brief Default constructor.
         */
        CTOMLParser() = default;

        /**
         * @brief Constructor providing automatic processing.
         * @param[in] rssConfig Reference to the configuration.
         */
        CTOMLParser(const std::string& rssConfig);

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

    inline CNode::CNode(const TInterfaceAccessPtr& rptrNode)
    {
        m_pNodeInfo = rptrNode.GetInterface<INodeInfo>();
        if (!m_pNodeInfo)
            return;
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

    inline sdv::u8string CNode::GetName()
    {
        return m_pNodeInfo ? m_pNodeInfo->GetName() : sdv::u8string();
    }

    inline sdv::u8string CNode::GetQualifiedPath()
    {
        return m_pNodeInfo ? m_pNodeInfo->GetPath(true) : sdv::u8string();
    }

    inline ENodeType CNode::GetType()
    {
        return m_pNodeInfo ? m_pNodeInfo->GetType() : ENodeType::node_invalid;
    }

    inline sdv::any_t CNode::GetValue()
    {
        return m_pNodeInfo ? m_pNodeInfo->GetValue() : sdv::any_t();
    }

    inline void CNode::Clear()
    {
        m_ptrNode = nullptr;
        m_pNodeInfo = nullptr;
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

}

#endif // !defined SDV_CONFIG_H