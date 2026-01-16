#include <algorithm>
#include "parser_node_toml.h"
#include "exception.h"
#include <sstream>
#include "parser_toml.h"

/// The TOML parser namespace
namespace toml_parser
{
    CGenContext::CGenContext(const std::string& rssPrefixKey /*= std::string()*/, uint32_t uiOptions /*= 0*/) :
        m_ssPrefixKey(rssPrefixKey), m_uiOptions(uiOptions)
    {}

    void CGenContext::InitTopMostNode(const std::shared_ptr<const CNode>& rptrNode)
    {
        if (!m_ptrTopMostNode)
            m_ptrTopMostNode = rptrNode;
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

    CGenContext CGenContext::CopyWithContext(const std::string& rssNewKeyContext) const
    {
        CGenContext context(m_ssPrefixKey, m_uiOptions);
        context.m_ssKeyContext = rssNewKeyContext;
        context.m_ptrTopMostNode = m_ptrTopMostNode;
        context.m_bTopMost = false;
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
        std::shared_ptr<CNodeCollection> ptrParent = m_ptrParent.lock();
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

    void CNode::SetComment(const sdv::u8string& ssComment, uint32_t uiFlags)
    {
        uint32_t uiIndex = uiFlags & static_cast<uint32_t>(sdv::toml::INodeInfo::ECommentFlags::comment_index_mask);

        // In case of raw comment code or when the replace flag is enabled, remove the current tokens from the list
        if ((uiFlags & static_cast<uint32_t>(sdv::toml::INodeInfo::ECommentFlags::raw_comment))
            || (uiFlags & static_cast<uint32_t>(sdv::toml::INodeInfo::ECommentFlags::replace_whitespace)))
        {
            CodeSnippet(uiIndex).List().clear();
            CodeSnippet(uiIndex).Str().clear();
        }

        // Generate code from the string when not raw mode
        std::string ssCommentCode;
        TTokenListIterator itInsertLocation = CodeSnippet(uiIndex).List().end();
        if (!(uiFlags & static_cast<uint32_t>(sdv::toml::INodeInfo::ECommentFlags::raw_comment)))
        {
            // Find the first comment and then the last comment. Delete everything in between.
            TTokenListIterator it = CodeSnippet(uiIndex).List().begin();
            TTokenListIterator itStart = itInsertLocation;
            TTokenListIterator itStop = itInsertLocation;
            while (it != CodeSnippet(uiIndex).List().end())
            {
                if (it->Category() == ETokenCategory::token_comment)
                {
                    if (itStart == itInsertLocation)
                        itStart = it;
                    itStop = it;
                }
            }

            // Move the stop to after the last comment
            if (itStop != CodeSnippet(uiIndex).List().end())
                itStop++;

            // Include the newline following the last comment
            if (itStop != CodeSnippet(uiIndex).List().end() && itStop->Category() == ETokenCategory::token_syntax_new_line)
                itStop++;

            // Determine insert location in text position (when on newline, no insert location, when following the code, determine the insert location based on the other topics. There is a max, then reduce).
            // Determine max length in text position
            // Build code string
            // The whitespace location is dependable on whether this is an assignment and the parent is a normal 





            // Remove the tokens if there are any.
            if (itStart != CodeSnippet(uiIndex).List().end())
                CodeSnippet(uiIndex).List().erase(itStart, itStop);

            // The insert location is the stop iterator
            itInsertLocation = itStop;


        }
        else
            ssCommentCode = ssComment;


        // Use the lexer to generate the tokens
        CLexer lexerComments;
        lexerComments.Feed(ssCommentCode);
        lexerComments.NavigationMode(CLexer::ENavigationMode::do_not_skip_anything);
        std::reference_wrapper<const CToken> refToken = lexerComments.Consume();
        while (refToken.get().Category() != ETokenCategory::token_none)
        {
            // Add whitespace and comments only...
            switch (refToken.get().Category())
            {
            case toml_parser::ETokenCategory::token_comment:
            case toml_parser::ETokenCategory::token_whitespace:
            case toml_parser::ETokenCategory::token_syntax_new_line:
                CodeSnippet(uiIndex).List().insert(itInsertLocation, refToken.get());
                break;
            default:
                break;
            }
        }

        // Add a newline if the last token was a comment
        if (!CodeSnippet(uiIndex).List().empty() &&
            CodeSnippet(uiIndex).List().back().Category() == ETokenCategory::token_comment)
            CodeSnippet(uiIndex).List().push_back(CToken(ETokenCategory::token_syntax_new_line));
    }

    sdv::u8string CNode::GetComment(uint32_t uiFlags)
    {
        uint32_t uiIndex = uiFlags & static_cast<uint32_t>(sdv::toml::INodeInfo::ECommentFlags::comment_index_mask);

        // Iterate through the code tokens.
        std::stringstream sstream;
        for (const CToken& rToken : CodeSnippet(uiIndex).List())
        {
            // Differentiate between interpreted comment or raw code comment
            if (uiIndex & static_cast<uint32_t>(sdv::toml::INodeInfo::ECommentFlags::raw_comment))
            {
                switch (rToken.Category())
                {
                case toml_parser::ETokenCategory::token_whitespace:
                case toml_parser::ETokenCategory::token_comment:
                case toml_parser::ETokenCategory::token_syntax_new_line:
                    sstream << rToken.RawString();
                    break;
                default:
                    break;
                }
            }
            else
            {
                if (rToken.Category() == toml_parser::ETokenCategory::token_comment && !rToken.RawString().empty()
                        && rToken.RawString()[0] == '#')
                {
                    // The comment doesn't have a newline inside the comment text. This allows us to glue multiple comments to one
                    // large string.
                    // A new line will be inserted on the following conditions:
                    // - the line starts as comment, but only has whitespace.
                    // - there are at least some characters in the stream already.
                    // - the comment starts with a tab or multiple spaces
                    if (rToken.RawString().size() <= 1 || rToken.RawString().find_first_not_of(" \t", 1) == std::string::npos)
                    {
                        // Line has only whitespace
                        sstream << std::endl;
                        continue;
                    }
                    if (rToken.RawString()[1] == '\t')
                    {
                        // Line starts with a tab, are there any characters in the stream already.
                        if (!sstream.str().empty())
                            sstream << std::endl;
                        sstream << rToken.RawString().substr(1);
                        continue;
                    }
                    size_t nStartComment = rToken.RawString()[1] == ' ' ? 2 : 1;
                    if (!sstream.str().empty())
                    {
                        // The stream is not empty. Are there extra spaces (indicating a list or so)
                        if (rToken.RawString().substr(1, 2) == "  ")
                            sstream << std::endl;
                        else // No newline, but space?
                            if (sstream.str().back() != ' ')
                                sstream << " ";
                    }

                    // Add the content to the stream
                    sstream << rToken.RawString().substr(nStartComment);
                }

            }
        }

        // Return the comment string
        return sstream.str();
    }

    void CNode::AutomaticFormat()
    {
        // TODO EVE
        // Read the comment of each code snippet and remove the tokens. This will automatically format the code when recomposing.
        for (auto& rmapSnippets : m_vecCodeSnippets)
        {
            for (auto& rvtSnippet : rmapSnippets)
            {
                std::cout << rvtSnippet.first;
            }
        }
    }

    void CNode::UpdateNodeCode(const CNodeTokenRange& rNodeRange)
    {
        // Extract the node pre and post comments and whitespace (belonging and not belonging to the node).
        CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::out_of_scope_comment_before)).List() =
            rNodeRange.LinesBeforeNode().TokenListSLice();
        CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::out_of_scope_comment_behind)).List() =
            rNodeRange.LinesBehindNode().TokenListSLice();
        CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::comment_before)).List() =
            rNodeRange.NodeCommentsBefore().TokenListSLice();
        CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::comment_behind)).List() =
            rNodeRange.NodeCommentsBehind().TokenListSLice();
    }

    bool CNode::DeleteNode()
    {
        bool bRet = false;

        // Remove the node from the parent.
        std::shared_ptr<CNodeCollection> ptrParent = m_ptrParent.lock();
        if (ptrParent)
        {
            // Remove the node from the parent --> this will put the node in the recycle bin.
            bRet = ptrParent->RemoveNode(std::const_pointer_cast<CNode>(shared_from_this()));
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

    std::shared_ptr<CNodeCollection> CNode::GetParentPtr() const
    {
        return m_ptrParent.lock();
    }

    void CNode::SetParentPtr(const std::shared_ptr<CNodeCollection>& rptrParent)
    {
        m_ptrParent = rptrParent;
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

    bool CNode::IsPartOfView(const CGenContext& rContext, const std::shared_ptr<const CNodeCollection>& rptrNode) const
    {
        std::shared_ptr<CNodeCollection> ptrStoredView = m_ptrView.lock();
        if (ptrStoredView && !rContext.PartOfExcludedParents(ptrStoredView))
            return ptrStoredView == rptrNode;
        return true;
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
        auto lstKeyPath = GetRawPath(false);
        auto prPrefixKey = SplitNodeKey(rssPrefixKey);
        auto itPos = lstKeyPath.begin();
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
        for (const auto& prKey : lstKeyPath)
        {
            // In case of a table array, the table doesn't have a name. Skip the table in the key composition.
            if (prKey.second.empty())
                continue;

            // Add a dot if this is not the first key.
            if (!ssCustomKeyPath.empty())
                ssCustomKeyPath += ".";

            // Add the pre-key cpde snippet.
            ssCustomKeyPath += CodeSnippet(m_nPreKeyCode, prKey.first).Compose(CCodeSnippet::EComposeMode::compose_inline);

            // Add the raw key string
            ssCustomKeyPath += prKey.second;

            // Add the pos-key cpde snippet.
            ssCustomKeyPath += CodeSnippet(m_nPostKeyCode, prKey.first).Compose(CCodeSnippet::EComposeMode::compose_inline);
        }
        return ssCustomKeyPath;
    }

    const CNode::CCodeSnippet& CNode::CodeSnippet(size_t nIndex, const std::string& rssKey /*= std::string()*/) const
    {
        static CCodeSnippet sEmptyCodeSnippet;
        if (nIndex >= m_vecCodeSnippets.size()) return sEmptyCodeSnippet;
        auto itKey = m_vecCodeSnippets[nIndex].find(rssKey);
        if (itKey == m_vecCodeSnippets[nIndex].end()) return sEmptyCodeSnippet;
        return itKey->second;
    }

    CNode::CCodeSnippet& CNode::CodeSnippet(size_t nIndex, const std::string& rssKey /*= std::string()*/)
    {
        if (nIndex >= m_vecCodeSnippets.size()) m_vecCodeSnippets.resize(nIndex + 1);
        return m_vecCodeSnippets[nIndex][rssKey];
    }

    bool CNode::ExplicitlyDefined() const
    {
        // Default implementation is explicitly.
        return true;
    }

    void CNode::MakeExplicit()
    {
        // Default implementaiton is explicitly. Therfore, nothing to do.
    }

    std::list<CToken>& CNode::CCodeSnippet::List()
    {
        return m_lstTokens;
    }

    std::string& CNode::CCodeSnippet::Str()
    {
        return m_ssComment;
    }

    std::string CNode::CCodeSnippet::Compose(EComposeMode eMode, size_t nAssignmentOffset /*= 0*/, size_t /*nCommentOffset = 0*/) const
    {
        // Build the stream until the first comment.
        std::stringstream sstream;
        TTokenListIterator it = m_lstTokens.begin();
        while (it != m_lstTokens.end() && it->Category() != ETokenCategory::token_comment)
        {
            if ((eMode != EComposeMode::compose_behind) || (it->Category() != ETokenCategory::token_syntax_comma))
                sstream << it->RawString();
            ++it;
        }

        // Determine the last comment.
        TTokenListIterator itFirstComment = it;
        TTokenListIterator itLastComment = it;
        while (it != m_lstTokens.end())
        {
            if (it->Category() == ETokenCategory::token_comment)
                itLastComment = it;
            ++it;
        }

        // Determine the code behind the last comment (and the obligatory newline).
        TTokenListIterator itPostComment = itLastComment;
        if (itPostComment != m_lstTokens.end())
            ++itPostComment;
        if (itPostComment != m_lstTokens.end() && itPostComment->Category() == ETokenCategory::token_syntax_new_line)
            ++itPostComment;

        // Stream the comment
        if (m_ssComment.empty())
        {
            for (it = itFirstComment; it != itPostComment; ++it)
            {
                if ((eMode != EComposeMode::compose_behind) ||
                    (it->Category() != ETokenCategory::token_syntax_comma))
                    sstream << it->RawString();
            }
        }
        else
        {
            switch (eMode)
            {
            case EComposeMode::compose_inline:
            case EComposeMode::compose_behind:
                if (itFirstComment != m_lstTokens.end())
                    sstream << " ";
                break;
            default:
                break;
            }
            // TODO: Align the comment....
            sstream << "# " << m_ssComment << std::endl;

            // TODO: Add spaces for next assignment
            switch (eMode)
            {
            case EComposeMode::compose_inline:
                if (itPostComment != m_lstTokens.end() && nAssignmentOffset)
                    sstream << std::string(nAssignmentOffset, ' ');
                break;
            default:
                break;
            }
        }

        // Stream the rest of the tokens
        for (it = itPostComment; it != m_lstTokens.end(); ++it)
        {
            if ((eMode != EComposeMode::compose_behind) || (it->Category() != ETokenCategory::token_syntax_comma))
                sstream << it->RawString();
        }

        // In some case a default new-line is needed
        if (m_lstTokens.empty() && m_ssComment.empty())
        {
            switch (eMode)
            {
            case EComposeMode::compose_behind:
                sstream << std::endl;
                break;
            default:
                break;
            }
        }

        return sstream.str();
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

        std::stringstream sstream;

        // Determine whether the statement should be embedded (same line separated by commas) and should have an assignment (not
        // having a key name).
        auto ptrParent = GetParentPtr();
        if (!ptrParent) return {};
        bool bEmbedded = ptrParent->Inline() && ptrParent->ExplicitlyDefined();
        bool bAssignment = !bEmbedded || !ptrParent->Cast<CArray>(); // No assignment only with normal array
        bool bLastNode = ptrParent->CheckLast(std::const_pointer_cast<CNode>(shared_from_this()));
        std::string ssContext = rContext.KeyContext().empty() ?
                (rContext.TopMostNode() ? GetParentPath() : std::string()) : rContext.KeyContext();

        // Add unconnected pre node comments
        sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::out_of_scope_comment_before)).
                       Compose(CCodeSnippet::EComposeMode::compose_standalone);

        // Add pre node comments
        sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::comment_before)).
                       Compose(CCodeSnippet::EComposeMode::compose_before);

        // Add assignment
        if (bAssignment) // Not an array entry
            sstream << GetCustomPath(rContext.PrefixKey(), ssContext) << "=";

        // Stream the value
        sstream << CodeSnippet(m_nPreValueCode).Compose(CCodeSnippet::EComposeMode::compose_inline) << RawValueText() <<
            CodeSnippet(m_nPostValueCode).Compose(CCodeSnippet::EComposeMode::compose_inline);

        // Add a comma if this is not the last node.
        if (bEmbedded && !bLastNode) sstream << ",";

        // Add post node comments
        sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::comment_behind)).
            Compose(bLastNode ? CCodeSnippet::EComposeMode::compose_inline : CCodeSnippet::EComposeMode::compose_behind);

        // Add unconnected post node comments
        sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::out_of_scope_comment_behind))
                       .Compose(CCodeSnippet::EComposeMode::compose_standalone);

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
                CodeSnippet(m_nPreKeyCode, ssKeyName).List() = std::move(lstWhitespace);
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
                CodeSnippet(m_nPreValueCode).List() = std::move(lstWhitespace);
                break;
            case ETokenCategory::token_syntax_assignment:
                if (eState != EState::assignment_or_separator)
                    return; // Unexpected
                eState = EState::value;
                CodeSnippet(m_nPostKeyCode, ssKeyName).List() = std::move(lstWhitespace);
                break;
            case ETokenCategory::token_syntax_dot:
                if (eState != EState::assignment_or_separator)
                    return; // Unexpected
                eState = EState::key;
                CodeSnippet(m_nPostKeyCode, ssKeyName).List() = std::move(lstWhitespace);
                break;
            default:
                return; // Unexpected
            }
        }
        if (eState != EState::post_value)
            return; // Unexpected
        
        CodeSnippet(m_nPostValueCode).List() = std::move(lstWhitespace);
    }

    std::string CValueNode::RawValueText() const
    {
        return m_ssRawValue.empty() ? ValueText() : m_ssRawValue;
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
        return sdv::any_t(m_bVal);
    }

    bool CBooleanNode::ChangeValue(sdv::any_t anyNewValue)
    {
        m_bVal = anyNewValue.get<bool>();
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
        return sdv::any_t(m_iVal);
    }

    bool CIntegerNode::ChangeValue(sdv::any_t anyNewValue)
    {
        m_iVal = anyNewValue.get<int64_t>();
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
        return sdv::any_t(m_dVal);
    }

    bool CFloatingPointNode::ChangeValue(sdv::any_t anyNewValue)
    {
        m_dVal = anyNewValue.get<double>();
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
        return sdv::any_t(m_ssVal);
    }

    bool CStringNode::ChangeValue(sdv::any_t anyNewValue)
    {
        m_ssVal = anyNewValue.get<sdv::u8string>();
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

    sdv::IInterfaceAccess* CNodeCollection::InsertValue(uint32_t /*uiIndex*/, const sdv::u8string& /*ssName*/, sdv::any_t /*anyValue*/)
    {
        //std::shared_ptr<CNode> ptrNode;
        //switch (anyValue.eValType)
        //{
        //case sdv::any_t::EValType::val_type_bool:
        //    ptrNode = std::make_shared<CBooleanNode>(Parser(), ssName, anyValue.get<bool>());
        //    break;
        //case sdv::any_t::EValType::val_type_int8:
        //case sdv::any_t::EValType::val_type_uint8:
        //case sdv::any_t::EValType::val_type_int16:
        //case sdv::any_t::EValType::val_type_uint16:
        //case sdv::any_t::EValType::val_type_int32:
        //case sdv::any_t::EValType::val_type_uint32:
        //case sdv::any_t::EValType::val_type_int64:
        //case sdv::any_t::EValType::val_type_uint64:
        //    ptrNode = std::make_shared<CIntegerNode>(Parser(), ssName, anyValue.get<int64_t>());
        //    break;
        //case sdv::any_t::EValType::val_type_float:
        //case sdv::any_t::EValType::val_type_double:
        //case sdv::any_t::EValType::val_type_long_double:
        //case sdv::any_t::EValType::val_type_fixed:
        //    ptrNode = std::make_shared<CFloatingPointNode>(Parser(), ssName, anyValue.get<double>());
        //    break;
        //case sdv::any_t::EValType::val_type_string:
        //case sdv::any_t::EValType::val_type_u8string:
        //case sdv::any_t::EValType::val_type_u16string:
        //case sdv::any_t::EValType::val_type_u32string:
        //case sdv::any_t::EValType::val_type_wstring:
        //    ptrNode = std::make_shared<CStringNode>(Parser(), ssName, anyValue.get<sdv::u8string>());
        //    break;
        //case sdv::any_t::EValType::val_type_empty:
        //case sdv::any_t::EValType::val_type_char:
        //case sdv::any_t::EValType::val_type_char16:
        //case sdv::any_t::EValType::val_type_char32:
        //case sdv::any_t::EValType::val_type_wchar:
        //case sdv::any_t::EValType::val_type_interface:
        //case sdv::any_t::EValType::val_type_interface_id:
        //case sdv::any_t::EValType::val_type_exception_id:
        //default:
        //    return nullptr; // Not supported
        //}



        return {};
    }

    sdv::IInterfaceAccess* CNodeCollection::InsertArray(uint32_t /*uiIndex*/, const sdv::u8string& /*ssName*/)
    {
        return {};
    }

    sdv::IInterfaceAccess* CNodeCollection::InsertTable(uint32_t /*uiIndex*/, const sdv::u8string& /*ssKeyName*/)
    {
        // TODO: Do not insert a table if the parent is a table array. Only allow the use of InsertTableArray to insert table array
        // tables.
        return {};
    }

    sdv::IInterfaceAccess* CNodeCollection::InsertTableArray(uint32_t /*uiIndex*/, const sdv::u8string& /*ssName*/)
    {
        return {};
    }

    sdv::toml::INodeCollectionInsert::EInsertResult CNodeCollection::InsertTOML(const sdv::u8string& /*ssTOML*/, bool /*bRollbackOnPartly*/)
    {
        return {};
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

    bool CNodeCollection::RemoveNode(const std::shared_ptr<CNode>& rptrNode)
    {
        // Remove from the view
        RemoveFromView(rptrNode);

        // Find the element
        auto itElement = std::find(m_lstNodes.begin(), m_lstNodes.end(), rptrNode);
        if (itElement == m_lstNodes.end())
            return false;

        // Shift the node into the recycle bin.
        m_lstRecycleBin.push_back(rptrNode);
        m_lstNodes.erase(itElement);
        return true;
    }

    bool CNodeCollection::RemoveFromView(const std::shared_ptr<CNode>& rptrNode)
    {
        // Find the element and remove it from the view
        auto itElement = std::find(m_vecNodeOrder.begin(), m_vecNodeOrder.end(), rptrNode);
        if (itElement != m_vecNodeOrder.end())
        {
            m_vecNodeOrder.erase(itElement);
            return true;
        }
        return false;
    }

    bool CNodeCollection::CheckLast(const std::shared_ptr<CNode>& rptrNode)
    {
        // NOTE: Check for an empty vector. This is actually an error and should not occur.
        return m_vecNodeOrder.empty() || m_vecNodeOrder.back() == rptrNode; 
    }

    uint32_t CNodeCollection::FindIndex(const std::shared_ptr<CNode>& rptrNode)
    {
        // Find the element
        for (uint32_t uiIndex = 0; uiIndex < m_vecNodeOrder.size(); uiIndex++)
        {
            if (rptrNode == m_vecNodeOrder[uiIndex])
                return uiIndex;
        }
        return sdv::toml::npos;
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
        CGenContext rContextCopy = rContext;
        rContextCopy.InitTopMostNode(shared_from_this());

        std::stringstream sstream;

        // Determine whether the statement should be embedded (same line separated by commas) and should have an assignment (not
        // having a key name). The collection node is inline when it is defined as inline or is embedded into a parent node.
        auto ptrParent = GetParentPtr();
        bool bEmbedded = ptrParent ? ptrParent->Inline() : false;
        bool bAssignment = !bEmbedded || (ptrParent ? !ptrParent->Cast<CArray>() : false); // No assignment only with normal array
        bool bInline = bEmbedded || Inline();
        bool bLastNode = ptrParent ? ptrParent->CheckLast(std::const_pointer_cast<CNode>(shared_from_this())) : false;
        bool bRoot = dynamic_cast<const CRootTable*>(this) ? true : false;

        std::string ssContext;
        if (rContextCopy.KeyContext().empty())
        {
            ssContext = rContextCopy.PrefixKey();
            std::string ssGenContext = rContextCopy.TopMostNode() ?
                static_cast<std::string>(GetPath(false)) : std::string();
            if (!ssContext.empty() && !ssGenContext.empty())
                ssContext += ".";
            ssContext += ssGenContext;
        } else
            ssContext = rContextCopy.KeyContext();
         std::string ssKeyPath = rContextCopy.PrefixKey();
         std::string ssFullContext = ssContext.empty() ? rContextCopy.PrefixKey() : ssContext;
         std::string ssRelKeyPath =
            GetCustomPath(rContextCopy.PrefixKey(), ssContext.empty() ? rContextCopy.PrefixKey() : ssContext);
         if (!ssKeyPath.empty() && !ssRelKeyPath.empty())
            ssKeyPath += ".";
         if (!ssFullContext.empty() && !ssRelKeyPath.empty())
            ssFullContext += ".";
         ssKeyPath += ssRelKeyPath;
         ssFullContext += ssRelKeyPath;

        // Impossible option: suppressing the table name (due to root flag) and being embedded.
        if (bEmbedded && bRoot) return {};

        // Do not print explicit tables with only tables inside (except if they have no children at all or if they have one or more
        // inline tables).
        // Do not print table headers when this sis the top most table.
        bool bDoNotPrint = (rContext.TopMostNode() || (!Inline() && GetCount())) ? true : false;
        for (uint32_t uiIndex = 0; bDoNotPrint && uiIndex < GetCount(); uiIndex++)
        {
            std::shared_ptr<CNode> ptrNode = Get(uiIndex);
            if (!ptrNode) continue;
            if (!ptrNode->Cast<CTable>() || ptrNode->Inline())
                bDoNotPrint = false;
        }

        // Special case, table as part of table array - but only if the parent is included in the generation. This can be identified
        // by the top most flag of the context.
        bool bTableArray = false;
        if (rContextCopy.TopMostNode() == 0)
            bTableArray = !bEmbedded && ptrParent && ptrParent->Cast<CArray>() && ptrParent->Cast<CArray>()->TableArray();

        // Add unconnected pre node comments
        sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::out_of_scope_comment_before))
                       .Compose(CCodeSnippet::EComposeMode::compose_standalone);

        // Add pre node comments
        sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::comment_before))
                       .Compose(CCodeSnippet::EComposeMode::compose_before);

        // Do we need to start a table?
        if (!bDoNotPrint && !bInline && !ssKeyPath.empty())
        {
            if (bTableArray)
                sstream << "[[" << ssKeyPath << "]]";
            else
                sstream << "[" << ssKeyPath << "]";

            // Add post node comments
            sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::comment_behind)).
                Compose(bLastNode ? CCodeSnippet::EComposeMode::compose_inline : CCodeSnippet::EComposeMode::compose_behind);
        }

        // Stream the table
        if (bAssignment && bInline && !bDoNotPrint) // Not an array entry
            sstream << ssRelKeyPath << "=";
        if ((bEmbedded || bInline) && (!bDoNotPrint || ptrParent->Cast<CArray>())) // Embedded table in an array
            sstream << CodeSnippet(m_nPreValueCode).Compose(CCodeSnippet::EComposeMode::compose_inline) << "{";
        for (uint32_t uiIndex = 0; uiIndex < GetCount(); uiIndex++)
        {
            std::shared_ptr<CNode> ptrNode = Get(uiIndex);
            if (!ptrNode) continue;

            // If the node has a view pointer, which is not identical to this pointer, do not print the node... it will be printed
            // by a different node.
            if (!ptrNode->IsPartOfView(rContextCopy, Cast<CNodeCollection>())) continue;

            if (bInline || ptrNode->Inline())
            {
                // Inline nodes are only presented with a relative path (in most cases this is no path) for each node.
                // If the parent node is a table array, the do-not-print-flag is active (because this node is a table as part of
                // the array, which is used for data management and not for printing the node key) - use the full context for
                // printing.
                sstream <<
                    ptrNode->GenerateTOML(rContextCopy.CopyWithContext((bDoNotPrint && !bTableArray) ? ssContext : ssFullContext));
            }
            else
            {
                // Explicit collection nodes (tables and table-arrays) are presented with the complete path for each node.
                sstream << ptrNode->GenerateTOML(rContextCopy.CopyWithContext(ssContext));
            }
        }
        if ((!bDoNotPrint || (ptrParent && ptrParent->Cast<CArray>())) && (bEmbedded || bInline)) // Embedded table in an array
        {
            sstream << CodeSnippet(m_nPostValuesArray).Compose(CCodeSnippet::EComposeMode::compose_inline) << "}";

            // Add a comma if this is not the last node.
            if (bEmbedded && !bLastNode)
                sstream << ",";

            // Add post node comments
            sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::comment_behind)).
                Compose(bLastNode ? CCodeSnippet::EComposeMode::compose_inline : CCodeSnippet::EComposeMode::compose_behind);

        }

        // Add unconnected post node comments
        sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::out_of_scope_comment_behind))
                       .Compose(CCodeSnippet::EComposeMode::compose_standalone);

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
                CodeSnippet(m_nPreKeyCode, ssKeyName).List() = std::move(lstWhitespace);
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
                CodeSnippet(m_nPostKeyCode, ssKeyName).List() = std::move(lstWhitespace);
                break;
            case ETokenCategory::token_syntax_inline_table_open:
                if (eState != EState::key_or_value_or_table_def_open && eState != EState::table_open)
                    return; // Unexpected
                eState = EState::table_content_and_close;
                CodeSnippet(m_nPreValueCode).List() = std::move(lstWhitespace);
                break;
            case ETokenCategory::token_syntax_assignment:
                if (eState != EState::assignment_or_separator)
                    return; // Unexpected
                eState = EState::table_open;
                CodeSnippet(m_nPostKeyCode, ssKeyName).List() = std::move(lstWhitespace);
                break;
            case ETokenCategory::token_syntax_dot:
                if (eState != EState::assignment_or_separator && eState != EState::table_def_close_or_separator)
                    return; // Unexpected
                eState = EState::key;
                CodeSnippet(m_nPostKeyCode, ssKeyName).List() = std::move(lstWhitespace);
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

        CodeSnippet(m_nPostValueCode).List() = std::move(lstWhitespace);
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
        } else
            uiIndex = std::stoi(prKey.first);

        // Get the node
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
        CGenContext rContextCopy = rContext;
        rContextCopy.InitTopMostNode(shared_from_this());

        std::stringstream sstream;

        // Determine whether the statement should be embedded (same line separated by commas) and should have an assignment (not
        // having a key name).
        auto ptrParent = GetParentPtr();
        if (!ptrParent) return {};
        bool bEmbedded = ptrParent->Inline();
        bool bAssignment = !bEmbedded || !ptrParent->Cast<CArray>(); // No assignment only with normal array
        bool bInline = bEmbedded || Inline();
        bool bLastNode = ptrParent->CheckLast(std::const_pointer_cast<CNode>(shared_from_this()));

        std::string ssContext;
        if (rContextCopy.KeyContext().empty())
        {
            ssContext = rContextCopy.PrefixKey();
            std::string ssGenContext = rContextCopy.TopMostNode() ? 
                (TableArray() ? static_cast<std::string>(ptrParent->GetPath(false)) : static_cast<std::string>(GetPath(false))) :
                std::string();
            if (!ssContext.empty() && !ssGenContext.empty())
                ssContext += ".";
            ssContext += ssGenContext;
        }
        else
            ssContext = rContextCopy.KeyContext();
        std::string ssKeyPath     = rContextCopy.PrefixKey();
        std::string ssFullContext = ssContext.empty() ? rContextCopy.PrefixKey() : ssContext;
        std::string ssRelKeyPath =
            GetCustomPath(rContextCopy.PrefixKey(), ssContext.empty() ? rContextCopy.PrefixKey() : ssContext);
        if (!ssKeyPath.empty() && !ssRelKeyPath.empty())
            ssKeyPath += ".";
        if (!ssFullContext.empty() && !ssRelKeyPath.empty())
            ssFullContext += ".";
        ssKeyPath += ssRelKeyPath;
        ssFullContext += ssRelKeyPath;
        
        // Stream only for inline
        if (bInline)
        {
            // Add unconnected pre node comments
            sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::out_of_scope_comment_before))
                           .Compose(CCodeSnippet::EComposeMode::compose_standalone);

            // Add pre node comments
            sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::comment_before))
                           .Compose(CCodeSnippet::EComposeMode::compose_before);

            // Add assignment
            if (bAssignment) // Not an array entry
                sstream << ssRelKeyPath << "=";

            sstream << CodeSnippet(m_nPreValueCode).Compose(CCodeSnippet::EComposeMode::compose_inline) << "[";
        }

        // Stream the array content
        for (uint32_t ui = 0; ui < GetCount(); ui++)
        {
            std::shared_ptr<CNode> ptrNode = Get(ui);
            if (!ptrNode) continue;

            // If the node has a view pointer, which is not identical to this pointer, do not print the node... it will be printed
            // by a different node.
            if (!ptrNode->IsPartOfView(rContextCopy, Cast<CNodeCollection>()))
                continue;

            sstream << ptrNode->GenerateTOML(rContextCopy.CopyWithContext(bInline ? ssFullContext : ssContext));
        }

        // Stream only for inline
        if (bInline)
        {
            sstream << CodeSnippet(m_nPostValuesArray).Compose(CCodeSnippet::EComposeMode::compose_inline) << "]";

            // Add a comma if this is not the last node.
            if (bEmbedded && !bLastNode) sstream << ",";

            // Add post node comments
            sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::comment_behind)).
                Compose(bLastNode ? CCodeSnippet::EComposeMode::compose_inline : CCodeSnippet::EComposeMode::compose_behind);

            // Add unconnected post node comments
            sstream << CodeSnippet(static_cast<size_t>(sdv::toml::INodeInfo::ECommentFlags::out_of_scope_comment_behind))
                           .Compose(CCodeSnippet::EComposeMode::compose_standalone);
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
                CodeSnippet(m_nPreKeyCode, ssKeyName).List() = std::move(lstWhitespace);
                break;
            case ETokenCategory::token_syntax_array_open:
                if (eState != EState::key_or_value && eState != EState::array_open)
                    return; // Unexpected
                eState = EState::array_content_and_close;
                CodeSnippet(m_nPreValueCode).List() = std::move(lstWhitespace);
                break;
            case ETokenCategory::token_syntax_assignment:
                if (eState != EState::assignment_or_separator)
                    return; // Unexpected
                eState = EState::array_open;
                CodeSnippet(m_nPostKeyCode, ssKeyName).List() = std::move(lstWhitespace);
                break;
            case ETokenCategory::token_syntax_dot:
                if (eState != EState::assignment_or_separator)
                    return; // Unexpected
                eState = EState::key;
                CodeSnippet(m_nPostKeyCode, ssKeyName).List() = std::move(lstWhitespace);
                break;
            default:
                return; // Unexpected
            }
        }

        // Expecting array content 
        if (eState != EState::array_content_and_close)
            return; // Unexpected

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
                CodeSnippet(m_nPostValuesArray).List() = std::move(lstWhitespace);
                break;
            default:
                return; // Unexpected
            }
        }

        // Expecting the array to have been closed
        if (eState != EState::post_value)
            return;
        
        CodeSnippet(m_nPostValueCode).List() = std::move(lstWhitespace);
    }

    bool CArray::TableArray() const
    {
        // At least one table is needed
        if (!GetCount()) return false;

        // Iterate throught he child-nodes and if a non-table is found, return false.
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

    bool CArray::Inline(bool bInline)
    {
        // The array is inline per default. Only in the case of a table array (an array with only tables, and at least one table)
        // the array ould be explicit.
        if (bInline && !TableArray()) return false;
        m_bInline = bInline;
        return true;
    }

    bool CRootTable::DeleteNode()
    {
        // Cannot delete the root node.
        return false;
    }
} // namespace toml_parser