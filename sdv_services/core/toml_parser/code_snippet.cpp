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

#include "code_snippet.h"
#include "parser_node_toml.h"

/// The TOML parser namespace
namespace toml_parser
{
    void CCodeSnippet::SetTokenList(const std::list<CToken>& rlstTokens)
    {
        m_lstTokens = rlstTokens;
        m_bUseDefault = false;
    }

    void CCodeSnippet::SetTokenList(std::list<CToken>&& rlstTokens)
    {
        m_lstTokens = std::move(rlstTokens);
        m_bUseDefault = false;
    }

    std::string CCodeSnippet::Compose(EComposeMode eMode, const CGenContext& rContext, size_t nAssignmentOffset /*= 0*/,
        size_t nCommentOffset /*= 0*/) const
    {
        // Build the stream until the first comment.
        std::stringstream sstream;
        TTokenListIterator it = m_lstTokens.begin();
        bool bCommaAvailable  = false;
        bool bPrintableCharsAvailable = false;
        bool bLastTokenIsComment = false;
        while (it != m_lstTokens.end() && it->Category() != ETokenCategory::token_comment)
        {
            // Add comma only when needed.
            bool bSkip = false;
            switch (it->Category())
            {
            case ETokenCategory::token_syntax_comma:
                if (eMode != EComposeMode::compose_behind || !rContext.CommaNeeded() || bCommaAvailable)
                {
                    bSkip = true;
                    break; // Only when placed behind a node
                }
                bCommaAvailable = true;
                break;
            case ETokenCategory::token_comment:
                bSkip = !rContext.CommentAndNewlineAllowed();
                if (!bSkip) bLastTokenIsComment = true;
                break;
            case ETokenCategory::token_syntax_new_line:
                bSkip = !rContext.NewlineAllowed();
                if (!bSkip) bLastTokenIsComment = false;
                break;
            default:
                break;
            }
            if (!bSkip)
            {
                sstream << it->RawString();
                bPrintableCharsAvailable = true;
            }
            ++it;
        }

        // Determine the last comment.
        TTokenListIterator itFirstComment = it;
        TTokenListIterator itLastComment  = it;
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
        if (!m_bCommentReplaced)
        {
            for (it = itFirstComment; it != itPostComment; ++it)
            {
                // Add comma only when needed.
                bool bSkip = false;
                switch (it->Category())
                {
                case ETokenCategory::token_syntax_comma:
                    if (eMode != EComposeMode::compose_behind || !rContext.CommaNeeded() || bCommaAvailable)
                    {
                        bSkip = true;
                        break; // Only when placed behind a node
                    }
                    bCommaAvailable = true;
                    break;
                case ETokenCategory::token_comment:
                    bSkip = !rContext.CommentAndNewlineAllowed();
                if (!bSkip) bLastTokenIsComment = true;
                    break;
                case ETokenCategory::token_syntax_new_line:
                    bSkip = !rContext.NewlineAllowed();
                if (!bSkip) bLastTokenIsComment = false;
                    break;
                default:
                    break;
                }
                if (!bSkip)
                {
                    sstream << it->RawString();
                    bPrintableCharsAvailable = true;
                }
            }
        }
        else
        {
            switch (eMode)
            {
            case EComposeMode::compose_inline:
            case EComposeMode::compose_behind:
                if (itFirstComment == m_lstTokens.end() && !m_ssComment.empty())
                    sstream << " ";
                break;
            case EComposeMode::compose_standalone_behind:
                if (itFirstComment == m_lstTokens.end() && !m_ssComment.empty())
                    sstream << std::endl;
                break;
            default:
                break;
            }
            // TODO: Align the comment....
            size_t nPos = 0;
            bool bEmptyLine = false;
            while (nPos != std::string::npos)
            {
                // Find the chunk until the next newline or the end of the string
                size_t nNextPos = m_ssComment.find('\n', nPos);
                if (nNextPos != std::string::npos) nNextPos++;
                if (nNextPos >= m_ssComment.size()) nNextPos = std::string::npos;
                std::string ssChunk = m_ssComment.substr(nPos, nNextPos);
                nPos = nNextPos;

                // Insert an extra line break, but only when there isn't a line-break already...
                if (!bEmptyLine && !sstream.str().empty())
                {
                    switch (eMode)
                    {
                    case EComposeMode::compose_before:
                    case EComposeMode::compose_behind:
                    case EComposeMode::compose_inline:
                        sstream << std::string(nCommentOffset, ' ') << "#";
                        break;
                    default:
                        break;
                    }
                    sstream << std::endl;
                }

                if (ssChunk.find_first_not_of("\r\n") == std::string::npos)
                    bEmptyLine = true;
                else
                {
                    sstream << std::string(nCommentOffset, ' ') << "# " << ssChunk;
                    if (ssChunk.find('\n') == std::string::npos)
                        sstream << std::endl;   // Comment is always followed by a newline.
                    bEmptyLine = false;
                }
            }

            // TODO: Add spaces for next assignment
            switch (eMode)
            {
            case EComposeMode::compose_inline:
                if (itPostComment == m_lstTokens.end() && nAssignmentOffset)
                    sstream << std::string(nAssignmentOffset, ' ');
                break;
            case EComposeMode::compose_standalone_before:
                if (itFirstComment == m_lstTokens.end() && !m_ssComment.empty())
                    sstream << std::endl;
                break;
            default:
                break;
            }
        }

        // Stream the rest of the tokens
        for (it = itPostComment; it != m_lstTokens.end(); ++it)
        {
            bool bSkip = false;
            switch (it->Category())
            {
            case ETokenCategory::token_syntax_comma:
                if (eMode != EComposeMode::compose_behind || !rContext.CommaNeeded() || bCommaAvailable)
                {
                    bSkip = true;
                    break; // Only when placed behind a node
                }
                bCommaAvailable = true;
                break;
            case ETokenCategory::token_comment:
                bSkip = !rContext.CommentAndNewlineAllowed();
                if (!bSkip) bLastTokenIsComment = true;
                break;
            case ETokenCategory::token_syntax_new_line:
                bSkip = !rContext.NewlineAllowed();
                if (!bSkip) bLastTokenIsComment = false;
                break;
            default:
                break;
            }
            if (!bSkip)
            {
                sstream << it->RawString();
                bPrintableCharsAvailable = true;
            }
        }

        // Post processing
        switch (eMode)
        {
        case EComposeMode::compose_behind:
            // Was a comma provided?
            if (!bCommaAvailable && rContext.CommaNeeded())
            {
                std::string ssTemp = sstream.str();
                ssTemp.insert(0, bPrintableCharsAvailable ? "," : ", ");
                sstream.str(ssTemp);
            }

            // Default newline needed
            if (rContext.FinalNewline() && ((m_lstTokens.empty() && m_ssComment.empty()) || bLastTokenIsComment))
                sstream << std::endl;
            break;
        case EComposeMode::compose_before:
            //if (!bPrintableCharsAvailable && rContext.Embedded())
            //    sstream << " ";
            break;
        default:
            break;
        }

        return sstream.str();
    }

