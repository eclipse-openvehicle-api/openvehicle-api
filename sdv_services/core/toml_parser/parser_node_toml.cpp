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

#include <algorithm>
#include "parser_node_toml.h"
#include "exception.h"
#include <sstream>
#include "parser_toml.h"
#include <algorithm>
#include <charconv>

/// The TOML parser namespace
namespace toml_parser
{
    CGenContext::CGenContext(const std::string& rssPrefixKey /*= std::string()*/, uint32_t uiOptions /*= 0*/) :
        m_ssPrefixKey(rssPrefixKey), m_uiOptions(uiOptions)
    {}

    void CGenContext::InitTopMostNode(const std::shared_ptr<const CNode>& rptrNode)
    {
        if (!m_ptrTopMostNode)
        {
            m_ptrTopMostNode = rptrNode;

            // When initialized, this is the only node and therefore also the last node...
            m_bFinalLastNode = true;
            m_bLastNode = true;

            // Extract node specific generation context
            ExtractContext(rptrNode);
        }
    }

    bool CGenContext::IsPartOfView(const std::shared_ptr<const CNode>& rptrNode) const
    {
        if (!rptrNode) return false;
        if (!m_ptrTopMostNode) return false;
        if (rptrNode->Inline()) return true;
        if (!rptrNode->GetParentPtr()) return false;
        return m_bTopMost || rptrNode->GetParentPtr() == m_ptrTopMostNode;
    }

    CGenContext CGenContext::CopyWithContext(const std::string& rssNewKeyContext, const std::shared_ptr<CNode>& rptrNode,
        bool bLastNode) const
    {
        CGenContext context(m_ssPrefixKey, m_uiOptions);
        
        // Copy the base context
        context.m_ssKeyContext = rssNewKeyContext;
        context.m_ptrTopMostNode = m_ptrTopMostNode;
        context.m_bTopMost = false;
        context.m_bLastNode = bLastNode;
        context.m_bFinalLastNode = bLastNode && m_bFinalLastNode;

        // Extract node specific generation context
        context.ExtractContext(rptrNode);

        return context;
    }

    const std::string& CGenContext::PrefixKey() const
    {
        return m_ssPrefixKey;
    }

    const std::string& CGenContext::KeyContext() const
    {
        return m_ssKeyContext;
    }

    const std::string& CGenContext::KeyPath() const
    {
        return m_ssKeyPath;
    }

    const std::string& CGenContext::FullKeyPath() const
    {
        return m_ssFullKeyPath;
    }

    const std::string& CGenContext::RelKeyPath() const
    {
        return m_ssRelKeyPath;
    }

    bool CGenContext::TopMostNode() const
    {
        return m_bTopMost;
    }

    void CGenContext::SetOption(EGenerateOptions eOption)
    {
        m_uiOptions |= static_cast<uint32_t>(eOption);
    }

    bool CGenContext::CheckOption(EGenerateOptions eOption) const
    {
        return (m_uiOptions & static_cast<uint32_t>(eOption)) ? true : false;
    }

    bool CGenContext::LastNode() const
    {
        return m_bLastNode;
    }

    CGenContext::EPresentation CGenContext::Presentation() const
    {
        return m_ePresentation;
    }

    bool CGenContext::Standard() const
    {
        return m_ePresentation == EPresentation::standard;
    }

    bool CGenContext::Inline() const
    {
        return m_ePresentation != EPresentation::standard;
    }

    bool CGenContext::Embedded() const
    {
        return m_ePresentation == EPresentation::embedded;
    }

    bool CGenContext::Assignment() const
    {
        return m_bAssignment;
    }

    bool CGenContext::CommaNeeded() const
    {
        return m_bCommaNeeded;
    }

    bool CGenContext::CommentAndNewlineAllowed() const
    {
        return !m_bOneLine && !CheckOption(EGenerateOptions::no_comments);
    }

    bool CGenContext::NewlineAllowed() const
    {
        return !m_bOneLine && !CheckOption(EGenerateOptions::no_comments) && !CheckOption(EGenerateOptions::reduce_whitespace);
    }

    bool CGenContext::FinalNewline() const
    {
        return m_bFinalNewline;
    }

    void CGenContext::ExtractContext(const std::shared_ptr<const CNode>& rptrNode)
    {
        if (!rptrNode) return;
        auto ptrParent = rptrNode->GetParentPtr();
        if (ptrParent && ptrParent->Inline() && ptrParent->ExplicitlyDefined())
            m_ePresentation = EPresentation::embedded;
        else if (rptrNode->Inline())
            m_ePresentation = EPresentation::standard_inline;

        // Get pointers
        std::shared_ptr<const CArray> ptrParentArray, ptrArray;
        if (ptrParent) ptrParentArray = ptrParent->Cast<CArray>();
        ptrArray = rptrNode->Cast<CArray>();
        std::shared_ptr<const CTable> ptrParentTable, ptrTable;
        if (ptrParent) ptrParentTable = ptrParent->Cast<CTable>();
        ptrTable = rptrNode->Cast<CTable>();
        std::shared_ptr<const CValueNode> ptrValueNode = rptrNode->Cast<CValueNode>();

        // Inline or embedded table nodes must be defined on one line, except when multi-line-strings are used. Comments and
        // syntax-base newlines are not allowed.
        if (ptrParent && (m_ePresentation == EPresentation::embedded))
            m_bOneLine = ptrParent->Cast<CTable>() ? true : false;

        // Assignements are not allowed within inline arrays
        m_bAssignment = !(m_ePresentation == EPresentation::embedded && ptrParentArray);

        // Separation commas are needed for embedded nodes. A final trailing comma behind the last node is allowed for array
        // elements only (not for the table elements).
        if (m_ePresentation == EPresentation::embedded)
            m_bCommaNeeded = !m_bLastNode || (ptrParentArray && ptrParentArray->LastNodeWithSucceedingComma());

        // Final new line for non-embedded nodes, which are not the last node, unless the last node is a standard table with
        // children.
        m_bFinalNewline = m_ePresentation != EPresentation::embedded &&
            !(m_bFinalLastNode && 
                !(m_ePresentation == EPresentation::standard && (ptrParentTable || ptrParentArray || !m_ssPrefixKey.empty()) &&
                    (ptrTable && ptrTable->GetCount())));

        // Extract the correct context and the kex paths
        if (m_ssKeyContext.empty())
        {
            m_ssKeyContext = m_ssPrefixKey;
            std::string ssGenContext;
            if (m_bTopMost)
            {
                if (ptrArray || ptrValueNode)   // Inline array or value.
                {
                    if (ptrParent)
                        ssGenContext = static_cast<std::string>(ptrParent->GetPath(false));
                }
                else if (!(CheckOption(EGenerateOptions::full_header) && ptrTable))  //  Not table entry and full header enabled.
                    ssGenContext = static_cast<std::string>(rptrNode->GetPath(false));
            }
            if (!m_ssKeyContext.empty() && !ssGenContext.empty())
                m_ssKeyContext += ".";
            m_ssKeyContext += ssGenContext;
        }
        m_ssKeyPath = m_ssPrefixKey;
        m_ssFullKeyPath = m_ssKeyContext.empty() ? m_ssPrefixKey : m_ssKeyContext;
        m_ssRelKeyPath  = rptrNode->GetCustomPath(m_ssPrefixKey, m_ssKeyContext.empty() ? m_ssPrefixKey : m_ssKeyContext);
        if (!m_ssKeyPath.empty() && !m_ssRelKeyPath.empty())
            m_ssKeyPath += ".";
        if (!m_ssFullKeyPath.empty() && !m_ssRelKeyPath.empty())
            m_ssFullKeyPath += ".";
        m_ssKeyPath += m_ssRelKeyPath;
        m_ssFullKeyPath += m_ssRelKeyPath;
    }

