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

    bool CGenContext::PartOfExcludedParents(const std::shared_ptr<const CNode>& rptrNode) const
    {
        if (!m_ptrTopMostNode) return false;
        std::shared_ptr<const CNode> ptrParent = m_ptrTopMostNode->GetParentPtr();
        while (ptrParent)
        {
            if (ptrParent == rptrNode) return true;
            ptrParent = ptrParent->GetParentPtr();
        }
        return false;
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
        m_ssName(rssName), m_ssRawName(rssRawName), m_rParser(rparser)
    {}

    CNode::~CNode()
    {}

    CParser& CNode::Parser()
    {
        return m_rParser;
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
        std::shared_ptr<CNodeCollection> ptrView = m_ptrView.lock();
        if (!ptrView) ptrView = m_ptrParent.lock();
        if (!ptrView) return sdv::toml::npos;

        return ptrView->FindIndex(std::const_pointer_cast<CNode>(shared_from_this()));
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

    void CNode::AutomaticFormat()
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

        // Remove the node from the view (this could also be the parent).
        auto ptrView = GetViewPtr();
        if (ptrView) ptrView->RemoveFromView(shared_from_this());

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

        m_ptrParent = rptrParent;
        if (!rptrParent) return;
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

    void CNode::SetViewPtr(const std::shared_ptr<CNodeCollection>& rptrView)
    {
        m_ptrView = rptrView;
    }

    std::shared_ptr<CNodeCollection> CNode::GetViewPtr() const
    {
        auto ptrView = m_ptrView.lock();
        if (!ptrView) ptrView = m_ptrParent.lock();
        return ptrView;
    }

    bool CNode::IsPartOfView(const CGenContext& rContext, const std::shared_ptr<const CNodeCollection>& rptrNode) const
    {
        std::shared_ptr<CNodeCollection> ptrStoredView = m_ptrView.lock();
        if (ptrStoredView && !rContext.PartOfExcludedParents(ptrStoredView))
            return ptrStoredView == rptrNode;
        return true;
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

    bool CValueNode::Inline(bool bInline)
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

    void CNodeCollection::AutomaticFormat()
    {
        // Format each child-node
        for (std::shared_ptr<CNode>& rptrNode : m_lstNodes)
            rptrNode->AutomaticFormat();
        CNode::AutomaticFormat();
    }

    uint32_t CNodeCollection::GetCount() const
    {
        return static_cast<uint32_t>(m_vecNodeOrder.size());
    }

    sdv::IInterfaceAccess* CNodeCollection::GetNode(/*in*/ uint32_t uiIndex) const
    {
        auto ptrNode = Get(uiIndex);
        return static_cast<sdv::IInterfaceAccess*>(ptrNode.get());
    }

    std::shared_ptr<CNode> CNodeCollection::Get(uint32_t uiIndex) const
    {
        if (static_cast<size_t>(uiIndex) >= m_vecNodeOrder.size())
            return nullptr;

        return m_vecNodeOrder[uiIndex];
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

    sdv::IInterfaceAccess* CNodeCollection::InsertValue(uint32_t uiIndex, const sdv::u8string& ssName, sdv::any_t anyValue)
    {
        std::stringstream sstreamTOML;
        std::string ssAccessName = ssName;
        if (GetType() == sdv::toml::ENodeType::node_array)
            ssAccessName = "[" + std::to_string(std::min(uiIndex, GetCount())) + "]";
        else
            sstreamTOML << QuoteText(ssName, EQuoteRequest::smart_key) << " = ";
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
        if (InsertTOML(uiIndex, sstreamTOML.str(), true) != EInsertResult::insert_success)
            return nullptr;
        
        // Return the inserted node.
        return GetNodeDirect(ssAccessName);
    }

    sdv::IInterfaceAccess* CNodeCollection::InsertArray(uint32_t uiIndex, const sdv::u8string& ssName)
    {
        // Insert the array node
        std::string ssAccessName = ssName;
        std::string ssTOML;
        if (GetType() == sdv::toml::ENodeType::node_array)
        {
            ssTOML = "[]";
            ssAccessName = "[" + std::to_string(std::min(uiIndex, GetCount())) + "]";
        }
        else
            ssTOML = QuoteText(ssName, EQuoteRequest::smart_key) + " = []";
        if (InsertTOML(uiIndex, ssTOML, true) != EInsertResult::insert_success)
            return nullptr;

        // Return the inserted node.
        return GetNodeDirect(ssAccessName);
    }

    sdv::IInterfaceAccess* CNodeCollection::InsertTable(uint32_t uiIndex, const sdv::u8string& ssName,
        sdv::toml::INodeCollectionInsert::EInsertPreference ePreference)
    {
        // Insert inline or standard table
        std::string ssAccessName = ssName;
        std::string ssTOML;
        if (Inline() || ePreference == sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline)
        {
            if (GetType() == sdv::toml::ENodeType::node_array)
            {
                ssTOML = "{}";
                ssAccessName = "[" + std::to_string(std::min(uiIndex, GetCount())) + "]";
            }
            else
                ssTOML = QuoteText(ssName, EQuoteRequest::smart_key) + " = {}";
        }
        else if (GetType() == sdv::toml::ENodeType::node_array) // Node is a table array
        {
            ssTOML = "[[" + QuoteText(GetName(), EQuoteRequest::smart_key) + "]]";
            ssAccessName = "[" + std::to_string(std::min(uiIndex, GetCount())) + "]";
        }
        else
            ssTOML = "[" + QuoteText(ssName, EQuoteRequest::smart_key) + "]";

        // Insert the table node
        if (InsertTOML(uiIndex, ssTOML, true) != EInsertResult::insert_success)
            return nullptr;

        // Return the inserted node.
        return GetNodeDirect(ssAccessName);
    }

    sdv::IInterfaceAccess* CNodeCollection::InsertTableArray(uint32_t uiIndex, const sdv::u8string& ssName,
        sdv::toml::INodeCollectionInsert::EInsertPreference ePreference)
    {
        // Insert inline or standard table array
        std::string ssAccessName = ssName;
        std::string ssTOML;

        if (Inline() || ePreference == sdv::toml::INodeCollectionInsert::EInsertPreference::prefer_inline)
        {
            if (GetType() == sdv::toml::ENodeType::node_array)
            {
                ssTOML = "[{}]";
                ssAccessName = "[" + std::to_string(std::min(uiIndex, GetCount())) + "][0]";
            }
            else
            {
                // If there is a table array with the same name already, the node will be added to the end.
                auto ptrExistingNode = Direct(ssName);
                auto ptrExistingArray = ptrExistingNode ? ptrExistingNode->Cast<CArray>() : std::shared_ptr<const CArray>();
                if (ptrExistingArray && !ptrExistingArray->TableArray())
                    return nullptr;
                size_t nCount = ptrExistingArray ? ptrExistingArray->GetCount() : 0;
                ssTOML = QuoteText(ssName, EQuoteRequest::smart_key) + " = [{}]";
                ssAccessName += "[" + std::to_string(nCount) + "]";
            }
        }
        else
        {
            // If there are table arrays, the node is added to the table array, but the order is determined by this node. Count the
            // amount of nodes before the insertion point.
            ssTOML = "[[" + QuoteText(ssName, EQuoteRequest::smart_key) + "]]";
            size_t nTableArrayCnt = std::count_if(m_vecNodeOrder.begin(),
                m_vecNodeOrder.begin() + std::min(static_cast<size_t>(uiIndex), m_vecNodeOrder.size()),
                [&](const std::shared_ptr<const CNode>& rptrNode) { return rptrNode->GetName() == ssName; });
            ssAccessName += "[" + std::to_string(nTableArrayCnt) + "]";
        }

        // Insert the table node
        if (InsertTOML(uiIndex, ssTOML, true) != EInsertResult::insert_success)
            return nullptr;

        // Return the inserted node.
        return GetNodeDirect(ssAccessName);
    }

    sdv::toml::INodeCollectionInsert::EInsertResult CNodeCollection::InsertTOML(uint32_t uiIndex, const sdv::u8string& ssTOML,
        bool bRollbackOnPartly)
    {
        try
        {
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
                    return sdv::toml::INodeCollectionInsert::EInsertResult::insert_fail;    
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
                        ptrPotentialNewNode->Cast<CNodeCollection>()->MakeStandard();
                    continue;
                }

                // Node exists already. Add to duplicate name nodes vector (or fail if no all nodes should fit).
                if (bRollbackOnPartly)
                    return sdv::toml::INodeCollectionInsert::EInsertResult::insert_fail;
                vecDuplicateNameNodes.push_back(ptrPotentialNewNode);
            }

            // Delete the nodes that have duplicate names
            for (auto& rptrNode : vecDuplicateNameNodes)
                rptrNode->DeleteNode();

            // Any nodes left to insert?
            if (!ptrCollection->GetCount())
                return sdv::toml::INodeCollectionInsert::EInsertResult::insert_fail;

            // Add the child nodes to the collection (use a copy of the nodes list, since SetParentPtr changes the node list).
            auto lstCopyNodes = ptrCollection->m_lstNodes;
            std::vector<std::shared_ptr<CNode>> vecSkipNodes;
            for (auto ptrNewNode : lstCopyNodes)
            {
                // If the node is a table array, it is allowed that the tables are added to an existing table array.
                auto ptrNewArray = ptrNewNode->Cast<CArray>();
                auto ptrExistingNode = Direct(ptrNewNode->GetName());
                auto ptrExistingArray = ptrExistingNode ? ptrExistingNode->Cast<CArray>() : std::shared_ptr<CArray>();
                if (ptrNewArray && ptrNewArray->TableArray() && ptrExistingArray &&
                    (ptrExistingArray->TableArray() || !ptrExistingArray->GetCount()))
                {
                    // Determine the current index of the array. This is used to determine to insert the nodes before or behind
                    // the existing nodes within the array.
                    uint32_t uiCurrentPos = ptrExistingNode->GetIndex();

                    // Insert each table in the already existing table array. Assign the parent pointer and the location in the
                    // parent table array.
                    for (uint32_t uiNewArrayIndex = 0; uiNewArrayIndex < ptrNewArray->GetCount(); uiNewArrayIndex++)
                    {
                        auto ptrNewTable = ptrNewArray->Get(uiNewArrayIndex);
                        ptrNewTable->SetParentPtr(ptrExistingArray);
                        ptrExistingArray->m_vecNodeOrder.insert(
                            uiCurrentPos > uiIndex ? (ptrExistingArray->m_vecNodeOrder.begin() + uiNewArrayIndex) : 
                                ptrExistingArray->m_vecNodeOrder.end(),
                            ptrNewTable);
                    }

                    // Do not process the table array when processing the position of all nodes in the parsed TOML code.
                    vecSkipNodes.push_back(ptrNewNode);
                    continue;
                }

                // Inserting values from an existing array into an array with the same name (which is not a table array) is not
                // automatically supported. For this the array content needs to specifically be added to the existing array.
                if (ptrExistingArray)
                    return sdv::toml::INodeCollectionInsert::EInsertResult::insert_fail; // Should not happen

                // Asign the new parent pointer.
                if (ptrNewNode->Cast<CNodeCollection>() && Inline())
                    ptrNewNode->Cast<CNodeCollection>()->MakeInline();
                ptrNewNode->SetParentPtr(Cast<CNodeCollection>());
            }
            
            // Insert the nodes in the node vector at the required index.
            size_t nTargetIndex = uiIndex;
            for (uint32_t uiNewNodeIndex = 0; uiNewNodeIndex < ptrCollection->GetCount(); uiNewNodeIndex++)
            {
                auto ptrNewNode = ptrCollection->Get(uiNewNodeIndex);

                // Skip when the node is already inserted previously.
                if (std::find(vecSkipNodes.begin(), vecSkipNodes.end(), ptrNewNode) != vecSkipNodes.end()) continue;

                // The target index should not supercede the size of the vector
                if (nTargetIndex > GetCount())
                    nTargetIndex = GetCount();

                // If the node collection is
                // - inline table or table array
                //   --> standard nodes will be converted to inline nodes.
                //   --> table-arrays can be added to additional table arrays
                // - standard table or table array
                //   --> standard nodes will be inserted behind the inline nodes.
                //   --> inline nodes will be inserted before the standard nodes.
                if (Inline())
                {
                    // Only inline nodes can be inserted in other inline nodes.
                    if (ptrNewNode->Cast<CNodeCollection>())
                        ptrNewNode->Cast<CNodeCollection>()->MakeInline();
                }
                else
                {
                    // Determine the begin of standard nodes in the node vector
                    size_t nBeginStandardIndex = 0;
                    for (; nBeginStandardIndex < m_vecNodeOrder.size(); nBeginStandardIndex++)
                    {
                        if (!m_vecNodeOrder[nBeginStandardIndex]->Inline())
                            break;
                    }

                    // Correct the target index if necessary
                    if (ptrNewNode->Inline())
                    {
                        if (nTargetIndex > nBeginStandardIndex)
                            nTargetIndex = nBeginStandardIndex;
                    }
                    else
                    {
                        if (nTargetIndex < nBeginStandardIndex)
                            nTargetIndex = nBeginStandardIndex;
                    }
                }

                // Insert the node
                m_vecNodeOrder.insert(m_vecNodeOrder.begin() + nTargetIndex, ptrNewNode);

                // Increase the target index for the next target
                nTargetIndex++;
            }

            // Return the result
            return vecDuplicateNameNodes.empty() ? sdv::toml::INodeCollectionInsert::EInsertResult::insert_success :
                sdv::toml::INodeCollectionInsert::EInsertResult::insert_partly_success;
        }
        catch (const sdv::toml::XTOMLParseException& /*rexcept*/)
        {
            return sdv::toml::INodeCollectionInsert::EInsertResult::insert_fail;
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

    bool CNodeCollection::CanMakeInline() const
    {
        // To make a node inline is always possible.
        return true;
    }

    bool CNodeCollection::MakeInline()
    {
        if (Inline()) return true;
        bool bRet = Inline(true);
        if (bRet) AutomaticFormat();
        return bRet;
    }

    bool CNodeCollection::CanMakeStandard() const
    {
        // To make a node as standard node, this is only possible when the parent is not inline.
        auto ptrParent = GetParentPtr();
        if (!ptrParent) return false;
        if (ptrParent->Inline()) return false;
        return true;
    }

    bool CNodeCollection::MakeStandard()
    {
        if (!Inline()) return true;
        bool bRet = Inline(false);
        if (bRet) AutomaticFormat();
        return bRet;
    }

    bool CNodeCollection::DeleteNode(const std::shared_ptr<CNode>& rptrNode)
    {
        // Remove from the view (just in case).
        RemoveFromView(rptrNode);

        // In case this is an array collection, it could still happen, that the node is in the vector. Explicitly remove the node
        // from the vector.
        auto itElementVec = std::find(m_vecNodeOrder.begin(), m_vecNodeOrder.end(), rptrNode);
        if (itElementVec != m_vecNodeOrder.end())
            m_vecNodeOrder.erase(itElementVec);

        // Find the element
        auto itElementLst = std::find(m_lstNodes.begin(), m_lstNodes.end(), rptrNode);
        if (itElementLst == m_lstNodes.end())
            return false;

        // Shift the node into the recycle bin.
        m_lstRecycleBin.push_back(rptrNode);
        m_lstNodes.erase(itElementLst);
        return true;
    }

    bool CNodeCollection::InsertIntoView(uint32_t uiIndex, const std::shared_ptr<CNode>& rptrNode)
    {
        // Check for vaidity
        if (!rptrNode) return false;
        if (Inline() && !rptrNode->Inline()) return false;  // Cannot assign a standard node to an inline node
        if (!IsDescendant(rptrNode)) return false;          // Must descent directly from this node

        // Get the current view
        auto ptrView = rptrNode->GetViewPtr();
        if (!ptrView) ptrView = rptrNode->GetParentPtr();
        if (!ptrView) return false;

        // Determine the start of the standard nodes in the vector.
        uint32_t uiStartStandard = 0;
        while (m_vecNodeOrder.size() > uiStartStandard && !m_vecNodeOrder[uiStartStandard]->IsStandard())
            ++uiStartStandard;

        // Determine the potential new index location.
        uint32_t uiNewIndex = uiIndex;
        if (rptrNode->IsStandard() && uiIndex < uiStartStandard) uiNewIndex = uiStartStandard;
        else if (rptrNode->IsInline() && uiIndex > uiStartStandard) uiNewIndex = uiStartStandard;
        if (static_cast<size_t>(uiNewIndex) > m_vecNodeOrder.size()) uiNewIndex = static_cast<uint32_t>(m_vecNodeOrder.size());

        // Special case, if the current view is identical and the node will be located at the same position, do not do anything.
        // This is necessary to keep the code snippets in place. Otherwise they might be relocated.
        // Also, if the current position is before the new position, correct the new index (since we remove the current position).
        if (ptrView == Cast<CNodeCollection>())
        {
            uint32_t uiCurrentIndex = rptrNode->GetIndex();
            if (uiCurrentIndex == uiNewIndex) return true;  // Nothing to do...
            if (uiCurrentIndex < uiNewIndex) uiNewIndex--;
            if (uiCurrentIndex < uiStartStandard) uiStartStandard--;
        }

        // Remove a previous view assignment (if necessary this will shift code snippets as well).
        ptrView->RemoveFromView(rptrNode);

        // Check whether the node is already part of the view
        if (std::find(m_vecNodeOrder.begin(), m_vecNodeOrder.end(), rptrNode) != m_vecNodeOrder.end())
            return true;    // Insert only once.
        
        // Determine the first position and the beyond-last position
        uint32_t uiFirst = rptrNode->IsStandard() ? uiStartStandard : 0;
        uint32_t uiLast = rptrNode->IsStandard() ? static_cast<uint32_t>(m_vecNodeOrder.size()) : uiStartStandard;

        // If the node is inserted at the first position, shift the out-of-scope code snippet from the first node.
        if ((uiNewIndex <= uiFirst) && (uiFirst < m_vecNodeOrder.size()))
        {
            rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before)
                .Insert(m_vecNodeOrder[uiFirst]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before));
            m_vecNodeOrder[uiFirst]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before).Clear();
        }

        // If the node is inserted at the last position, shift the out-of-scope code snippet from the last node.
        if ((uiNewIndex >= uiLast) && (uiFirst < m_vecNodeOrder.size()))
        {
            rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind)
                .Append(m_vecNodeOrder[uiLast - 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind));
            m_vecNodeOrder[uiLast - 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before).Clear();
        }

        // Insert the node
        m_vecNodeOrder.insert(m_vecNodeOrder.begin() + uiNewIndex, rptrNode);

        return true;
    }

    bool CNodeCollection::RemoveFromView(const std::shared_ptr<CNode>& rptrNode)
    {
        // Check for validity
        if (!rptrNode) return false;
        if (rptrNode->GetViewPtr() != Cast<CNodeCollection>()) return false;

        // Get the current position
        uint32_t uiCurrentIndex = rptrNode->GetIndex();
        if (uiCurrentIndex >= m_vecNodeOrder.size()) return false; // Not present (likely already removed)

        // Determine the start of the standard nodes in the vector.
        uint32_t uiStartStandard = 0;
        while (m_vecNodeOrder.size() > uiStartStandard && !m_vecNodeOrder[uiStartStandard]->IsStandard())
            ++uiStartStandard;

        // Determine the first position and the beyond-last position
        uint32_t uiFirst = rptrNode->IsStandard() ? uiStartStandard : 0;
        uint32_t uiLast  = rptrNode->IsStandard() ? static_cast<uint32_t>(m_vecNodeOrder.size()) : uiStartStandard;

        // Move the out-of-scope code to another node
        if ((uiLast - uiFirst) > 1)
        {
            // If removing the node and a node with the same classification (standard or inline) is still available, shift the
            // out-of-scope code to the left-over nodes with the same classification.
            if (uiCurrentIndex <= uiFirst)
            {
                // The first node will be removed - copy the out-of-scope code to the node behind
                m_vecNodeOrder[uiCurrentIndex + 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before)
                    .Insert(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind));
                m_vecNodeOrder[uiCurrentIndex + 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before)
                    .Insert(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before));
            } else if (uiCurrentIndex >= (uiLast - 1))
            {
                // The last node will be removed - copy the out-of-scope code to the node before
                m_vecNodeOrder[uiCurrentIndex - 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind)
                    .Append(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before));
                m_vecNodeOrder[uiCurrentIndex - 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind)
                    .Append(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind));
            } else
            {
                // Copy the out-of-scope code from the end of the node to the node following
                m_vecNodeOrder[uiCurrentIndex - 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind)
                    .Append(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before));
                m_vecNodeOrder[uiCurrentIndex + 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before)
                    .Insert(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind));
            }
        }
        else if (m_vecNodeOrder.size() > 1)
        {
            // If there are no more nodes with the same classification, but there are nodes with another classification, then move
            // the out-of-scope code to the node with the other classification.
            if (uiCurrentIndex == 0)
            {
                // The first node will be removed - copy the out-of-scope code to the node behind.
                m_vecNodeOrder[uiCurrentIndex + 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before)
                    .Insert(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind));
                m_vecNodeOrder[uiCurrentIndex + 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before)
                    .Insert(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before));
            } else
            {
                // The last node will be removed - copy the out-of-scope code to the node before.
                m_vecNodeOrder[uiCurrentIndex - 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind)
                    .Append(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before));
                m_vecNodeOrder[uiCurrentIndex - 1]->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind)
                    .Append(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind));
            }
        }
        else if (IsStandard())
        {
            // If there are no nodes left over, but this node is a standard node, move the out-of-scope code to this node.
            CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind)
                .Insert(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind));
            CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind)
                .Insert(rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before));
        }

        // If this node is not a standard node, then the out-of-scope code cannot be maintained.

        // Delete the out-of-scope code.
        rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_before).Clear();
        rptrNode->CodeSnippet(sdv::toml::INodeInfo::ECommentType::out_of_scope_comment_behind).Clear();

        // Remove the actual pointer.
        rptrNode->SetViewPtr(nullptr);

        // And remove from the vector (but only when the collection is not an array).
        if (!Cast<CArray>())
            m_vecNodeOrder.erase(m_vecNodeOrder.begin() + uiCurrentIndex);

        return true;
    }

    uint32_t CNodeCollection::FindIndex(const std::shared_ptr<CNode>& rptrNode) const
    {
        // Find the element
        for (uint32_t uiIndex = 0; uiIndex < m_vecNodeOrder.size(); uiIndex++)
        {
            if (rptrNode == m_vecNodeOrder[uiIndex])
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

    CTable::CTable(CParser& rparser, const std::string& rssName, const std::string& rssRawName, bool bDefaultInline,
        bool bExplicit /*= true*/) :
        CNodeCollection(rparser, rssName, rssRawName), m_bDefinedExplicitly(bExplicit), m_bInline(bDefaultInline)
    {}

    sdv::toml::ENodeType CTable::GetType() const
    {
        return IsDeleted() ? sdv::toml::ENodeType::node_invalid : sdv::toml::ENodeType::node_table;
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
        if (!contextCopy.TopMostNode() || contextCopy.CheckOption(toml_parser::EGenerateOptions::full_header))
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
            if (bTableArray || !ptrNode->Cast<CTable>() || ptrNode->Inline())
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

            // If the node has a view pointer, which is not identical to this pointer, do not print the node... it will be printed
            // by a different node.
            if (!ptrNode->IsPartOfView(contextCopy, Cast<CNodeCollection>())) continue;

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
        auto ptrParent = GetParentPtr();
        if (!ptrParent) return m_bInline;
        return m_bInline;
    }

    bool CTable::Inline(bool bInline)
    {
        if (bInline == m_bInline) return true;
        auto ptrParent = GetParentPtr();
        if (ptrParent && ptrParent->Inline())
            m_bInline = true;
        else
            m_bInline = bInline;

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
            // Exception to the rule... when the existing node is a table array and the node to add is a table, do not do replace the node.
            if (!ptrExistNode || (ptrNewNode->GetType() == sdv::toml::ENodeType::node_table &&
                    ptrExistNode->Cast<CArray>() && ptrExistNode->Cast<CArray>()->TableArray()))
            {
                bResult &= InsertTOML(sdv::toml::npos, ptrNewNode->GenerateTOML(contextGeneration), true) ==
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
                    bResult &= InsertTOML(uiExistIndex, ptrNewNode->GenerateTOML(contextGeneration), true) ==
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

            // Check whether the node is exists; if not, leave the node in the collection.
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

    CArray::CArray(CParser& rparser, const std::string& rssName, const std::string& rssRawName,
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

            // If the node has a view pointer, which is not identical to the this pointer, do not print the node... it will be
            // printed by a different node.
            if (!ptrNode->IsPartOfView(contextCopy, Cast<CNodeCollection>()))
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

    bool CArray::CanMakeStandard() const
    {
        // Check with the collection
        if (!CNodeCollection::CanMakeStandard())
            return false;

        // To make an array as standard node, this is only possible when the array is a table array.
        return TableArray();
    }

    bool CArray::Inline() const
    {
        return m_bInline;
    }

    bool CArray::Inline(bool bInline)
    {
        // Does anything change?
        if (bInline == m_bInline) return true;

        // When making standard, the parent should not be inline.
        auto ptrParent = GetParentPtr();
        if (!ptrParent) return false;
        if (ptrParent->Inline() && !bInline) return false;

        // The array is inline per default. Only in the case of a table array (an array with only tables, and at least one table)
        // the array could be standard.
        if (!TableArray()) return false;

        // Check the order in the vector of the view or of the parent. When becoming inline, must be located before the standard
        // nodes. When becoming standard, must be located behind the inline nodes.
        // Special case for a table array: when becoming inline, remove all the tables from the view and add the array to the
        // correct location. When becoming standard, remove the array from the location and add all the tables into the view.
        m_bInline = bInline;
        if (bInline)
        {
            // If becoming inline, remove the view and make all children inline as well.
            uint32_t uiCurrentPos = sdv::toml::npos;
            for (uint32_t uiIndex = 0; uiIndex < GetCount(); uiIndex++)
            {
                auto ptrNode = Get(uiIndex);
                if (!ptrNode) continue;
                if (uiCurrentPos == sdv::toml::npos)
                    uiCurrentPos = ptrNode->GetIndex();
                if (ptrNode->Cast<CNodeCollection>())
                    ptrNode->Cast<CNodeCollection>()->MakeInline();
                InsertIntoView(uiIndex, ptrNode);
            }

            // Move the array into the view of the parent.
            ptrParent->InsertIntoView(uiCurrentPos, shared_from_this());
        } else
        {
            // Get the current position
            uint32_t uiCurrentPos = GetIndex();

            // Make all the children standard as well and move the into the view of the parent.
            for (uint32_t uiIndex = 0; uiIndex < GetCount(); uiIndex++)
            {
                auto ptrNode = Get(uiIndex);
                if (!ptrNode) continue;
                if (uiCurrentPos == sdv::toml::npos)
                    uiCurrentPos = ptrNode->GetIndex();
                if (ptrNode->Inline() && ptrNode->Cast<CNodeCollection>())
                    ptrNode->Cast<CNodeCollection>()->MakeStandard();
                ptrParent->InsertIntoView(uiCurrentPos, ptrNode);
                if (uiCurrentPos != sdv::toml::npos)
                    ++uiCurrentPos;
            }

            // Remove the array from the view of the parent
            ptrParent->RemoveFromView(shared_from_this());
        }

        return true;
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

        // Differentiate between a table array and a normal array:
        //  - A normal array should be identical. If not, take over the complete array.
        //  - A table array might contain tables that are identical, extend with tables that are not.
        
        // Run through the provided collection list
        // If nodes are a different type, replace the nodes of the existing array by the nodes of the new array.
        // In all other cases, update the content if necessary.

        bool bResult = true;
        toml_parser::CGenContext contextGeneration;
        contextGeneration.SetOption(EGenerateOptions::reduce_whitespace);
        contextGeneration.SetOption(EGenerateOptions::full_header);
        bool bTableArray = TableArray() && rptrCollection->Cast<CArray>()->TableArray();
        for (uint32_t uiIndex = 0; uiIndex < rptrCollection->GetCount(); uiIndex++)
        {
            // Get the new and potentially existing nodes.
            auto ptrNewNode = rptrCollection->Get(uiIndex);
            if (!ptrNewNode) continue;
            std::shared_ptr<CNode> ptrExistNode;
            uint32_t uiTargetIndex = uiIndex;
            if (bTableArray)
            {
                // Go through the current array and compare the generate TOML string for identical values.
                toml_parser::CGenContext contextComparison;
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
                {
                    uiTargetIndex = sdv::toml::npos;    // New node will be added at the end of the array.
                    ptrExistNode.reset();
                }
            } else
                ptrExistNode = Get(uiIndex);

            // Check whether there is an existing node. If not, add the node.
            if (!ptrExistNode)
            {
                bResult &= InsertTOML(uiTargetIndex, ptrNewNode->GenerateTOML(contextGeneration), true) ==
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
                    bResult &= InsertTOML(uiExistIndex, ptrNewNode->GenerateTOML(contextGeneration), true) ==
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
        
        // DIfferentiate between a table array and a normal array:
        //  - A normal array should be identical. If so, remove the complete array.
        //  - A table array might contain tables that are identical, those are removed. All others stay.

        // For normal array, simply compare the generate TOML strings. If identical, remove the array content.
        bool bResult = true;
        toml_parser::CGenContext context;
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