    std::string CCodeSnippet::GetComment() const
    {
        if (m_bCommentReplaced)
            return m_ssComment;

        std::stringstream sstream;
        size_t nNewLineCount = 0;
        for (const CToken& rToken : m_lstTokens)
        {
            // Deal with new line.
            if (rToken.Category() == ETokenCategory::token_syntax_new_line)
            {
                if (sstream.str().empty())
                    continue;
                nNewLineCount++;
                if (nNewLineCount < 2)
                    continue;
                sstream << std::endl;
                continue;
            }
            if (rToken.Category() == ETokenCategory::token_whitespace)
                continue;
            nNewLineCount = 0;
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
                if (rToken.RawString()[1] == '\t' || rToken.RawString().substr(1, 2) == "  "
                    || rToken.RawString().substr(1, 2) == " \t")
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
                        if (!std::isspace(sstream.str().back()))
                        sstream << " ";
                }

                // Add the content to the stream
                sstream << rToken.RawString().substr(nStartComment);
            }
        }

        // Trim space at the end of the string
        std::string ssResult = sstream.str();
        while (!ssResult.empty() && std::isspace(ssResult.back()))
            ssResult.pop_back();
        return ssResult;
    }

    void CCodeSnippet::SetComment(const std::string& rssComment)
    {
        // Just store the string. Further processing is done in the compose function.
        m_ssComment = rssComment;
        m_bCommentReplaced = true;
        m_bUseDefault = false;
    }

    void CCodeSnippet::Clear()
    {
        m_ssComment.clear();
        m_lstTokens.clear();
        m_bCommentReplaced = false;
        m_bUseDefault = true;
    }

    bool CCodeSnippet::HasCode() const
    {
        return !m_bUseDefault || (m_bCommentReplaced || !m_lstTokens.empty());
    }

    void CCodeSnippet::RemoveFormat()
    {
        // Read the comment from the token list.
        if (!m_bCommentReplaced && !m_lstTokens.empty())
        {
            m_ssComment = GetComment();
            m_bCommentReplaced = !m_ssComment.empty();
        }

        // Clear the token list
        m_lstTokens.clear();

        m_bUseDefault = !m_bCommentReplaced;
    }

    bool CCodeSnippet::HasComma() const
    {
        return !m_bCommentReplaced && (std::find_if(m_lstTokens.begin(), m_lstTokens.end(),
            [](const CToken& rToken) { return rToken.Category() == ETokenCategory::token_syntax_comma; }) != m_lstTokens.end());
    }

    void CCodeSnippet::Insert(const CCodeSnippet& rCode)
    {
        // Something to do?
        if (!rCode.HasCode()) return;

        // Simple assignment?
        if (!HasCode())
        {
            m_bCommentReplaced = rCode.m_bCommentReplaced;
            m_ssComment = rCode.m_ssComment;
            m_lstTokens = rCode.m_lstTokens;
            return;
        }

        // Differentiate between comment text or token lists.
        if (m_bCommentReplaced)
            m_ssComment.insert(0, rCode.GetComment());
        else
        {
            if (rCode.m_bCommentReplaced)
            {
                m_ssComment = rCode.GetComment() + GetComment();
                m_bCommentReplaced = true;
            }
            else
                m_lstTokens.insert(m_lstTokens.begin(), rCode.m_lstTokens.begin(), rCode.m_lstTokens.end());
        }
    }

    void CCodeSnippet::Append(const CCodeSnippet& rCode)
    {
        // Something to do?
        if (!rCode.HasCode()) return;

        // Simple assignment?
        if (!HasCode())
        {
            m_bCommentReplaced = rCode.m_bCommentReplaced;
            m_ssComment = rCode.m_ssComment;
            m_lstTokens = rCode.m_lstTokens;
            return;
        }

        // Differentiate between comment text or token lists.
        if (m_bCommentReplaced)
            m_ssComment.append(rCode.GetComment());
        else
        {
            if (rCode.m_bCommentReplaced)
            {
                m_ssComment = rCode.GetComment() + GetComment();
                m_bCommentReplaced = true;
            }
            else
                m_lstTokens.insert(m_lstTokens.end(), rCode.m_lstTokens.begin(), rCode.m_lstTokens.end());
        }
    }
} // namespace toml_parser