    CNode::CNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName) :
        m_index(rparser.Indexer().CreateIndex()), m_ssName(rssName), m_ssRawName(rssRawName), m_refParser(rparser)
    {}

    CNode::~CNode()
    {}

    CParser& CNode::Parser()
    {
        return m_refParser.get();
    }

    const CParser& CNode::Parser() const
    {
        return m_refParser.get();
    }

    const CNodeIndex& CNode::NodeIndex() const
    {
        return m_index;
    }

    CNodeIndex& CNode::NodeIndex()
    {
        return m_index;
    }

    bool CNode::operator<(const CNode& rNode) const
    {
        return m_index < rNode.m_index;
    }

    bool CNode::operator<(const std::shared_ptr<CNode>& rptrNode) const
    {
        return m_index < rptrNode->m_index;
    }

    sdv::u8string CNode::GetName() const
    {
        return m_ssName;
    }

    sdv::u8string CNode::GetPath(bool bResolveArrays) const
    {
        bool bRoot = dynamic_cast<const CRootTable*>(this) ? true : false;
        if (!m_bDeleted && !bRoot)  // Do not include the root table in the path.
        {
            // Get the parent
            std::string ssName;
            auto ptrParent = m_ptrParent.lock();
            if (ptrParent && ptrParent->Cast<CArray>())
            {
                if (bResolveArrays)
                    ssName = "[" +
                        std::to_string(ptrParent->Cast<CArray>()->FindIndex(std::const_pointer_cast<CNode>(shared_from_this())))
                        + "]";
            }
            else
                ssName = QuoteText(m_ssName, EQuoteRequest::smart_key);
            if (!ptrParent) return ssName;

            // Compose the path. Add a dot after a valid parent, except if the parent is an array.
            std::string ssPath = ptrParent->GetPath(bResolveArrays);
            if (!ssPath.empty() && !ptrParent->Cast<CArray>())
                ssPath += ".";
            ssPath += ssName;
            return ssPath;
        }

        return {};
    }

    std::list<std::pair<std::string, std::string>> CNode::GetRawPath(bool bResolveArrays) const
    {
        bool bRoot = dynamic_cast<const CRootTable*>(this) ? true : false;
        if (!m_bDeleted && !bRoot) // Do not include the root table in the path.
        {
            // Get the parent
            std::string ssName, ssRawName;
            auto ptrParent = m_ptrParent.lock();
            if (ptrParent && ptrParent->Cast<CArray>())
            {
                if (bResolveArrays)
                    ssName =
                        "["
                        + std::to_string(ptrParent->Cast<CArray>()->FindIndex(std::const_pointer_cast<CNode>(shared_from_this())))
                        + "]";
            }
            else
            {
                ssName = m_ssName;
                ssRawName = m_ssRawName.empty() ? QuoteText(m_ssName, EQuoteRequest::smart_key) : m_ssRawName;
            }
            if (!ptrParent) // Up-most leaf
                return std::list<std::pair<std::string, std::string>>{std::make_pair(ssName, ssRawName)};

            // Compose the path. Add a dot after a valid parent, except if the parent is an array.
            std::list<std::pair<std::string, std::string>> lstKeyPath = ptrParent->GetRawPath(bResolveArrays);
            lstKeyPath.push_back(std::make_pair(ssName, ssRawName));
            return lstKeyPath;
        }

        return {};
    }

    sdv::any_t CNode::GetValue() const
    {
        return sdv::any_t();
    }

    uint32_t CNode::GetIndex() const
    {
        auto ptrParent = GetParentPtr();
        if (!ptrParent) return sdv::toml::npos;
        return ptrParent->FindIndex(std::const_pointer_cast<CNode>(shared_from_this()));
    }

    sdv::IInterfaceAccess* CNode::GetParent() const
    {
        std::shared_ptr<CNodeCollection> ptrParent = m_ptrParent.lock();
        if (!ptrParent) return nullptr;
        return ptrParent.get();
    }

    sdv::u8string CNode::GetTOML() const
    {
        if (m_bDeleted) return {};

        return GenerateTOML();
    }

    void CNode::SetComment(sdv::toml::INodeInfo::ECommentType eType, const sdv::u8string& ssComment)
    {
        CodeSnippet(eType).SetComment(ssComment);
    }

    sdv::u8string CNode::GetComment(sdv::toml::INodeInfo::ECommentType eType)
    {
        return CodeSnippet(eType).GetComment();
    }

    void CNode::AutomaticFormat(/*in*/ bool bRemoveComments)
    {
        if (bRemoveComments)
            m_vecCodeSnippets.clear();
        else
        {
            // Automatically format the code snippets of the node.
            for (auto& rmapSnippets : m_vecCodeSnippets)
            {
                for (auto& rvtSnippet : rmapSnippets)
                {
                    rvtSnippet.second.RemoveFormat();
                }
            }
        }
    }

    bool CNode::IsInline() const
    {
        return Inline();
    }

    bool CNode::IsStandard() const
    {
        return !Inline();
    }

    void CNode::UpdateNodeCode(const CNodeTokenRange& rNodeRange)
    {
        // Extract the node pre and post comments and whitespace (belonging and not belonging to the node).
        CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before).SetTokenList(
            rNodeRange.LinesBeforeNode().TokenListSLice());
        CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind).SetTokenList(
            rNodeRange.LinesBehindNode().TokenListSLice());
        CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_before).SetTokenList(
            rNodeRange.NodeCommentsBefore().TokenListSLice());
        CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_behind).SetTokenList(
            rNodeRange.NodeCommentsBehind().TokenListSLice());
    }

    bool CNode::ChangeName(const sdv::u8string& ssNewName)
    {
        // Deleted?
        if (m_bDeleted) return false;
        
        // Has parent?
        std::shared_ptr<CNodeCollection> ptrParent = m_ptrParent.lock();
        if (!ptrParent) return false;

        // Node with identical name exists?
        if (ptrParent->Direct(ssNewName)) return false;

        m_ssName = ssNewName;

        return true;
    }

    bool CNode::ChangeValue(sdv::any_t /*anyNewValue*/)
    {
        // Default implementation doesn't do anything.
        return false;
    }

    bool CNode::MoveUp()
    {
        // TODO...
        return false;
    }

    bool CNode::MoveDown()
    {
        // TODO...
        return false;
    }

    bool CNode::DeleteNode()
    {
        bool bRet = false;

        // Block rebuilding
        auto lock = Parser().CreateRebuildLockObject();

        // Remove the node from the parent.
        auto ptrParent = GetParentPtr();
        if (ptrParent)
        {
            // Remove the node from the parent --> this will put the node in the recycle bin.
            bRet = ptrParent->DeleteNode(std::const_pointer_cast<CNode>(shared_from_this()));
        }

        // Remove the parent.
        m_ptrParent.reset();

        // Set the node to be deleted.
        m_bDeleted = true;

        return bRet;
    }

    bool CNode::IsDeleted() const
    {
        return m_bDeleted;
    }

    void CNode::SetParentPtr(const std::shared_ptr<CNodeCollection>& rptrParent)
    {
        auto ptrOldParent = m_ptrParent.lock();
        if (rptrParent == ptrOldParent) return; // Nothing to do...
        if (ptrOldParent)
        {
            auto itNode = std::find(ptrOldParent->m_lstNodes.begin(), ptrOldParent->m_lstNodes.end(), shared_from_this());
            if (itNode != ptrOldParent->m_lstNodes.end())
                ptrOldParent->m_lstNodes.erase(itNode);
        }

        // Assign the parent
        m_ptrParent = rptrParent;
        if (!rptrParent) return;

        // Assign the parser of the parent
        ReassignParser(rptrParent->Parser());

        // Add the node to the node list if not already present.
        auto itNode = std::find(rptrParent->m_lstNodes.begin(), rptrParent->m_lstNodes.end(), shared_from_this());
        if (itNode == rptrParent->m_lstNodes.end())
            rptrParent->m_lstNodes.push_back(shared_from_this());
    }

    std::shared_ptr<CNodeCollection> CNode::GetParentPtr() const
    {
        return m_ptrParent.lock();
    }

    std::string CNode::GetParentPath() const
    {
        std::shared_ptr<CNodeCollection> ptrParent = m_ptrParent.lock();
        if (!ptrParent) return {};
        return ptrParent->GetPath(false);
    }

    const CCodeSnippet& CNode::CodeSnippet(size_t nIndex, const std::string& rssKey /*= std::string()*/) const
    {
        static CCodeSnippet sEmptyCodeSnippet;
        sEmptyCodeSnippet.RemoveFormat();
        if (nIndex >= m_vecCodeSnippets.size()) return sEmptyCodeSnippet;
        auto itKey = m_vecCodeSnippets[nIndex].find(rssKey);
        if (itKey == m_vecCodeSnippets[nIndex].end()) return sEmptyCodeSnippet;
        return itKey->second;
    }

    CCodeSnippet& CNode::CodeSnippet(size_t nIndex, const std::string& rssKey /*= std::string()*/)
    {
        if (nIndex >= m_vecCodeSnippets.size()) m_vecCodeSnippets.resize(nIndex + 1);
        return m_vecCodeSnippets[nIndex][rssKey];
    }

    const CCodeSnippet& CNode::CodeSnippet(sdv::toml::INodeInfo::ECommentType eType) const
    {
        static CCodeSnippet sEmptyCodeSnippet;
        sEmptyCodeSnippet.RemoveFormat();
        switch (eType)
        {
        case sdv::toml::INodeInfo::ECommentType::comment_before:
            return CodeSnippet(0);
        case sdv::toml::INodeInfo::ECommentType::comment_behind:
            return CodeSnippet(1);
        case sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before:
            return CodeSnippet(2);
        case sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind:
            return CodeSnippet(3);
        default:
            sEmptyCodeSnippet = CCodeSnippet();
            return sEmptyCodeSnippet;
        }
    }

    CCodeSnippet& CNode::CodeSnippet(sdv::toml::INodeInfo::ECommentType eType)
    {
        static CCodeSnippet sEmptyCodeSnippet;
        switch (eType)
        {
        case sdv::toml::INodeInfo::ECommentType::comment_before:
            return CodeSnippet(0);
        case sdv::toml::INodeInfo::ECommentType::comment_behind:
            return CodeSnippet(1);
        case sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before:
            return CodeSnippet(2);
        case sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind:
            return CodeSnippet(3);
        default:
            sEmptyCodeSnippet = CCodeSnippet();
            return sEmptyCodeSnippet;
        }
    }

    std::string CNode::GetCustomPath(const std::string& rssPrefixKey, const std::string& rssContext) const
    {
        // Example of path transformation with a table:
        //
        // [] (suppressed)                     ""              ""
        // [fruit] (suppressed)                ""              ""
        // [fruit.apple]                       ""              ""
        // color = "red"                       ""              "fruit.apple"
        // [fruit.apple.taste]                 ""              ""
        // sweet = true                        ""              "fruit.apple.taste"
        //
        // transfer with all
        // [transfer]  (suppressed)            "transfer"      ""
        // [transfer.fruit] (suppressed)       "transfer"      "transfer"
        // [transfer.fruit.apple]              "transfer"      "transfer"
        // color = "red"                       "transfer"      "transfer.fruit.apple"
        // [transfer.fruit.apple.taste]        "transfer"      "transfer"
        // sweet = true                        "transfer"      "transfer.fruit.apple.taste"
        //
        // transfer with apple
        // [transfer] (suppressed)             "transfer"      "fruit"
        // [transfer.apple]                    "transfer"      "transfer.fruit"
        // color = "red"                       "transfer"      "transfer.fruit.apple"
        // [transfer.apple.taste]              "transfer"      "transfer.fruit"
        // sweet = true                        "transfer"      "transfer.fruit.apple.taste"

        // Insert the prefix before the key path.
        auto lstKeyPath  = GetRawPath(false);
        auto prPrefixKey = SplitNodeKey(rssPrefixKey);
        auto itPos       = lstKeyPath.begin();
        while (!prPrefixKey.first.empty())
        {
            lstKeyPath.insert(itPos, std::make_pair(prPrefixKey.first, prPrefixKey.first));
            prPrefixKey = SplitNodeKey(prPrefixKey.second);
        }

        // Build relative node path by removing all the parent nodes that are equal to the parent nodes of the context.
        std::string ssContextPath = rssContext;
        while (!ssContextPath.empty() && !lstKeyPath.empty())
        {
            // In case of a table array, the table doesn't have a name. Skip the table in the key composition.
            if (lstKeyPath.begin()->second.empty())
            {
                lstKeyPath.pop_front();
                continue;
            }

            auto prContextPath = SplitNodeKey(ssContextPath);
            if (lstKeyPath.begin()->first != prContextPath.first)
                break;
            lstKeyPath.pop_front();
            ssContextPath = prContextPath.second;
        }

        // Build a custom key path by including all raw keys including their code snippet. Skip the initial pre-key code snippet,
        // though.
        std::string ssCustomKeyPath;
        bool bLastSpace = false;
        for (const auto& prKey : lstKeyPath)
        {
            // In case of a table array, the table doesn't have a name. Skip the table in the key composition.
            if (prKey.second.empty())
                continue;

            // Add a dot if this is not the first key.
            if (!ssCustomKeyPath.empty())
                ssCustomKeyPath += ".";

            // Add the pre-key cpde snippet.
            ssCustomKeyPath += CodeSnippet(m_nPreKeyCode, prKey.first).Compose(CCodeSnippet::EComposeMode::compose_inline, CGenContext());

            // Add the raw key string
            ssCustomKeyPath += prKey.second;

            // Add the pos-key cpde snippet.
            ssCustomKeyPath += CodeSnippet(m_nPostKeyCode, prKey.first).Compose(CCodeSnippet::EComposeMode::compose_inline, CGenContext());
            bLastSpace = CodeSnippet(m_nPostKeyCode, prKey.first).HasCode();
        }
        if (!bLastSpace && Inline())
            ssCustomKeyPath += " ";
        return ssCustomKeyPath;
    }

    void CNode::ReassignParser(CParser& rParser)
    {
        m_refParser = rParser;
    }

    bool CNode::ExplicitlyDefined() const
    {
        // Default implementation is explicitly.
        return true;
    }

    void CNode::MakeExplicit()
    {
        // Default implementation is explicitly. Therefore, nothing to do.
    }

    CValueNode::CValueNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName,
        const std::string& rssRawValue) :
        CNode(rparser, rssName, rssRawName), m_ssRawValue(rssRawValue)
    {}

    bool CValueNode::Inline() const
    {
        // Default implementation is always inline.
        return true;
    }

    bool CValueNode::Inline(bool bInline, bool /*bIncludeChildren*/ /*= true*/)
    {
        return bInline; // When set to inline, okay; otherwise not.
    }

    std::shared_ptr<CNode> CValueNode::Direct(const std::string& /*rssPath*/) const
    {
        // The CNodeValue implementation doesn't have any children. Therefore there is nothing to get.
        return std::shared_ptr<CNode>();
    }

    std::string CValueNode::GenerateTOML(const CGenContext& rContext /*= CGenContext()*/) const
    {
        if (IsDeleted()) return {};

        // Create a writable copy of the context and provide this node as potential top node
        CGenContext contextCopy = rContext;
        contextCopy.InitTopMostNode(shared_from_this());

        std::stringstream sstream;

        // Add unconnected pre node comments
        sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before).Compose(
            CCodeSnippet::EComposeMode::compose_standalone_before, contextCopy);

        // Add pre node comments
        sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_before).Compose(
            CCodeSnippet::EComposeMode::compose_before, contextCopy);

        // Add assignment
        if (contextCopy.Assignment()) // Not an array entry
        {
            sstream << contextCopy.RelKeyPath() << "=";
            if (!CodeSnippet(m_nPreValueCode).HasCode())sstream << " ";
        }

        // Stream the value
        sstream << CodeSnippet(m_nPreValueCode).Compose(CCodeSnippet::EComposeMode::compose_inline, contextCopy) <<
            RawValueText() << CodeSnippet(m_nPostValueCode).Compose(CCodeSnippet::EComposeMode::compose_inline, contextCopy);

        // Add post node comments
        sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_behind).Compose(
            CCodeSnippet::EComposeMode::compose_behind, contextCopy);

        // Add unconnected post node comments
        sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind).Compose(
            CCodeSnippet::EComposeMode::compose_standalone_behind, contextCopy);

        return sstream.str();
    }

    void CValueNode::UpdateNodeCode(const CNodeTokenRange& rNodeRange)
    {
        // Update the comments before and behind the code first (this is done in the CNode implementation of the UpdateNodeCode
        // function).
        CNode::UpdateNodeCode(rNodeRange);

        // Process the nodes
        std::list<CToken> lstWhitespace;
        enum class EState
        {
            key_or_value,               // Initial value, needing a key or a value
            assignment_or_separator,    // Assignment or key separator or whitespace before
            key,                        // Key or whitespace before
            value,                      // Value or whitespace before
            post_value                  // Nothing more
        } eState = EState::key_or_value;
        std::string ssKeyName;
        for (std::reference_wrapper<const CToken> refToken = rNodeRange.NodeMain().Begin();
            refToken.get() != rNodeRange.NodeMain().End(); refToken = refToken.get().Next())
        {
            switch (refToken.get().Category())
            {
            case ETokenCategory::token_whitespace:
            case ETokenCategory::token_syntax_new_line:
            case ETokenCategory::token_comment:
                if (eState != EState::assignment_or_separator && eState != EState::key && eState != EState::value &&
                    eState != EState::post_value)
                    return; // Unexpected
                lstWhitespace.push_back(refToken.get());
                break;
            case ETokenCategory::token_key:
                if (eState != EState::key_or_value && eState != EState::key)
                    return; // Unexpected
                eState = EState::assignment_or_separator;
                ssKeyName = refToken.get().StringValue();
                CodeSnippet(m_nPreKeyCode, ssKeyName).SetTokenList(std::move(lstWhitespace));
                break;
            case ETokenCategory::token_boolean:
            case ETokenCategory::token_integer:
            case ETokenCategory::token_float:
            case ETokenCategory::token_string:
            case ETokenCategory::token_date_local:
            case ETokenCategory::token_time_local:
            case ETokenCategory::token_date_time_local:
            case ETokenCategory::token_date_time_offset:
                if (eState != EState::key_or_value && eState != EState::value)
                    return; // Unexpected
                eState = EState::post_value;
                CodeSnippet(m_nPreValueCode).SetTokenList(std::move(lstWhitespace));
                break;
            case ETokenCategory::token_syntax_assignment:
                if (eState != EState::assignment_or_separator)
                    return; // Unexpected
                eState = EState::value;
                CodeSnippet(m_nPostKeyCode, ssKeyName).SetTokenList(std::move(lstWhitespace));
                break;
            case ETokenCategory::token_syntax_dot:
                if (eState != EState::assignment_or_separator)
                    return; // Unexpected
                eState = EState::key;
                CodeSnippet(m_nPostKeyCode, ssKeyName).SetTokenList(std::move(lstWhitespace));
                break;
            default:
                return; // Unexpected
            }
        }
        if (eState != EState::post_value)
            return; // Unexpected
        
        CodeSnippet(m_nPostValueCode).SetTokenList(std::move(lstWhitespace));
    }

    std::string CValueNode::RawValueText() const
    {
        return m_ssRawValue.empty() ? ValueText() : m_ssRawValue;
    }

    void CValueNode::ResetRawValueText()
    {
        m_ssRawValue.clear();
    }

    CBooleanNode::CBooleanNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName, bool bVal,
        const std::string& rssRawValue) :
        CValueNode(rparser, rssName, rssRawName, rssRawValue), m_bVal(bVal)
    {}

    sdv::toml::ENodeType CBooleanNode::GetType() const
    {
        return IsDeleted() ? sdv::toml::ENodeType::node_invalid : sdv::toml::ENodeType::node_boolean;
    }

    sdv::any_t CBooleanNode::GetValue() const
    {
        return m_bVal;
    }

    bool CBooleanNode::ChangeValue(sdv::any_t anyNewValue)
    {
        m_bVal = anyNewValue.get<bool>();
        ResetRawValueText();
        return true;
    }

    std::string CBooleanNode::ValueText() const
    {
        return m_bVal ? "true" : "false";
    }

    CIntegerNode::CIntegerNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName, int64_t iVal,
        const std::string& rssRawValue) :
        CValueNode(rparser, rssName, rssRawName, rssRawValue), m_iVal(iVal)
    {}

    sdv::toml::ENodeType CIntegerNode::GetType() const
    {
        return IsDeleted() ? sdv::toml::ENodeType::node_invalid : sdv::toml::ENodeType::node_integer;
    }

    sdv::any_t CIntegerNode::GetValue() const
    {
        return m_iVal;
    }

    bool CIntegerNode::ChangeValue(sdv::any_t anyNewValue)
    {
        m_iVal = anyNewValue.get<int64_t>();
        ResetRawValueText();
        return true;
    }

    std::string CIntegerNode::ValueText() const
    {
        return std::to_string(m_iVal);
    }

    CFloatingPointNode::CFloatingPointNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName, double dVal,
        const std::string& rssRawValue) :
        CValueNode(rparser, rssName, rssRawName, rssRawValue), m_dVal(dVal)
    {}

    sdv::toml::ENodeType CFloatingPointNode::GetType() const
    {
        return IsDeleted() ? sdv::toml::ENodeType::node_invalid : sdv::toml::ENodeType::node_floating_point;
    }

    sdv::any_t CFloatingPointNode::GetValue() const
    {
        return m_dVal;
    }

    bool CFloatingPointNode::ChangeValue(sdv::any_t anyNewValue)
    {
        m_dVal = anyNewValue.get<double>();
        ResetRawValueText();
        return true;
    }

    std::string CFloatingPointNode::ValueText() const
    {
        std::stringstream sstream;
        sstream << std::setprecision(15) << std::defaultfloat << m_dVal;
        return sstream.str();
    }

    CStringNode::CStringNode(CParser& rparser, const std::string& rssName, const std::string& rssRawName, const std::string& rssVal,
        EQuotationType eQuotationType, const std::string& rssRawValue) :
        CValueNode(rparser, rssName, rssRawName, rssRawValue), m_ssVal(rssVal), m_eQuotationType(eQuotationType)
    {}

    sdv::toml::ENodeType CStringNode::GetType() const
    {
        return IsDeleted() ? sdv::toml::ENodeType::node_invalid : sdv::toml::ENodeType::node_string;
    }

    sdv::any_t CStringNode::GetValue() const
    {
        return m_ssVal;
    }

    bool CStringNode::ChangeValue(sdv::any_t anyNewValue)
    {
        m_ssVal = anyNewValue.get<sdv::u8string>();
        ResetRawValueText();
        return true;
    }

    std::string CStringNode::ValueText() const
    {
        switch (m_eQuotationType)
        {
        case EQuotationType::literal_string:
            return QuoteText(m_ssVal, EQuoteRequest::literal_text);
            break;
        case EQuotationType::multi_line_quoted:
            return QuoteText(m_ssVal, EQuoteRequest::multi_line_quoted_text);
            break;
        case EQuotationType::multi_line_literal:
            return QuoteText(m_ssVal, EQuoteRequest::multi_line_literal_text);
            break;
        case EQuotationType::quoted_string:
        default:
            return QuoteText(m_ssVal, EQuoteRequest::quoted_text);
            break;
        }
    }

    CNodeCollection::CNodeCollection(CParser& rparser, const std::string& rssName, const std::string& rssRawName) :
        CNode(rparser, rssName, rssRawName)
    {}

    void CNodeCollection::AutomaticFormat(/*in*/ bool bRemoveComments)
    {
        // Format each child-node
        for (std::shared_ptr<CNode>& rptrNode : m_lstNodes)
            rptrNode->AutomaticFormat(bRemoveComments);
        CNode::AutomaticFormat(bRemoveComments);
    }

    uint32_t CNodeCollection::GetCount() const
    {
        return static_cast<uint32_t>(m_lstNodes.size());
    }

    sdv::IInterfaceAccess* CNodeCollection::GetNode(/*in*/ uint32_t uiIndex) const
    {
        auto ptrNode = Get(uiIndex);
        return static_cast<sdv::IInterfaceAccess*>(ptrNode.get());
    }

    std::shared_ptr<CNode> CNodeCollection::Get(uint32_t uiIndex) const
    {
        if (static_cast<size_t>(uiIndex) >= m_lstNodes.size())
            return nullptr;

        auto it = m_lstNodes.begin();
        std::advance(it, uiIndex);
        return *it;
    }

    void CNodeCollection::RebuildNodeOrder(bool bForce)
    {
        // Is rebuild locked? Then do not rebuild.
        if (!bForce && Parser().RebuildLocked()) return;

        // Sort the nodes
        m_lstNodes.sort([&](const std::shared_ptr<CNode>& rptrNode1, const std::shared_ptr<CNode>& rptrNode2) -> bool
            {
                if (!rptrNode2) return true;
                if (!rptrNode1) return false;

                // Check whether only one is inline. If so, node 1 is smaller when node 1 is inline.
                if (rptrNode1->Inline() != rptrNode2->Inline())
                    return rptrNode1->Inline();

                // Check for the smallest index
                return *rptrNode1 < *rptrNode2;
            });

        // Call rebuild for all sub-node-collections
        for (auto& rptrNode : m_lstNodes)
        {
            std::shared_ptr<CNodeCollection> ptrCollection = rptrNode->Cast<CNodeCollection>();
            if (ptrCollection)
                ptrCollection->RebuildNodeOrder(bForce);
        }
    }

    void CNodeCollection::FillNodeOrderVector(std::vector<std::shared_ptr<CNode>>& rvecNodes, bool bRootView /*= true*/,
        bool bTopLevel /*= true*/)
    {
        // Run through the nodes and add the nodes that are explicit.
        for (const auto& rptrNode : m_lstNodes)
        {
            // If top level is enabled, count all explicit nodes. If root view is enabled, count all explicit standard tables.
            bool bIsTable = rptrNode->Cast<CTable>() ? true : false;
            bool bIsTableArray = rptrNode->Cast<CArray>() && rptrNode->Cast<CArray>()->TableArray();
            bool bExplicit = rptrNode->ExplicitlyDefined();
            bool bIsInline = rptrNode->Inline();
            if (bExplicit && (bTopLevel || (bRootView && bIsTable && !bIsInline)) && !(bIsTableArray && !bIsInline))
                rvecNodes.push_back(rptrNode);

            // Add children if the node is not explicitly defined (then all children if the top level flag is set) or root view
            // is set and the node is not inline, all the tables within the collection.
            auto ptrCollection = rptrNode->Cast<CNodeCollection>();
            std::shared_ptr<CNode> ptrTargetNode;
            if (ptrCollection && ((bRootView && !ptrCollection->Inline()) || !rptrNode->ExplicitlyDefined()))
                ptrCollection->FillNodeOrderVector(rvecNodes, bRootView, bTopLevel && !rptrNode->ExplicitlyDefined());
        }
    }

    std::shared_ptr<CNode> CNodeCollection::Direct(const std::string& rssPath) const
    {
        auto prKey = SplitNodeKey(rssPath);
        std::shared_ptr<CNode> ptrNode;
        for (const auto& rptrNode : m_lstNodes)
        {
            if (!rptrNode) continue;
            if (rptrNode->GetName() == prKey.first)
            {
                ptrNode = rptrNode;
                break;
            }
        }
        if (!ptrNode) return ptrNode; // Not found

        // Done?
        if (prKey.second.empty()) return ptrNode;

        // There is more...
        return ptrNode->Direct(prKey.second);
    }

    sdv::IInterfaceAccess* CNodeCollection::GetNodeDirect(/*in*/ const sdv::u8string& ssPath) const
    {
        auto ptrNode = Direct(ssPath);
        return static_cast<sdv::IInterfaceAccess*>(ptrNode.get());
    }

    std::pair<std::shared_ptr<CNodeCollection>, std::string> CNodeCollection::SmartParentCreate(const std::string& rssPath,
        bool bInsertTableArray /*= false*/)
    {
        auto prKey = SplitNodeKey(rssPath);
        if (prKey.first.empty()) return std::make_pair(Cast<CNodeCollection>(), rssPath);
        if (prKey.second.empty())
        {
            // Check to see if the key exists already. If so, this could be an array. If not, return.
            // Special treatment for table arrays needed.
            auto ptrNode = Direct(prKey.first);
            if (ptrNode && ptrNode->GetType() == sdv::toml::ENodeType::node_array &&
                (!bInsertTableArray || !ptrNode->Cast<CArray>()->TableArray()))
                return std::make_pair(ptrNode->Cast<CNodeCollection>(), "");
            else
                return std::make_pair(Cast<CNodeCollection>(), rssPath);
        }

        // Determine whether the node should be a table or an array. For this take the next name part and check for an index.
        auto prKeyNext = SplitNodeKey(prKey.second);
        sdv::toml::ENodeType eTargetType =
            (!prKeyNext.first.empty() && prKeyNext.first.find_first_not_of("0123456789") != std::string::npos) ?
            sdv::toml::ENodeType::node_table : sdv::toml::ENodeType::node_array;

        // Find the node
        std::shared_ptr<CNodeCollection> ptrNodeCollection;
        for (auto& rptrNode : m_lstNodes)
        {
            if (!rptrNode) continue;
            if (rptrNode->GetName() == prKey.first)
            {
                ptrNodeCollection = rptrNode->Cast<CNodeCollection>();
                if (rptrNode->GetType() != eTargetType)
                {
                    // Special case... the type is an array, but no index was supplied. Use the index of the last element or if none
                    // exists, the largest index number to indicate a new element.
                    if (rptrNode->GetType() == sdv::toml::ENodeType::node_array)
                        return SmartParentCreate(
                            prKey.first + "[" + std::to_string(ptrNodeCollection->GetCount() - 1) + "]." + prKey.second,
                            bInsertTableArray);

                    // Incorrect type
                    return std::make_pair(nullptr, rssPath);
                }
                break;
            }
        }

        // If not found, create the node collection
        if (!ptrNodeCollection)
        {
            // Determine whether the next key part is a number - then create an array or whether the next key part  is a name, then
            // create a table.
            if (eTargetType == sdv::toml::ENodeType::node_array)
            {
                auto* pArray = dynamic_cast<CNodeCollection*>(InsertArray("", prKey.first));
                if (pArray) ptrNodeCollection = pArray->Cast<CNodeCollection>();
            }
            else
            {
                auto* pTable = dynamic_cast<CNodeCollection*>(InsertTable("", prKey.first,
                    sdv::toml::EInsertPreference::prefer_standard));
                if (pTable) ptrNodeCollection = pTable->Cast<CNodeCollection>();
            }
            if (!ptrNodeCollection) return std::make_pair(nullptr, rssPath); // Not found
        }

        // Get or create next node
        return ptrNodeCollection->SmartParentCreate(prKey.second, bInsertTableArray);
    }

    sdv::IInterfaceAccess* CNodeCollection::InsertValue(/*in*/ const sdv::u8string& ssInsertBefore,
        /*in*/ const sdv::u8string& ssName, /*in*/ sdv::any_t anyValue)
    {
        // Get the parent to inser this node into
        auto prParent = SmartParentCreate(ssName);
        if (!prParent.first) return nullptr;

        // Determine the node to insert before and make sure that the node is not occurring before the parent.
        std::shared_ptr<CNode> ptrInsertBefore = Direct(ssInsertBefore);
        if (ptrInsertBefore && (ptrInsertBefore == prParent.first || ptrInsertBefore < prParent.first))
            ptrInsertBefore = prParent.first->Get(0);

        // Create TOML text
        std::stringstream sstreamTOML;
        if (prParent.first->GetType() != sdv::toml::ENodeType::node_array)
            sstreamTOML << QuoteText(prParent.second, EQuoteRequest::smart_key) << " = ";
        switch (anyValue.eValType)
        {
        case sdv::any_t::EValType::val_type_bool:
            sstreamTOML << (anyValue.get<bool>() ? "true" : "false"); 
            break;
        case sdv::any_t::EValType::val_type_int8:
        case sdv::any_t::EValType::val_type_uint8:
        case sdv::any_t::EValType::val_type_int16:
        case sdv::any_t::EValType::val_type_uint16:
        case sdv::any_t::EValType::val_type_int32:
        case sdv::any_t::EValType::val_type_uint32:
        case sdv::any_t::EValType::val_type_int64:
        case sdv::any_t::EValType::val_type_uint64:
            sstreamTOML << std::to_string(anyValue.get<int64_t>());
            break;
        case sdv::any_t::EValType::val_type_float:
        case sdv::any_t::EValType::val_type_double:
        case sdv::any_t::EValType::val_type_long_double:
        case sdv::any_t::EValType::val_type_fixed:
            sstreamTOML << std::setprecision(15) << std::defaultfloat << anyValue.get<double>();
            break;
        case sdv::any_t::EValType::val_type_string:
        case sdv::any_t::EValType::val_type_u8string:
        case sdv::any_t::EValType::val_type_u16string:
        case sdv::any_t::EValType::val_type_u32string:
        case sdv::any_t::EValType::val_type_wstring:
            sstreamTOML << QuoteText(anyValue.get<sdv::u8string>());
            break;
        case sdv::any_t::EValType::val_type_empty:
        case sdv::any_t::EValType::val_type_char:
        case sdv::any_t::EValType::val_type_char16:
        case sdv::any_t::EValType::val_type_char32:
        case sdv::any_t::EValType::val_type_wchar:
        case sdv::any_t::EValType::val_type_interface:
        case sdv::any_t::EValType::val_type_interface_id:
        case sdv::any_t::EValType::val_type_exception_id:
        default:
            return nullptr; // Not supported
        }

        // Insert the node
        auto prResult = prParent.first->InsertTOML(ptrInsertBefore, sstreamTOML.str(), true);
        if (prResult.first != EInsertResult::insert_success || prResult.second.empty())
            return nullptr;

        // Return the inserted node.
        return prResult.second.back().get();
    }

    sdv::IInterfaceAccess* CNodeCollection::InsertArray(/*in*/ const sdv::u8string& ssInsertBefore,
        /*in*/ const sdv::u8string& ssName)
    {
        // Get the parent to inser this node into
        auto prParent = SmartParentCreate(ssName);
        if (!prParent.first) return nullptr;

        // Determine the node to insert before and make sure that the node is not occurring before the parent.
        std::shared_ptr<CNode> ptrInsertBefore = Direct(ssInsertBefore);
        if (ptrInsertBefore && (ptrInsertBefore == prParent.first || ptrInsertBefore < prParent.first))
            ptrInsertBefore = prParent.first->Get(0);

        // Insert the array node
        std::string ssTOML;
        if (prParent.first->GetType() == sdv::toml::ENodeType::node_array)
            ssTOML = "[]";
        else
            ssTOML = QuoteText(prParent.second, EQuoteRequest::smart_key) + " = []";
        auto prResult = prParent.first->InsertTOML(ptrInsertBefore, ssTOML, true);
        if (prResult.first != EInsertResult::insert_success || prResult.second.empty())
            return nullptr;

        // Return the inserted node.
        return prResult.second.back().get();
    }

    sdv::IInterfaceAccess* CNodeCollection::InsertTable(/*in*/ const sdv::u8string& ssInsertBefore,
        /*in*/ const sdv::u8string& ssName, /*in*/ sdv::toml::EInsertPreference ePreference)
    {
        // Get the parent to inser this node into
        auto ptrArray = Cast<CArray>();
        auto prParent = SmartParentCreate(ssName, ptrArray && ptrArray->TableArray());
        if (!prParent.first) return nullptr;

        // Determine the node to insert before and make sure that the node is not occurring before the parent.
        std::shared_ptr<CNode> ptrInsertBefore = Direct(ssInsertBefore);
        if (ptrInsertBefore && (ptrInsertBefore == prParent.first || ptrInsertBefore < prParent.first))
            ptrInsertBefore = prParent.first->Get(0);

        // Insert inline or standard table
        std::string ssTOML;
        if (prParent.first->Inline() || ePreference == sdv::toml::EInsertPreference::prefer_inline)
        {
            if (prParent.first->GetType() == sdv::toml::ENodeType::node_array)
                ssTOML = "{}";
            else
                ssTOML = QuoteText(prParent.second, EQuoteRequest::smart_key) + " = {}";
        }
        else if (prParent.first->GetType() == sdv::toml::ENodeType::node_array) // Node is a table array
        {
            ssTOML = "[[" + QuoteText(GetName(), EQuoteRequest::smart_key) + "]]";
        }
        else
            ssTOML = "[" + QuoteText(prParent.second, EQuoteRequest::smart_key) + "]";

        // Insert the table node
        auto prResult = prParent.first->InsertTOML(ptrInsertBefore, ssTOML, true);
        if (prResult.first != EInsertResult::insert_success || prResult.second.empty())
            return nullptr;

        // Return the inserted node.
        return prResult.second.back().get();
    }

    sdv::IInterfaceAccess* CNodeCollection::InsertTableArray(/*in*/ const sdv::u8string& ssInsertBefore,
        /*in*/ const sdv::u8string& ssName, /*in*/ sdv::toml::EInsertPreference ePreference)
    {
        // Get the parent to insert this node into
        auto prParent = SmartParentCreate(ssName, true);
        if (!prParent.first) return nullptr;

        // Determine the node to insert before and make sure that the node is not occurring before the parent.
        std::shared_ptr<CNode> ptrInsertBefore = Direct(ssInsertBefore);
        if (ptrInsertBefore && (ptrInsertBefore == prParent.first || ptrInsertBefore < prParent.first))
            ptrInsertBefore = prParent.first->Get(0);

        // Insert inline or standard table array
        std::string ssTOML;
        if (prParent.first->Inline() || ePreference == sdv::toml::EInsertPreference::prefer_inline)
        {
            if (prParent.first->GetType() == sdv::toml::ENodeType::node_array)
                ssTOML = "[{}]";
            else
            {
                // If there is a table array with the same name already, the node will be added to the end.
                auto ptrExistingNode = Direct(prParent.second);
                auto ptrExistingArray = ptrExistingNode ? ptrExistingNode->Cast<CArray>() : std::shared_ptr<const CArray>();
                if (ptrExistingArray && !ptrExistingArray->TableArray()) return nullptr;
                ssTOML = QuoteText(prParent.second, EQuoteRequest::smart_key) + " = [{}]";
            }
        }
        else
        {
            // If there are table arrays, the node is added to the table array, but the order is determined by this node. Count the
            // amount of nodes before the insertion point.
            ssTOML = "[[" + QuoteText(prParent.second, EQuoteRequest::smart_key) + "]]";
        }

        // Insert the table node
        auto prResult = prParent.first->InsertTOML(ptrInsertBefore, ssTOML, true);
        if (prResult.first != EInsertResult::insert_success || prResult.second.empty())
            return nullptr;

        // Return the inserted node.
        return prResult.second.back().get();
    }

    sdv::toml::INodeCollectionInsert::EInsertResult CNodeCollection::InsertTOML(/*in*/ const sdv::u8string& ssInsertBefore,
        /*in*/ const sdv::u8string& ssTOML, /*in*/ bool bRollbackOnPartly)
    {
        std::shared_ptr<CNode> ptrInsertBefore = Direct(ssInsertBefore);

        return InsertTOML(ptrInsertBefore, ssTOML, bRollbackOnPartly).first;
    }

    std::pair<sdv::toml::INodeCollectionInsert::EInsertResult, std::vector<std::shared_ptr<CNode>>>
        CNodeCollection::InsertTOML(const std::shared_ptr<CNode>& rptrInsertBefore,const sdv::u8string& ssTOML,
            bool bRollbackOnPartly)
    {
        std::vector<std::shared_ptr<CNode>> vecRetNodes;
        try
        {
            // Lock the rebuild of the node order
            auto lock = Parser().CreateRebuildLockObject();

            // Parser for the new TOML code
            CParser parser;

            // In case the target collection is an inline array, add additional code to allow insertion to take place between
            // brackets.
            std::shared_ptr<const CNodeCollection> ptrCollection;
            if (Cast<CArray>() && Inline())
            {
                std::string ssTOMLArray = "DummyArray = [" + ssTOML;
                size_t nLastCommentPos = ssTOML.rfind('#');
                if (nLastCommentPos != std::string::npos)
                {
                    // Is there a newline following the comment? If not, add a newline.
                    size_t nLastNewline = ssTOML.find('\n', nLastCommentPos);
                    if (nLastNewline == std::string::npos)
                        ssTOMLArray += "\n";
                }
                ssTOMLArray += "]";

                // Parse the specialized TOML
                parser.Process(ssTOMLArray);

                auto ptrArray = parser.Root().Direct("DummyArray");
                if (ptrArray)
                    ptrCollection = ptrArray->Cast<const CNodeCollection>();
                if (!ptrCollection)
                    return std::make_pair(sdv::toml::INodeCollectionInsert::EInsertResult::insert_fail, vecRetNodes);
            }
            else
            {
                // Normal processing
                parser.Process(ssTOML);
                ptrCollection = parser.Root().Cast<const CNodeCollection>();
            }

            // Check for duplicate naming.
            std::vector<std::shared_ptr<CNode>> vecDuplicateNameNodes;
            for (auto ptrPotentialNewNode : ptrCollection->m_lstNodes)
            {
                // When this collection is an inline-collection, all the nodes from the new collection need to be inline as well.
                if (Inline())
                {
                    if (ptrPotentialNewNode->Cast<CNodeCollection>())
                        ptrPotentialNewNode->Cast<CNodeCollection>()->MakeInline();
                }

                if (Cast<CArray>()) continue; // Names in an array have no meaning; skip this phase.
                auto ptrNode = Direct(ptrPotentialNewNode->GetName());
                if (!ptrNode) continue;

                // If the node is a standard table array and this node collection is a standard table array with the same name, this
                // is allowed. In all other situations, a duplicate name is not allowed.
                bool bPotentialTableArray = ptrPotentialNewNode->Cast<CArray>() &&
                    ptrPotentialNewNode->Cast<CArray>()->TableArray();
                bool bExistingTableArray = ptrNode->Cast<CArray>() &&
                    (ptrNode->Cast<CArray>()->TableArray() || !ptrNode->Cast<CArray>()->GetCount());
                if (bPotentialTableArray && bExistingTableArray)
                {
                    // Make standard or inline, dependable on the target
                    if (ptrNode->Inline())
                        ptrPotentialNewNode->Cast<CNodeCollection>()->MakeInline();
                    else
                        ptrPotentialNewNode->Cast<CNodeCollection>()->MakeStandard(true);
                    continue;
                }

                // Node exists already. Add to duplicate name nodes vector (or fail if no all nodes should fit).
                if (bRollbackOnPartly)
                    return std::make_pair(sdv::toml::INodeCollectionInsert::EInsertResult::insert_fail, vecRetNodes);
                vecDuplicateNameNodes.push_back(ptrPotentialNewNode);
            }

            // Delete the nodes that have duplicate names
            for (auto& rptrNode : vecDuplicateNameNodes)
                rptrNode->DeleteNode();

            // Any nodes left to insert?
            if (!ptrCollection->GetCount())
                return std::make_pair(sdv::toml::INodeCollectionInsert::EInsertResult::insert_fail, vecRetNodes);

            // Add the child nodes to the collection (use a copy of the nodes list, since SetParentPtr changes the node list).
            auto lstCopyNodes = ptrCollection->m_lstNodes;
            for (auto ptrNewNode : lstCopyNodes)
            {
                // If the node is a table array, it is allowed that the tables are added to an existing table array.
                auto ptrNewArray = ptrNewNode->Cast<CArray>();
                auto ptrExistingNode = Direct(ptrNewNode->GetName());
                auto ptrExistingArray = ptrExistingNode ? ptrExistingNode->Cast<CArray>() : std::shared_ptr<CArray>();

                // Shift the node to the target list
                if (!ptrExistingArray)
                {
                    // Asign the new parent pointer.
                    if (ptrNewNode->Cast<CNodeCollection>() && Inline())
                        ptrNewNode->Cast<CNodeCollection>()->MakeInline();
                    ptrNewNode->SetParentPtr(Cast<CNodeCollection>());
                    if (rptrInsertBefore)
                        ptrNewNode->NodeIndex().MoveBefore(rptrInsertBefore->NodeIndex());
                    vecRetNodes.push_back(ptrNewNode);
                }
                else
                {
                    // Inserting values from an existing array into an array with the same name (which is not a table array) is not
                    // automatically supported. For this the array content needs to specifically be added to the existing array.
                    if (!ptrExistingArray->TableArray()) // Should not happen
                        return std::make_pair(sdv::toml::INodeCollectionInsert::EInsertResult::insert_fail, vecRetNodes);
                }

                // Set the parent pointer for each table in the table array.
                if (ptrNewArray && ptrNewArray->TableArray())
                {
                    // Insert each table in the already existing table array. Assign the parent pointer and the location in the
                    // parent table array.
                    uint32_t uiNewArrayIndex = 0;
                    while (uiNewArrayIndex < ptrNewArray->GetCount())
                    {
                        auto ptrNewTable = ptrNewArray->Get(uiNewArrayIndex);
                        // Since the new array might have received a new parent (or in any case a new parser reference), assign the
                        // parent once more to also assign the parser reference.
                        // Shift the table to the target array (this removes it from the current array).
                        if (ptrExistingArray)
                            ptrNewTable->SetParentPtr(ptrExistingArray);
                        else
                            ++uiNewArrayIndex;
                        if (rptrInsertBefore)
                            ptrNewTable->NodeIndex().MoveBefore(rptrInsertBefore->NodeIndex());
                        vecRetNodes.push_back(ptrNewTable);
                    }
                }
            }
            
            // Return the result
            return std::make_pair(vecDuplicateNameNodes.empty() ? sdv::toml::INodeCollectionInsert::EInsertResult::insert_success :
                sdv::toml::INodeCollectionInsert::EInsertResult::insert_partly_success, vecRetNodes);
        }
        catch (const sdv::toml::XTOMLParseException& rexcept)
        {
            std::cerr << rexcept.what() << std::endl;
            return std::make_pair(sdv::toml::INodeCollectionInsert::EInsertResult::insert_fail, vecRetNodes);
        }
    }

    bool CNodeCollection::DeleteNode()
    {
        // Delete the children first
        auto lstCopy = m_lstNodes;
        bool bRet = true;
        for (auto& rptrNode : lstCopy)
        {
            if (rptrNode)
                bRet |= rptrNode->DeleteNode();
        }

        // Delete this node
        return bRet && CNode::DeleteNode();
    }

    bool CNodeCollection::Inline() const
    {
        // Will be overridden
        return false;
    }

    bool CNodeCollection::Inline(bool bInline, bool bIncludeChildren /*= true*/)
    {
        // Updating children, but only when inline or when include children flags has been set.
        if (!bInline && !bIncludeChildren) return true;

        // Update the children
        for (auto& rptrNode : m_lstNodes)
        {
            rptrNode->Inline(bInline, bIncludeChildren);
        }

        return true;
    }

    bool CNodeCollection::CanMakeInline() const
    {
        // To make a node inline is always possible.
        return true;
    }

    bool CNodeCollection::MakeInline()
    {
        // Block rebuilding
        auto lock = Parser().CreateRebuildLockObject();

        bool bRet = Inline(true);
        if (bRet) AutomaticFormat(false);
        return bRet;
    }

    bool CNodeCollection::CanMakeStandard() const
    {
        // To make a node as standard node, this is only possible when the parent is not inline.
        auto ptrParent = GetParentPtr();
        if (ptrParent && ptrParent->Inline())
            return false;

        // If this is the root node, check if one of the nodes can be made standard
        if (Cast<CRootTable>())
        {
            bool bRet = false;
            for (const auto& rptrNode : m_lstNodes)
                bRet |= rptrNode->Cast<CNodeCollection>() ? rptrNode->Cast<CNodeCollection>()->CanMakeStandard() : false;
            return bRet;
        }

        return true;
    }

    bool CNodeCollection::MakeStandard(/*in*/ bool bIncludeChildren)
    {
        // Block rebuilding
        auto lock = Parser().CreateRebuildLockObject();

        if (!CanMakeStandard())
            return false;
        bool bRet = Inline(false, bIncludeChildren);
        if (bRet) AutomaticFormat(false);
        return bRet;
    }

    bool CNodeCollection::DeleteNode(const std::shared_ptr<CNode>& rptrNode)
    {
        // Find the element
        auto itElementLst = std::find(m_lstNodes.begin(), m_lstNodes.end(), rptrNode);
        if (itElementLst == m_lstNodes.end())
            return false;

        // Shift the node into the recycle bin.
        m_lstRecycleBin.push_back(rptrNode);
        m_lstNodes.erase(itElementLst);
        return true;
    }

    uint32_t CNodeCollection::FindIndex(const std::shared_ptr<CNode>& rptrNode) const
    {
        // Find the element
        for (uint32_t uiIndex = 0; uiIndex < GetCount(); uiIndex++)
        {
            if (rptrNode == Get(uiIndex))
                return uiIndex;
        }
        return sdv::toml::npos;
    }

    bool CNodeCollection::IsDescendant(const std::shared_ptr<CNode>& rptrNode) const
    {
        // Check wihin the list of nodes, whether the provided node is a child node and if not, ask all child collection nodes.
        for (auto ptrChild : m_lstNodes)
        {
            if (ptrChild == rptrNode) return true;
            auto ptrCollection = ptrChild->Cast<CNodeCollection>();
            if (ptrCollection && ptrCollection->IsDescendant(rptrNode))
                return true;
        }
        return false;
    }

    void CNodeCollection::ReassignParser(CParser& rParser)
    {
        CNode::ReassignParser(rParser);
        for (auto& rptrNode : m_lstNodes)
            rptrNode->ReassignParser(rParser);
    }

    CTable::CTable(CParser& rparser, const std::string& rssName, const std::string& rssRawName, bool bDefaultInline,
        bool bExplicit /*= true*/) :
        CNodeCollection(rparser, rssName, rssRawName), m_bDefinedExplicitly(bExplicit), m_bInline(bDefaultInline)
    {}

    sdv::toml::ENodeType CTable::GetType() const
    {
        return IsDeleted() ? sdv::toml::ENodeType::node_invalid : sdv::toml::ENodeType::node_table;
    }

    uint32_t CTable::GetCount() const
    {
        return static_cast<uint32_t>(m_vecNodeOrder.size());
    }

    std::shared_ptr<CNode> CTable::Get(uint32_t uiIndex) const
    {
        if (static_cast<size_t>(uiIndex) >= m_vecNodeOrder.size())
            return nullptr;

        return m_vecNodeOrder[uiIndex];
    }

    bool CTable::DeleteNode()
    {
        // Get parent
        auto ptrParent = GetParentPtr();

        // Check for inline
        bool bInline = Inline();

        // Delete the node
        bool bRet = CNodeCollection::DeleteNode();

        // If the parent node is an array and doesn't have any members any more and is not inline; remove the parent as well.
        if (!bInline && ptrParent->GetType() == sdv::toml::ENodeType::node_array && !ptrParent->GetCount())
            ptrParent->DeleteNode();

        return bRet;
    }

    std::string CTable::GenerateTOML(const CGenContext& rContext /*= CGenContext()*/) const
    {
        if (IsDeleted()) return {};

        // Create a writable copy of the context and provide this node as potential top node
        CGenContext contextCopy = rContext;
        contextCopy.InitTopMostNode(shared_from_this());

        std::stringstream sstream;

        auto ptrParent = GetParentPtr();
        bool bRoot = dynamic_cast<const CRootTable*>(this) ? true : false;

        // Impossible option: suppressing the table name (due to root flag) and being embedded.
         if (contextCopy.Embedded() && bRoot)
             return {};

        // Special case, table as part of table array - but only if the parent is included in the generation. This can be identified
        // by the top most flag of the context.
        bool bTableArray = false;
        if (!contextCopy.TopMostNode() || contextCopy.CheckOption(EGenerateOptions::full_header))
            bTableArray =
                !contextCopy.Embedded() && ptrParent && ptrParent->Cast<CArray>() && ptrParent->Cast<CArray>()->TableArray();

        // Do not print explicit tables with only tables inside (except if they have no children at all or if they have one or more
        // inline tables).
        // Do not print table headers when this sis the top most table.
        bool bDoNotPrint = (rContext.TopMostNode() || (!Inline() && GetCount())) ? true : false;
        for (uint32_t uiIndex = 0; bDoNotPrint && uiIndex < GetCount(); uiIndex++)
        {
            std::shared_ptr<CNode> ptrNode = Get(uiIndex);
            if (!ptrNode) continue;
            if (bTableArray || (!ptrNode->Cast<CTable>() && !ptrNode->Cast<CArray>()) || ptrNode->Inline())
                bDoNotPrint = false;
        }

        // Add unconnected pre node comments
        sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before).Compose(CCodeSnippet::EComposeMode::compose_standalone_before, contextCopy);

        // Add pre node comments
        sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_before).Compose(CCodeSnippet::EComposeMode::compose_before, contextCopy);

        // Do we need to start a table?
        if (!bDoNotPrint && !contextCopy.Inline() && !contextCopy.KeyPath().empty())
        {
            if (bTableArray)
                sstream << "[[" << contextCopy.KeyPath() << "]]";
            else
                sstream << "[" << contextCopy.KeyPath() << "]";

            // Add post node comments
            sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_behind)
                           .Compose(CCodeSnippet::EComposeMode::compose_behind, contextCopy);
        }

        // Stream the table - for a standard table first stream the inline nodes, then the standard nodes
        if (contextCopy.Assignment() && contextCopy.Inline() && !bDoNotPrint) // Not an array entry
        {
            //if (!CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_before).HasCode()) sstream << " ";
            sstream << contextCopy.RelKeyPath() << "=";
            if (!CodeSnippet(m_nPreValueCode).HasCode())sstream << " ";
        }
        if ((contextCopy.Inline()) && (!bDoNotPrint || ptrParent->Cast<CArray>())) // Embedded table in an array
            sstream << CodeSnippet(m_nPreValueCode).Compose(CCodeSnippet::EComposeMode::compose_inline, contextCopy) << "{";
        for (uint32_t uiIndex = 0; uiIndex < GetCount(); uiIndex++)
        {
            std::shared_ptr<CNode> ptrNode = Get(uiIndex);
            if (!ptrNode) continue;

            // If the node is part of the view in the context, it is printed by the view node and not here.
            if (!contextCopy.IsPartOfView(ptrNode)) continue;

            if (contextCopy.Inline() || ptrNode->Inline())
            {
                // Inline nodes are only presented with a relative path (in most cases this is no path) for each node.
                // If the parent node is a table array, the do-not-print-flag is active (because this node is a table as part of
                // the array, which is used for data management and not for printing the node key) - use the full context for
                // printing.
                sstream << ptrNode->GenerateTOML(contextCopy.CopyWithContext(
                    (bDoNotPrint && !bTableArray) ? contextCopy.KeyContext() : contextCopy.FullKeyPath(), ptrNode, uiIndex == GetCount() - 1));
            }
            else
            {
                // Explicit collection nodes (tables and table-arrays) are presented with the complete path for each node.
                sstream << ptrNode->GenerateTOML(contextCopy.CopyWithContext(contextCopy.KeyContext(), ptrNode, uiIndex == GetCount() - 1));
            }
        }
        if ((!bDoNotPrint || (ptrParent && ptrParent->Cast<CArray>())) && (contextCopy.Embedded() || contextCopy.Inline())) // Embedded table in an array
        {
            sstream << CodeSnippet(m_nPostValuesArray).Compose(CCodeSnippet::EComposeMode::compose_inline, contextCopy) << "}";

            // Add post node comments
            sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_behind).Compose(CCodeSnippet::EComposeMode::compose_behind, contextCopy);
        }

        // Add unconnected post node comments
        sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind).Compose(CCodeSnippet::EComposeMode::compose_standalone_behind, contextCopy);

        return sstream.str();
    }

    void CTable::UpdateNodeCode(const CNodeTokenRange& rNodeRange)
    {
        // Update the comments before and behind the code first (this is done in the CNode implementation of the UpdateNodeCode
        // function).
        CNode::UpdateNodeCode(rNodeRange);

        // Process the main node tokens until array open
        std::list<CToken> lstWhitespace;
        enum class EState
        {
            key_or_value_or_table_def_open,     // Needing a key or a value or a table definition open-character
            table_def_close_or_separator,       // Needing a separator or a table definition close-character or whitespace before
            assignment_or_separator,            // Assignment or key separator or whitespace before
            key,                                // Key or whitespace before
            table_open,                         // Table open or whitespace before
            table_content_and_close,            // Table content and table close
            done                                // Nothing more
        } eState = EState::key_or_value_or_table_def_open;
        std::string ssKeyName;
        bool bExplicit = false;
        for (std::reference_wrapper<const CToken> refToken = rNodeRange.NodeMain().Begin();
             refToken.get() != rNodeRange.NodeMain().End();
             refToken = refToken.get().Next())
        {
            switch (refToken.get().Category())
            {
            case ETokenCategory::token_whitespace:
            case ETokenCategory::token_syntax_new_line:
            case ETokenCategory::token_comment:
                if (eState != EState::assignment_or_separator && eState != EState::key && eState != EState::table_open && 
                    eState != EState::table_def_close_or_separator && eState != EState::key_or_value_or_table_def_open)
                    return; // Unexpected
                lstWhitespace.push_back(refToken.get());
                break;
            case ETokenCategory::token_key:
                if (eState != EState::key_or_value_or_table_def_open && eState != EState::key)
                    return; // Unexpected
                eState = bExplicit ? EState::table_def_close_or_separator : EState::assignment_or_separator;
                ssKeyName = refToken.get().StringValue();
                CodeSnippet(m_nPreKeyCode, ssKeyName).SetTokenList(std::move(lstWhitespace));
                break;
            case ETokenCategory::token_syntax_table_open:
                if (eState != EState::key_or_value_or_table_def_open)
                    return; // Unexpected
                bExplicit = true;
                eState = EState::key;
                break;
            case ETokenCategory::token_syntax_table_close:
                if (eState != EState::table_def_close_or_separator)
                    return; // Unexpected
                eState = EState::done;
                CodeSnippet(m_nPostKeyCode, ssKeyName).SetTokenList(std::move(lstWhitespace));
                break;
            case ETokenCategory::token_syntax_inline_table_open:
                if (eState != EState::key_or_value_or_table_def_open && eState != EState::table_open)
                    return; // Unexpected
                eState = EState::table_content_and_close;
                CodeSnippet(m_nPreValueCode).SetTokenList(std::move(lstWhitespace));
                break;
            case ETokenCategory::token_syntax_assignment:
                if (eState != EState::assignment_or_separator)
                    return; // Unexpected
                eState = EState::table_open;
                CodeSnippet(m_nPostKeyCode, ssKeyName).SetTokenList(std::move(lstWhitespace));
                break;
            case ETokenCategory::token_syntax_dot:
                if (eState != EState::assignment_or_separator && eState != EState::table_def_close_or_separator)
                    return; // Unexpected
                eState = EState::key;
                CodeSnippet(m_nPostKeyCode, ssKeyName).SetTokenList(std::move(lstWhitespace));
                break;
            default:
                return; // Unexpected
            }
        }

        // Done if this is an explicit table
        if (bExplicit) return;

        // Expecting table content
        if (eState != EState::table_content_and_close)
            return; // Unexpected

        // Process the main node tokens from array close
        lstWhitespace.clear();
        for (std::reference_wrapper<const CToken> refToken = rNodeRange.NodeMainFinish().Begin();
             refToken.get() != rNodeRange.NodeMainFinish().End();
             refToken = refToken.get().Next())
        {
            switch (refToken.get().Category())
            {
            case ETokenCategory::token_whitespace:
            case ETokenCategory::token_syntax_new_line:
            case ETokenCategory::token_comment:
                if (eState != EState::table_content_and_close && eState != EState::done)
                    return; // Unexpected
                lstWhitespace.push_back(refToken.get());
                break;
            case ETokenCategory::token_syntax_inline_table_close:
                if (eState != EState::table_content_and_close)
                    return; // Unexpected
                eState = EState::done;
                break;
            default:
                return; // Unexpected
            }
        }

        // Expecting the table to have been closed
        if (eState != EState::done)
            return;

        CodeSnippet(m_nPostValueCode).SetTokenList(std::move(lstWhitespace));
    }

    bool CTable::Inline() const
    {
        return m_bInline;
    }

    bool CTable::Inline(bool bInline, bool bIncludeChildren /*= true*/)
    {
        // When inline, all children are also inline.
        if (m_bInline && bInline) return true; // Nothing to do

        // When not inline and children are not to be considered, shortcut when possible
        if (!m_bInline && !bInline && !bIncludeChildren) return true; // Nothing to do

        // Allowed to make standard
        if (!Cast<CRootTable>() && !bInline && !CanMakeStandard()) return false; // Nothing to do

        // If a parent if inline, the child needs to be inline as well.
        bool bTargetInline = bInline;
        auto ptrParent = GetParentPtr();
        if (ptrParent && ptrParent->Inline())
            bTargetInline = true;

        // If making inline, make children inline first (call to base class).
        // If making standard, make standard before making children inline.
        if (bTargetInline)
        {
            if (CNodeCollection::Inline(true))
                m_bInline = true;
        } else
        {
            m_bInline = false;
            CNodeCollection::Inline(false, bIncludeChildren);
        }

        return m_bInline == bInline;
    }

    bool CTable::ExplicitlyDefined() const
    {
        return m_bDefinedExplicitly;
    }

    void CTable::MakeExplicit()
    {
        m_bDefinedExplicitly = true;
    }

    bool CTable::Combine(const std::shared_ptr<CNodeCollection>& rptrCollection)
    {
        if (!rptrCollection) return false;  // No collection supplied.
        if (rptrCollection == Cast<CNodeCollection>()) return true; // Collections are identical, nothing to combine.
        if (rptrCollection->IsDescendant(shared_from_this())) return false; // Circular reference.

        // Lock the rebuild of the node order
        auto lock = Parser().CreateRebuildLockObject();
        
        // Run through the provided collection list
        // If nodes don't exist in the current collection, add the nodes
        // If nodes exist in the current collection, but are different, update the nodes

        bool bResult = true;
        CGenContext contextGeneration;
        contextGeneration.SetOption(EGenerateOptions::reduce_whitespace);
        contextGeneration.SetOption(EGenerateOptions::full_header);
        for (uint32_t uiIndex = 0; uiIndex < rptrCollection->GetCount(); uiIndex++)
        {
            // Get the new and potentially existing nodes.
            auto ptrNewNode = rptrCollection->Get(uiIndex);
            if (!ptrNewNode) continue;
            auto ptrExistNode = Direct(ptrNewNode->GetName());

            // Check whether the node is really new; if it is, add the node
            // Exception to the rule... when the existing node is a table array and the node to add is a table, do not do replace
            // the node.
            if (!ptrExistNode || (ptrNewNode->GetType() == sdv::toml::ENodeType::node_table &&
                    ptrExistNode->Cast<CArray>() && ptrExistNode->Cast<CArray>()->TableArray()))
            {
                bResult &= InsertTOML("", ptrNewNode->GenerateTOML(contextGeneration), true) ==
                    sdv::toml::INodeCollectionInsert::EInsertResult::insert_success;
                continue;
            }

            // The node exists. Are the node types identical? If not, replace the existing node with the new node
            if (ptrNewNode->GetType() != ptrExistNode->GetType())
            {
                bool bLocalResult = ptrExistNode->DeleteNode();
                bResult &= bLocalResult;
                if (bLocalResult)
                    bResult &= InsertTOML(ptrExistNode, ptrNewNode->GenerateTOML(contextGeneration), true).first ==
                        sdv::toml::INodeCollectionInsert::EInsertResult::insert_success;
                continue;
            }

            // The nodes have identical type. Are they a collection themselves, then use the combine function of the collection.
            if (ptrExistNode->Cast<CNodeCollection>())
            {
                bResult &= ptrExistNode->Cast<CNodeCollection>()->Combine(ptrNewNode->Cast<CNodeCollection>());
                continue;
            }

            // Both nodes are value nodes of the same type. Check the value to be identical. If not, update the value.
            if (ptrExistNode->GetValue() != ptrNewNode->GetValue())
            {
                bResult &= ptrExistNode->ChangeValue(ptrNewNode->GetValue());
                continue;
            }

            // Nodes are identical; there's nothing to do.
        }

        return bResult;
    }

    bool CTable::Reduce(const std::shared_ptr<CNodeCollection>& rptrCollection)
    {
        if (!rptrCollection) return false;  // No collection supplied.
        if (rptrCollection == Cast<CNodeCollection>()) return false; // Collections are identical, this would empty the collection.
        if (rptrCollection->IsDescendant(shared_from_this())) return false; // Circular reference.
        
        // Lock the rebuild of the node order
        auto lock = Parser().CreateRebuildLockObject();

        // Run through the provided collection list.
        // If nodes don't exist in the current collection, leave the nodes in the collection.
        // If nodes exist in the current collection, but are different, leave the nodes in the collection.
        // If the nodes exist and are identical, delete the nodes from the collection.

        bool bResult = true;
        for (uint32_t uiIndex = 0; uiIndex < rptrCollection->GetCount(); uiIndex++)
        {
            // Get the new and potentially existing nodes.
            auto ptrReductorNode = rptrCollection->Get(uiIndex);
            if (!ptrReductorNode) continue;
            auto ptrExistNode = Direct(ptrReductorNode->GetName());

            // Check whether the node exists; if not, leave the node in the collection.
            if (!ptrExistNode) continue;

            // Deal with table arrays...
            if (ptrReductorNode->GetType() == sdv::toml::ENodeType::node_table && ptrReductorNode->GetParentPtr() &&
                ptrReductorNode->GetParentPtr()->Cast<CArray>() && ptrReductorNode->GetParentPtr()->Cast<CArray>()->TableArray() &&
                ptrExistNode->Cast<CArray>() && ptrExistNode->Cast<CArray>()->TableArray())
            {
                bResult &= ptrExistNode->Cast<CArray>()->Reduce(ptrReductorNode->GetParentPtr()->Cast<CArray>());
                continue;
            }

            // The node exists. Are the node types identical? If not, leave the node in the collection
            if (ptrReductorNode->GetType() != ptrExistNode->GetType()) continue;

            // The nodes have identical type. Are they a collection themselves, then use the reduce function of the collection.
            if (ptrExistNode->Cast<CNodeCollection>())
            {
                bool bLocalResult = ptrExistNode->Cast<CNodeCollection>()->Reduce(ptrReductorNode->Cast<CNodeCollection>());
                bResult &= bLocalResult;

                // if the collection is empty after the reduction; remove the collection completely.
                if (bLocalResult && !ptrExistNode->Cast<CNodeCollection>()->GetCount())
                    bResult &= ptrExistNode->DeleteNode();
                continue;
            }

            // Both nodes are value nodes of the same type. Check the value to be identical. If not, leave the node in the
            // collection.
            if (ptrExistNode->GetValue() != ptrReductorNode->GetValue()) continue;

            // Nodes are identical and therefore should be removed from the collection.
            bResult &= ptrExistNode->DeleteNode();
        }

        return bResult;
    }

    void CTable::RebuildNodeOrder(bool bForce)
    {
        // Is rebuild locked? Then do not rebuild.
        if (!bForce && Parser().RebuildLocked()) return;

        m_vecNodeOrder.clear();
        FillNodeOrderVector(m_vecNodeOrder);

        // Sort the nodes
        std::sort(m_vecNodeOrder.begin(),
            m_vecNodeOrder.end(),
            [&](const std::shared_ptr<CNode>& rptrNode1, const std::shared_ptr<CNode>& rptrNode2) -> bool
            {
                if (!rptrNode2) return true;
                if (!rptrNode1) return false;

                // Check whether only one is inline. If so, node 1 is smaller when node 1 is inline.
                if (rptrNode1->Inline() != rptrNode2->Inline())
                    return rptrNode1->Inline();

                // Check for the smallest index
                return *rptrNode1 < *rptrNode2;
            });

        // Call base class
        CNodeCollection::RebuildNodeOrder(bForce);
    }

    CArray::CArray(CParser& rparser,
        const std::string& rssName,
        const std::string& rssRawName,
        bool bExplicitTableArray /*= false*/) :
        CNodeCollection(rparser, rssName, rssRawName), m_bDefinedExplicitly(bExplicitTableArray), m_bInline(!bExplicitTableArray)
    {}

    sdv::toml::ENodeType CArray::GetType() const
    {
        return IsDeleted() ? sdv::toml::ENodeType::node_invalid : sdv::toml::ENodeType::node_array;
    }

    std::shared_ptr<CNode> CArray::Direct(const std::string& rssPath) const
    {
        // The key might be an empty string.
        auto prKey = SplitNodeKey(rssPath);
        uint32_t uiIndex = GetCount() - 1;
        std::string ssSecond = prKey.second;
        if (prKey.first.find_first_not_of("0123456789)") != std::string::npos)
        {
            // If there is no index, take the latest entry in the array. This is needed to be able to insert into table arrays,
            // which are automatically indexed by occurance.
            if (!GetCount()) return {}; // Unexpected
            ssSecond = rssPath;
        }
        else
        {
            try
            {
                uiIndex = static_cast<uint32_t>(std::stoul(prKey.first));
            } catch (const std::exception&)
            {
                return {};
            }
        }

        // Get the node
        if (uiIndex == sdv::toml::npos && GetCount())
            uiIndex = GetCount() - 1;
        if (uiIndex >= GetCount()) return {}; // Not found
        std::shared_ptr<CNode> ptrNode = Get(uiIndex);
        if (!ptrNode) return {};

        // Done?
        if (ssSecond.empty()) return ptrNode;

        return ptrNode->Direct(ssSecond);
    }

    std::pair<std::shared_ptr<CNodeCollection>, std::string> CArray::SmartParentCreate(const std::string& rssPath,
        bool bInsertTableArray /*= false*/)
    {
        auto prKey = SplitNodeKey(rssPath);

        // Special situation: if the first part of the name is empty or not a number, this is not an error, but should cause the
        // node to be added to the end of the array.
        if (rssPath.empty()) return std::make_pair(Cast<CNodeCollection>(), rssPath);
        if (prKey.first.empty() || prKey.first.find_first_not_of("0123456789") != std::string::npos)
            prKey = std::make_pair(std::to_string(GetCount()), rssPath);
        uint32_t uiArrayIndex = 0;
        std::from_chars(prKey.first.data(), prKey.first.data() + prKey.first.size(), uiArrayIndex);

        // Determine whether the node should be a table or an array. For this take the next name part and check for an index.
        auto prKeyNext = SplitNodeKey(prKey.second);
        sdv::toml::ENodeType eTargetType =
            (!prKeyNext.first.empty() && prKeyNext.first.find_first_not_of("0123456789") != std::string::npos)
                ? sdv::toml::ENodeType::node_table
                : sdv::toml::ENodeType::node_array;

        // Find the node
        std::shared_ptr<CNode> ptrNode = Get(uiArrayIndex);
        std::shared_ptr<CNodeCollection> ptrNodeCollection;
        if (ptrNode)
        {
            ptrNodeCollection = ptrNode->Cast<CNodeCollection>();
            if (ptrNode->GetType() != eTargetType)
            {
                // If the node is an array and there is no index, take the last node in the array.
                if (ptrNode->GetType() == sdv::toml::ENodeType::node_array && ptrNodeCollection->GetCount())
                {
                    ptrNode = ptrNodeCollection->Get(ptrNodeCollection->GetCount() - 1);
                    ptrNodeCollection = ptrNode->Cast<CNodeCollection>();
                } else
                    return std::make_pair(nullptr, rssPath);
            }
        }

        // If not found, create the node collection
        if (!ptrNodeCollection)
        {
            // Determine whether the next key part is a number - then create an array or whether the next key part  is a name, then
            // create a table.
            if (eTargetType == sdv::toml::ENodeType::node_array)
            {
                auto* pArray = dynamic_cast<CNodeCollection*>(InsertArray("[" + std::to_string(uiArrayIndex) + "]", ""));
                if (pArray)
                    ptrNodeCollection = pArray->Cast<CNodeCollection>();
            }
            else
            {
                // If this is an array, do not insert another table. Use the last node collection instead.
                if (!bInsertTableArray && GetCount())
                {
                    ptrNode = Get(GetCount() - 1);
                    if (ptrNode)
                        ptrNodeCollection = ptrNode->Cast<CNodeCollection>();
                }
                else
                {
                    auto* pTable = dynamic_cast<CNodeCollection*>(
                        InsertTable("[" + std::to_string(uiArrayIndex) + "]", "", sdv::toml::EInsertPreference::prefer_standard));
                    if (pTable)
                        ptrNodeCollection = pTable->Cast<CNodeCollection>();
                }
            }
            if (!ptrNodeCollection)
                return std::make_pair(nullptr, rssPath); // Not found
        }

        // Get or create next node
        return ptrNodeCollection->SmartParentCreate(prKey.second, bInsertTableArray);
    }

    std::string CArray::GenerateTOML(const CGenContext& rContext /*= CGenContext()*/) const
    {
        if (IsDeleted()) return {};

        // Create a writable copy of the context and provide this node as potential top node
        CGenContext contextCopy = rContext;
        contextCopy.InitTopMostNode(shared_from_this());

        std::stringstream sstream;

        // Determine whether the statement should be embedded (same line separated by commas) and should have an assignment (not
        // having a key name).
        auto ptrParent = GetParentPtr();
        if (!ptrParent) return {};

        // Stream only for inline
        if (contextCopy.Inline())
        {
            // Add unconnected pre node comments
            sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before).Compose(CCodeSnippet::EComposeMode::compose_standalone_before, contextCopy);

            // Add pre node comments
            sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_before).Compose(CCodeSnippet::EComposeMode::compose_before, contextCopy);

            // Add assignment
            if (contextCopy.Assignment()) // Not an array entry
            {
                //if (!CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_before).HasCode()) sstream << " ";
                sstream << contextCopy.RelKeyPath() << "=";
                if (!CodeSnippet(m_nPreValueCode).HasCode())sstream << " ";
            }

            sstream << CodeSnippet(m_nPreValueCode).Compose(CCodeSnippet::EComposeMode::compose_inline, contextCopy) << "[";
        }

        // Stream the array content
        for (uint32_t uiIndex = 0; uiIndex < GetCount(); uiIndex++)
        {
            std::shared_ptr<CNode> ptrNode = Get(uiIndex);
            if (!ptrNode) continue;

            // If the node is part of the view in the context, it is printed by the view node and not here.
            if (!contextCopy.IsPartOfView(ptrNode))
                continue;

            // Generate the TOML for the array node. Copy the context with the full key path (when inline) or the key context of
            // this array (when table array).
            sstream << ptrNode->GenerateTOML(
                contextCopy.CopyWithContext(contextCopy.Inline() ? contextCopy.FullKeyPath() : contextCopy.KeyContext(),
                    ptrNode, uiIndex == GetCount() - 1));
        }

        // Stream only for inline
        if (contextCopy.Inline())
        {
            sstream << CodeSnippet(m_nPostValuesArray).Compose(CCodeSnippet::EComposeMode::compose_inline, contextCopy) << "]";

            // Add post node comments
            sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::comment_behind).Compose(CCodeSnippet::EComposeMode::compose_behind, contextCopy);

            // Add unconnected post node comments
            sstream << CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind).Compose(CCodeSnippet::EComposeMode::compose_standalone_behind, contextCopy);
        }

        return sstream.str();
    }

    void CArray::UpdateNodeCode(const CNodeTokenRange& rNodeRange)
    {
        // Update the comments before and behind the code first (this is done in the CNode implementation of the UpdateNodeCode
        // function).
        CNode::UpdateNodeCode(rNodeRange);

        // Process the main node tokens until array open
        std::list<CToken> lstWhitespace;
        enum class EState
        {
            key_or_value,               // Initial value, needing a key or a value
            assignment_or_separator,    // Assignment or key separator or whitespace before
            key,                        // Key or whitespace before
            array_open,                 // Array open or whitespace before
            array_content_and_close,    // Array content and whitespace following the content, not belonging to a value
            post_value                  // Nothing more
        } eState = EState::key_or_value;
        std::string ssKeyName;
        for (std::reference_wrapper<const CToken> refToken = rNodeRange.NodeMain().Begin();
            refToken.get() != rNodeRange.NodeMain().End(); refToken = refToken.get().Next())
        {
            switch (refToken.get().Category())
            {
            case ETokenCategory::token_whitespace:
            case ETokenCategory::token_syntax_new_line:
            case ETokenCategory::token_comment:
                if (eState != EState::assignment_or_separator && eState != EState::key && eState != EState::array_open)
                    return; // Unexpected
                lstWhitespace.push_back(refToken.get());
                break;
            case ETokenCategory::token_key:
                if (eState != EState::key_or_value && eState != EState::key)
                    return; // Unexpected
                eState = EState::assignment_or_separator;
                ssKeyName = refToken.get().StringValue();
                CodeSnippet(m_nPreKeyCode, ssKeyName).SetTokenList(std::move(lstWhitespace));
                break;
            case ETokenCategory::token_syntax_array_open:
                if (eState != EState::key_or_value && eState != EState::array_open)
                    return; // Unexpected
                eState = EState::array_content_and_close;
                CodeSnippet(m_nPreValueCode).SetTokenList(std::move(lstWhitespace));
                break;
            case ETokenCategory::token_syntax_assignment:
                if (eState != EState::assignment_or_separator)
                    return; // Unexpected
                eState = EState::array_open;
                CodeSnippet(m_nPostKeyCode, ssKeyName).SetTokenList(std::move(lstWhitespace));
                break;
            case ETokenCategory::token_syntax_dot:
                if (eState != EState::assignment_or_separator)
                    return; // Unexpected
                eState = EState::key;
                CodeSnippet(m_nPostKeyCode, ssKeyName).SetTokenList(std::move(lstWhitespace));
                break;
            default:
                return; // Unexpected
            }
        }

        // Expecting array content 
        if (eState != EState::array_content_and_close)
            return; // Unexpected

        // Check the last node of the array for a potential comma in the code snippet following the node assignment.
        RebuildNodeOrder(true);
        if (GetCount())
        {
            auto ptrNode = Get(GetCount() - 1);
            m_bLastChildNodeWithComma = ptrNode->CodeSnippet(m_nPostNodeCode).HasComma();
        }

        // Process the main node tokens from array close
        lstWhitespace.clear();
        for (std::reference_wrapper<const CToken> refToken = rNodeRange.NodeMainFinish().Begin();
            refToken.get() != rNodeRange.NodeMainFinish().End(); refToken = refToken.get().Next())
        {
            switch (refToken.get().Category())
            {
            case ETokenCategory::token_whitespace:
            case ETokenCategory::token_syntax_new_line:
            case ETokenCategory::token_comment:
                if (eState != EState::array_content_and_close && eState != EState::post_value)
                    return; // Unexpected
                lstWhitespace.push_back(refToken.get());
                break;
            case ETokenCategory::token_syntax_comma:    // A comma is allowed after the last element and belongs to the finishing.
                if (eState != EState::array_content_and_close)
                    return;
                break;
            case ETokenCategory::token_syntax_array_close:
                if (eState != EState::array_content_and_close)
                    return; // Unexpected
                eState = EState::post_value;
                CodeSnippet(m_nPostValuesArray).SetTokenList(std::move(lstWhitespace));
                break;
            default:
                return; // Unexpected
            }
        }

        // Expecting the array to have been closed
        if (eState != EState::post_value)
            return;
        
        CodeSnippet(m_nPostValueCode).SetTokenList(std::move(lstWhitespace));
    }

    bool CArray::TableArray() const
    {
        // At least one table is needed
        if (!GetCount()) return false;

        // Iterate through the child-nodes and if a non-table is found, return false.
        for (uint32_t ui = 0; ui < GetCount(); ui++)
        {
            std::shared_ptr<CNode> ptrNode = Get(ui);
            if (!ptrNode) continue;
            if (ptrNode->GetType() != sdv::toml::ENodeType::node_table)
                return false;
        }

        // Only tables in the array, return true.
        return true;
    }

    bool CArray::Inline() const
    {
        return m_bInline;
    }

    bool CArray::Inline(bool bInline, bool bIncludeChildren /*= true*/)
    {
        // When inline, all children are also inline.
        if (m_bInline && bInline) return true; // Nothing to do

        // When not inline and children are not to be considered, shortcut when possible
        if (!m_bInline && !bInline && !bIncludeChildren) return true; // Nothing to do

        // Allowed to make standard
        if (!bInline && !CanMakeStandard()) return false; // Nothing to do

        // If a parent if inline, the child needs to be inline as well.
        bool bTargetInline = bInline;
        auto ptrParent = GetParentPtr();
        if (ptrParent && ptrParent->Inline())
            bTargetInline = true;

        // If making inline, make children inline first (call to base class).
        // If making standard, make standard before making children inline.
        if (bTargetInline)
        {
            if (CNodeCollection::Inline(true))
                m_bInline = true;
        }
        else
        {
            // Make standard
            m_bInline = false;
            CNodeCollection::Inline(false, bIncludeChildren);
        }

        return true;
    }

    bool CArray::CanMakeStandard() const
    {
        // Check with the collection
        if (!CNodeCollection::CanMakeStandard())
            return false;

        // To make an array as standard node, this is only possible when the array is a table array.
        return TableArray();
    }

    bool CArray::LastNodeWithSucceedingComma() const
    {
        return m_bLastChildNodeWithComma;
    }

    bool CArray::Combine(const std::shared_ptr<CNodeCollection>& rptrCollection)
    {
        if (!rptrCollection) return false;  // No collection supplied.
        if (rptrCollection == Cast<CNodeCollection>()) return true; // Collections are identical, nothing to combine.
        if (rptrCollection->IsDescendant(shared_from_this())) return false; // Circular reference.

        // Lock the rebuild of the node order
        auto lock = Parser().CreateRebuildLockObject();

        // Differentiate between a table array and a normal array:
        //  - A normal array should be identical. If not, take over the complete array.
        //  - A table array might contain tables that are identical, extend with tables that are not.
        
        // Run through the provided collection list
        // If nodes are a different type, replace the nodes of the existing array by the nodes of the new array.
        // In all other cases, update the content if necessary.

        bool bResult = true;
        CGenContext contextGeneration;
        contextGeneration.SetOption(EGenerateOptions::reduce_whitespace);
        contextGeneration.SetOption(EGenerateOptions::full_header);
        bool bTableArray = TableArray() && rptrCollection->Cast<CArray>()->TableArray();
        for (uint32_t uiIndex = 0; uiIndex < rptrCollection->GetCount(); uiIndex++)
        {
            // Get the new and potentially existing nodes.
            auto ptrNewNode = rptrCollection->Get(uiIndex);
            if (!ptrNewNode) continue;
            std::shared_ptr<CNode> ptrExistNode;
            if (bTableArray)
            {
                // Go through the current array and compare the generate TOML string for identical values.
                CGenContext contextComparison;
                contextComparison.SetOption(EGenerateOptions::no_comments);
                std::string ssNewNodeTOML = ptrNewNode->GenerateTOML(contextComparison);
                bool bIdentical = false;
                for (uint32_t uiExistIndex = 0; !bIdentical && (uiExistIndex < GetCount()); uiExistIndex++)
                {
                    ptrExistNode = Get(uiExistIndex);
                    if (!ptrExistNode) continue;
                    std::string ssExistTOML = ptrExistNode->GenerateTOML(contextComparison);
                    bIdentical = ssNewNodeTOML == ssExistTOML;
                }
                if (!bIdentical)
                    ptrExistNode.reset();
            } else
                ptrExistNode = Get(uiIndex);

            // Check whether there is an existing node. If not, add the node.
            if (!ptrExistNode)
            {
                bResult &= InsertTOML("", ptrNewNode->GenerateTOML(contextGeneration), true) ==
                    sdv::toml::INodeCollectionInsert::EInsertResult::insert_success;
                continue;
            }

            // The node exists. Are the node types identical? If not, replace the existing node with the new node
            if (ptrNewNode->GetType() != ptrExistNode->GetType())
            {
                uint32_t uiExistIndex = ptrExistNode->GetIndex();
                bool bLocalResult = ptrExistNode->DeleteNode();
                bResult &= bLocalResult;
                if (bLocalResult)
                    bResult &= InsertTOML(Get(uiExistIndex), ptrNewNode->GenerateTOML(contextGeneration), true).first
                               == sdv::toml::INodeCollectionInsert::EInsertResult::insert_success;
                continue;
            }

            // The nodes have identical type. Are they a collection themselves, then use the combine function of the collection.
            if (ptrExistNode->Cast<CNodeCollection>())
            {
                bResult &= ptrExistNode->Cast<CNodeCollection>()->Combine(ptrNewNode->Cast<CNodeCollection>());
                continue;
            }

            // Both nodes are value nodes of the same type. Check the value to be identical. If not, update the value.
            if (ptrExistNode->GetValue() != ptrNewNode->GetValue())
            {
                bResult &= ptrExistNode->ChangeValue(ptrNewNode->GetValue());
                continue;
            }

            // Nodes are identical; there's nothing to do.
        }

        // Check whether the current array is larger than the provided array. If so, remove the entries at the end to make it the
        // same size.
        while (!bTableArray && (GetCount() > rptrCollection->GetCount()))
        {
            auto ptrExistNode = Get(GetCount() - 1);
            if (!ptrExistNode) break;
            bool bLocalResult = ptrExistNode->DeleteNode();
            bResult &= bLocalResult;
            if (!bLocalResult) break; // Prevent endless loop.
        }

        return bResult;
    }

    bool CArray::Reduce(const std::shared_ptr<CNodeCollection>& rptrCollection)
    {
        if (!rptrCollection) return false;  // No collection supplied.
        if (rptrCollection == Cast<CNodeCollection>()) return false; // Collections are identical, this would empty the collection.
        if (rptrCollection->IsDescendant(shared_from_this())) return false; // Circular reference.
        
        // Lock the rebuild of the node order
        auto lock = Parser().CreateRebuildLockObject();

        // DIfferentiate between a table array and a normal array:
        //  - A normal array should be identical. If so, remove the complete array.
        //  - A table array might contain tables that are identical, those are removed. All others stay.

        // For normal array, simply compare the generate TOML strings. If identical, remove the array content.
        bool bResult = true;
        CGenContext context;
        context.SetOption(EGenerateOptions::no_comments);
        if (!TableArray() || !rptrCollection->Cast<CArray>()->TableArray())
        {
            // At least one of the arrays is a normal arrays, compare content as strings
            std::string ssReductorNodeTOML = rptrCollection->GenerateTOML(context);
            std::string ssExistNodeTOML = GenerateTOML(context);
            if (ssReductorNodeTOML == ssExistNodeTOML)
            {
                // Remove the content of the array
                while (GetCount())
                {
                    auto ptrExistNode = Get(GetCount() - 1);
                    if (!ptrExistNode) break;
                    bool bLocalResult = ptrExistNode->DeleteNode();
                    bResult &= bLocalResult;
                    if (!bLocalResult) break; // Prevent endless loop.
                }
            }
            return bResult;
        }

        // For table arrays, run through the provided collection list.
        // If nodes don't exist in the current collection, leave the nodes in the collection.
        // If nodes exist in the current collection, but are different, leave the nodes in the collection.
        // If the nodes exist and are identical, delete the nodes from the collection.
        for (uint32_t uiIndex = 0; uiIndex < rptrCollection->GetCount(); uiIndex++)
        {
            // Get the new and potentially existing nodes.
            auto ptrReductorNode = rptrCollection->Get(uiIndex);
            if (!ptrReductorNode) continue;

            // Go through the current array and compare the generate TOML string for identical values.
            std::string ssReductorNodeTOML = ptrReductorNode->GenerateTOML(context);
            for (uint32_t uiExistIndex = 0; uiExistIndex < GetCount(); uiExistIndex++)
            {
                std::shared_ptr<CNode> ptrExistNode = Get(uiExistIndex);
                if (!ptrExistNode) continue;
                std::string ssExistTOML = ptrExistNode->GenerateTOML(context);
                if (ssReductorNodeTOML == ssExistTOML)
                {
                    // Found an identical table. Remove the table from the array.
                    bResult &= ptrExistNode->DeleteNode();
                    break;
                }
            }
        }

        return bResult;
    }

    bool CRootTable::DeleteNode()
    {
        // Cannot delete the root node.
        return false;
    }
} // namespace toml_parser