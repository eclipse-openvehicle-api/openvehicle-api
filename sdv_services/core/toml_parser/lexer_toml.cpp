#include "lexer_toml.h"
#include <interfaces/toml.h>
#include "exception.h"
#include <iterator>
#include "miscellaneous.h"

/// The TOML parser namespace
namespace toml_parser
{
    CNodeTokenRange::CNodeTokenRange(const CToken& rInitialToken) :
        m_refBeforeNodeBegin(rInitialToken), m_rangeExtendedNode(rInitialToken, rInitialToken),
        m_rangeNodeMain(rInitialToken, rInitialToken), m_rangeNodeFinish(rInitialToken, rInitialToken),
        m_refBehindNodeEnd(rInitialToken)
    {}

    CNodeTokenRange::CNodeTokenRange(const CTokenRange& rrangeNodeMain) :
        m_refBeforeNodeBegin(rrangeNodeMain.Begin()), m_rangeExtendedNode(rrangeNodeMain.Begin(), rrangeNodeMain.End()),
        m_rangeNodeMain(rrangeNodeMain), m_rangeNodeFinish(rrangeNodeMain.End(), rrangeNodeMain.End()),
        m_refBehindNodeEnd(rrangeNodeMain.End())
    {}

    void CNodeTokenRange::LinesBeforeNode(const CToken& rTokenBegin)
    {
        // Check if the begin is before or at the same location as the extended node begin.
        m_refBeforeNodeBegin = (rTokenBegin.TokenIndex() > m_rangeExtendedNode.Begin().TokenIndex()) ?
            m_rangeExtendedNode.Begin() : rTokenBegin;
    }
    
    CTokenRange CNodeTokenRange::LinesBeforeNode() const
    {
        return CTokenRange(m_refBeforeNodeBegin.get(), m_rangeExtendedNode.Begin());
    }

    void CNodeTokenRange::ExtendedNode(const CTokenRange& rRange)
    {
        // The begin is not allowed to start behind the end.
        if (rRange.Begin().TokenIndex() > rRange.End().TokenIndex()) return;

        // The extended node begin should start at or before the main node begin.
        const CToken& rTokenBegin = (rRange.Begin().TokenIndex() > m_rangeNodeMain.Begin().TokenIndex()) ?
            m_rangeNodeMain.Begin() : rRange.Begin();

        // The extended node end should start at or behind the main node end.
        const CToken& rTokenEnd = (rRange.End().TokenIndex() < m_rangeNodeMain.End().TokenIndex()) ?
            m_rangeNodeMain.End() : rRange.End();

        // Set the extended node.
        m_rangeExtendedNode = CTokenRange(rTokenBegin, rTokenEnd);

        // Check if the lines before the node start before or at the extended node.
        if (m_refBeforeNodeBegin.get().TokenIndex() > m_rangeExtendedNode.Begin().TokenIndex())
            m_refBeforeNodeBegin = m_rangeExtendedNode.Begin();

        // Check if the lines behind the node end behind or at the extended node.
        if (m_refBehindNodeEnd.get().TokenIndex() < m_rangeExtendedNode.End().TokenIndex())
            m_refBehindNodeEnd = m_rangeExtendedNode.End();
    }

    CTokenRange CNodeTokenRange::ExtendedNode() const
    {
        return m_rangeExtendedNode;
    }

    CTokenRange CNodeTokenRange::NodeCommentsBefore() const
    {
        return CTokenRange(m_rangeExtendedNode.Begin(), m_rangeNodeMain.Begin());
    }

    void CNodeTokenRange::NodeMain(const CTokenRange& rRange)
    {
        // The begin is not allowed to start behind the end.
        if (rRange.Begin().TokenIndex() > rRange.End().TokenIndex()) return;

        // Set the main node range
        m_rangeNodeMain = rRange;

        // Update of main node finish required?
        const CToken& rTokenBeginMainFinishRange = (m_rangeNodeFinish.Begin().TokenIndex() < m_rangeNodeMain.End().TokenIndex()) ?
            m_rangeNodeMain.End() : m_rangeNodeFinish.Begin();
        const CToken& rTokenEndMainFinishRange = (m_rangeNodeFinish.End().TokenIndex() < rTokenBeginMainFinishRange.TokenIndex()) ?
            rTokenBeginMainFinishRange : m_rangeNodeFinish.End();
        if (m_rangeNodeFinish.Begin() != rTokenBeginMainFinishRange || m_rangeNodeFinish.End() != rTokenEndMainFinishRange)
        {
            NodeMainFinish(CTokenRange(rTokenBeginMainFinishRange, rTokenEndMainFinishRange));

            // No need to check the extended range; this is done by the NodeMainFinish function.
            return;
        }

        // Check extended range begin before and end behind rRange.
        const CToken& rTokenBeginExtendedRange = (m_rangeExtendedNode.Begin().TokenIndex() > m_rangeNodeMain.Begin().TokenIndex()) ?
            m_rangeNodeMain.Begin() : m_rangeExtendedNode.Begin();
        const CToken& rTokenEndExtendedRange = (m_rangeExtendedNode.End().TokenIndex() < m_rangeNodeFinish.End().TokenIndex()) ?
            m_rangeNodeMain.End() : m_rangeExtendedNode.End();
        if (m_rangeExtendedNode.Begin() != rTokenBeginExtendedRange || m_rangeExtendedNode.End() != rTokenEndExtendedRange)
            ExtendedNode(CTokenRange(rTokenBeginExtendedRange, rTokenEndExtendedRange));
    }

    CTokenRange CNodeTokenRange::NodeMain() const
    {
        return m_rangeNodeMain;
    }

    void CNodeTokenRange::NodeMainFinish(const CTokenRange& rRange)
    {
        // The begin is not allowed to start behind the end.
        if (rRange.Begin().TokenIndex() > rRange.End().TokenIndex()) return;

        // Update of main node finish if required.
        const CToken& rTokenBeginMainFinishRange = (rRange.Begin().TokenIndex() < m_rangeNodeMain.End().TokenIndex()) ?
            m_rangeNodeMain.End() : rRange.Begin();
        const CToken& rTokenEndMainFinishRange = (rRange.End().TokenIndex() < rTokenBeginMainFinishRange.TokenIndex()) ?
            rTokenBeginMainFinishRange : rRange.End();

        // Set the main node finish range
        m_rangeNodeFinish = CTokenRange(rTokenBeginMainFinishRange, rTokenEndMainFinishRange);

        // Check extended range begin before and end behind rRange.
        const CToken& rTokenBeginExtendedRange = (m_rangeExtendedNode.Begin().TokenIndex() > m_rangeNodeMain.Begin().TokenIndex()) ?
            m_rangeNodeMain.Begin() : m_rangeExtendedNode.Begin();
        const CToken& rTokenEndExtendedRange = (m_rangeExtendedNode.End().TokenIndex() < m_rangeNodeFinish.End().TokenIndex()) ?
            m_rangeNodeMain.End() : m_rangeExtendedNode.End();
        if (m_rangeExtendedNode.Begin() != rTokenBeginExtendedRange || m_rangeExtendedNode.End() != rTokenEndExtendedRange)
            ExtendedNode(CTokenRange(rTokenBeginExtendedRange, rTokenEndExtendedRange));
    }

    CTokenRange CNodeTokenRange::NodeMainFinish() const
    {
        return m_rangeNodeFinish;
    }

    CTokenRange CNodeTokenRange::NodeCommentsBehind() const
    {
        return CTokenRange(m_rangeNodeFinish.End(), m_rangeExtendedNode.End());
    }

    void CNodeTokenRange::LinesBehindNode(const CToken& rTokenEnd)
    {
        // Check if the end is behind or at the same location as the extended node end.
        m_refBehindNodeEnd =
            (rTokenEnd.TokenIndex() < m_rangeExtendedNode.End().TokenIndex()) ? m_rangeExtendedNode.End() : rTokenEnd;
    }

    CTokenRange CNodeTokenRange::LinesBehindNode() const
    {
        return CTokenRange(m_rangeExtendedNode.End(), m_refBehindNodeEnd.get());
    }

    CLexer::CLexer(const std::string& rssString, bool bValueOnly /*= false*/)
    {
        Feed(rssString, bValueOnly);
    }

    void CLexer::Feed(const std::string& rssString, bool bValueOnly /*= false*/)
    {
        m_lstTokens.clear();
        m_itCursor = m_lstTokens.end();
        CCharacterReaderUTF8 reader;
        try
        {
            reader.Feed(rssString);
            if (bValueOnly)
                m_stackExpectations.push(EExpectation::expect_value);
            else
                m_stackExpectations.push(EExpectation::expect_key);
            GenerateTokens(reader);
            m_itCursor = m_lstTokens.begin();
        }
        catch (const sdv::toml::XTOMLParseException& rexcept)
        {
            CToken tokenTerminated(ETokenCategory::token_terminated, rexcept);
            TTokenListIterator itLocation = m_lstTokens.insert(m_lstTokens.end(), tokenTerminated);
            m_lstTokens.back().RawDataInfo(reader.StringFromBookmark(), m_lstTokens, itLocation);
            m_itCursor = m_lstTokens.begin();
            throw;
        }
    }

    void CLexer::Reset()
    {
        m_itCursor = m_lstTokens.begin();
    }

    CLexer::ENavigationMode CLexer::NavigationMode() const
    {
        return m_eNavMode;
    }

    void CLexer::NavigationMode(ENavigationMode eMode)
    {
        m_eNavMode = eMode;
    }

    const CToken& CLexer::Peek(size_t nSkip) const
    {
        size_t nInternalSkip = nSkip;
        size_t nOffset = 0;
        TTokenListIterator itCursorTemp = m_itCursor;
        TTokenListIterator itCurrent = m_itCursor;

        do
        {
            // Current value is the one to return.
            itCurrent = itCursorTemp;

            // If past the end of the token list, return EOF
            if (itCursorTemp == m_lstTokens.end())
                break;
            if (!*itCursorTemp)
                break;

            // Skip whitespace and newlines.
            switch (itCursorTemp->Category())
            {
            case ETokenCategory::token_whitespace:
            case ETokenCategory::token_comment:
                if (m_eNavMode == ENavigationMode::skip_comments_and_whitespace)
                    nInternalSkip++;
                break;
            default:
                break;
            }

            // Increase current position
            ++itCursorTemp;
            ++nOffset;
        } while (nOffset <= nInternalSkip);

        if (itCurrent == m_lstTokens.end())
            return m_lstTokens.tokenEnd;
        return *itCurrent;
    }

    const CToken& CLexer::Consume(size_t nSkip)
    {
        size_t nOffset = 0;
        size_t nInternalSkip = nSkip;
        TTokenListIterator itCurrent = m_itCursor;

        do
        {
            // Current value is the one to return.
            itCurrent = m_itCursor;

            // If past the end of the token list, return EOF
            if (m_itCursor == m_lstTokens.end()) break;
            if (!*m_itCursor) break;

            // Skip whitespace and newlines.
            switch (m_itCursor->Category())
            {
            case ETokenCategory::token_whitespace:
            case ETokenCategory::token_comment:
                if (m_eNavMode == ENavigationMode::skip_comments_and_whitespace)
                    nInternalSkip++;
                break;
            default:
                break;
            }

            // Increase current position
            ++m_itCursor;
            ++nOffset;
        } while (nOffset <= nInternalSkip);

        if (itCurrent == m_lstTokens.end())
            return m_lstTokens.tokenEnd;
        return *itCurrent;
    }

    bool CLexer::IsEnd() const
    {
        return m_itCursor == m_lstTokens.end();
    }

    void CLexer::SmartExtendNodeRange(CNodeTokenRange& rTokenRange) const
    {
        // Define an iterator range type.
        using TRange = std::pair<TTokenListIterator, TTokenListIterator>;

        // Get the first token in the line (the begin token in the list or the one following a newline) and one past the last token
        // in the line (being the end token in the list or the first token of the next line).
        // remarks No validity check is done for the supplied token iterator.
        // param[in] rit Reference to the token list iterator marking the current position within the line to return.
        // return A pair containing the begin and one past the end of the line.
        auto fnGetLine = [this](const TTokenListIterator& rit) -> TRange
        {
            // When the end of the list is reached, there is no more line.
            if (rit == m_lstTokens.end())
                return std::make_pair(m_lstTokens.end(), m_lstTokens.end());

            // Search for the start of the line
            TTokenListIterator itBegin = rit;
            while (itBegin != m_lstTokens.begin())
            {
                TTokenListIterator itOneBefore = itBegin;
                --itOneBefore;
                if (itOneBefore->Category() == ETokenCategory::token_syntax_new_line)
                    break;
                itBegin = itOneBefore;
            }

            // Search for the end of the line (and return one past the end).
            TTokenListIterator itEnd = rit;
            while (itEnd != m_lstTokens.end()) {
                TTokenListIterator itTempEnd = itEnd;
                ++itEnd;
                if (itTempEnd->Category() == ETokenCategory::token_syntax_new_line)
                    break;
            }
            return std::make_pair(itBegin, itEnd);
        };

        // Get the previous line.
        // remarks No validity check is done for the supplied token iterator.
        // param[in, out] rprLine Reference to a pair with the start position and one past the end position of a line. Will be
        // updated when there is a previous line.
        // return Returns whether there is a previous line or not.
        auto fnGetPrevLine = [this, fnGetLine](TRange& rprLine) -> bool
        {
            if (rprLine.first == m_lstTokens.begin())
                return false;
            auto itPrev = rprLine.first;
            --itPrev;
            rprLine = fnGetLine(itPrev);
            return true;
        };

        // Get the next line.
        // remarks No validity check is done for the supplied token iterator.
        // param[in, out] rprLine Reference to a pair with the start position and one past the end position of a line. Will be
        // updated when there is a next line.
        // return Returns whether there is a next line or not.
        auto fnGetNextLine = [this, fnGetLine](TRange& rprLine) -> bool
        {
            if (rprLine.second == m_lstTokens.end())
                return false;
            rprLine = fnGetLine(rprLine.second);
            return true;
        };

        // Determine the length (in characters) of the whitespace based on spaces and tabs.
        // remarks No validity check is done for the supplied token iterator.
        // param[in] rprRange Pair with the start position and one past the end position of a range.
        // return The length of the whitespace in characters (tabs can contain up to 4 spaces).
        auto fnGetIndentation = [this](const TRange& rprRange) -> size_t
        {
            if (rprRange.first == m_lstTokens.end() || rprRange.first->Category() != ETokenCategory::token_whitespace)
                return 0;
            std::string ssWhitespace = rprRange.first->RawString();
            size_t nLen = 0;
            for (char c : ssWhitespace)
            {
                switch (c)
                {
                case ' ':
                    nLen++;
                    break;
                case '\t':
                    nLen += (nLen % 4) ? (4 - nLen % 4) : 4;
                    break;
                default:
                    return nLen;
                }
            }
            return nLen;
        };

        // Check a range for comments only (ignore whitespace and newlines).
        // remarks No validity check is done for the supplied token iterator.
        // param[in] rprRange Pair with the start position and one past the end position of a range.
        // return Returns whether the line contains comments (and optionally whitespace and newlines) only.
        auto fnCommentsOnly = [this](const TRange& rprRange) -> bool
        {
            auto itToken = rprRange.first;
            bool bComments = false;
            while (itToken != rprRange.second)
            {
                switch (itToken->Category())
                {
                case ETokenCategory::token_comment:
                    bComments = true;
                    break;
                case ETokenCategory::token_whitespace:
                case ETokenCategory::token_syntax_new_line:
                    break;
                default:
                    return false;
                }
                ++itToken;
            }
            return bComments;
        };

        // Check a range for empty line(s) only (ignore whitespace and newlines).
        // remarks No validity check is done for the supplied token iterator.
        // param[in] rprRange Pair with the start position and one past the end position of a range.
        // return Returns whether the line is (lines are) empty.
        auto fnEmptyLine = [this](const TRange& rprRange) -> bool
        {
            auto itToken   = rprRange.first;
            while (itToken != rprRange.second)
            {
                switch (itToken->Category())
                {
                case ETokenCategory::token_whitespace:
                case ETokenCategory::token_syntax_new_line:
                    break;
                default:
                    return false;
                }
                ++itToken;
            }
            return true;
        };

        // Skip certain tokens.
        // param[in] ritToken Reference to the token to be updated.
        // param[in] eTokenCategory Category of the token to skip (if present).
        auto fnSkipToken = [this](TTokenListIterator& ritToken, toml_parser::ETokenCategory eTokenCategory) -> bool
        {
            if (ritToken != m_lstTokens.end() && *ritToken && ritToken->Category() == eTokenCategory)
            {
                ritToken++;
                return true;
            }
            return false;
        };

        // Get a token from the iterator.
        // param[in] rit Reference to the iterator.
        // return Reference to the token, either being an empty token when the iterator is pointer to the end of the list, or the
        // token that is pointer to by the iterator.
        auto fnToken = [this](const TTokenListIterator& rit) -> const CToken&
        {
            return rit == m_lstTokens.end() ? m_lstTokens.tokenEnd : *rit;
        };

        // Is a token range supplied?
        if (!rTokenRange.NodeMain().Begin() || !rTokenRange.NodeMain().Begin().Location())
            throw XTOMLParseException("No token range was supplied to check for boundaries.");

        // Check whether the begin token (and if set, the end token automatically as well) are present in the token list of this
        // lexer.
        if (&(*rTokenRange.NodeMain().Begin().TokenList()).get() != &m_lstTokens)
            throw XTOMLParseException("The tokens in the token range were not provided by this lexer.");

        // Get the initial iterator.
        auto itTokenBegin = *rTokenRange.NodeMain().Begin().Location();

        // Check whether there is a final iterator.
        auto itTokenEnd = *rTokenRange.NodeMainFinish().End().Location();

        // Following whitespace belongs to the extended range
        fnSkipToken(itTokenEnd, toml_parser::ETokenCategory::token_whitespace);

        // If the following token is a comma, this belongs to the extended token (then the token is part of an array or table).
        // If the following token is a dot, this belongs to the extended token (then the token is part of a table). The comments
        // before belong to a child member and should not be used. Also table and table array syntax tokens do not belong to the
        // statement.
        fnSkipToken(itTokenEnd, toml_parser::ETokenCategory::token_syntax_comma);
        bool bIsParent = fnSkipToken(itTokenEnd, toml_parser::ETokenCategory::token_syntax_dot);

        // The key could be part of a composed table name. Check for keys separated by dots prepending the key. Check for preceeding
        // comments.
        enum class EPrepending
        {
            expected_dot_or_comma,
            expected_dot,
            expected_key,
            done
        } ePrepending = EPrepending::expected_dot_or_comma;
        auto itPrepending = itTokenBegin;
        auto itPrependingComment = itTokenBegin;
        bool bRelevantNewlineBefore = false;
        bool bCommentFound = false;
        while (itPrepending != m_lstTokens.begin() && ePrepending != EPrepending::done)
        {
            itPrepending--;
            switch (itPrepending->Category())
            {
            case toml_parser::ETokenCategory::token_syntax_dot:
                // Dot okay; need a preceeding key
                if (ePrepending == EPrepending::expected_dot || ePrepending == EPrepending::expected_dot_or_comma)
                    ePrepending = EPrepending::expected_key;
                else
                    ePrepending = EPrepending::done;
                break;
            case toml_parser::ETokenCategory::token_key: // Key preceeding a dot; this belongs to the statement.
                if (ePrepending == EPrepending::expected_key)
                {
                    itTokenBegin = itPrepending;
                    ePrepending = EPrepending::expected_dot;
                }
                else
                    ePrepending = EPrepending::done;
                break;
            case toml_parser::ETokenCategory::token_whitespace:
                if (bCommentFound)
                {
                    bCommentFound = false;
                    ePrepending   = EPrepending::done;
                } else if (!bRelevantNewlineBefore)
                    itTokenBegin = itPrepending;
                break;
            case toml_parser::ETokenCategory::token_syntax_new_line:
                if (bRelevantNewlineBefore || bIsParent)
                    ePrepending = EPrepending::done;
                else
                {
                    bRelevantNewlineBefore = true;
                    if (bCommentFound)
                        itTokenBegin = itPrependingComment;
                }
                bCommentFound = false;
                break;
            case toml_parser::ETokenCategory::token_comment:
                itPrependingComment = itPrepending;
                bRelevantNewlineBefore = false;
                bCommentFound = true;
                break; // Ignore
            default:
                ePrepending = EPrepending::done;
                bCommentFound = false;
                break;
            }
        }
        if (bCommentFound) itTokenBegin = itPrependingComment;

        // The beginning and the end cannot be the same, except for an empty range.
        if (itTokenBegin == itTokenEnd) return;

        // Determine line boundaries of the current range
        auto itTokenBeginLine = itTokenBegin == m_lstTokens.begin() ? itTokenBegin : fnGetLine(itTokenBegin).first;
        auto itTokenEndLine = itTokenEnd;
        --itTokenEndLine;
        itTokenEndLine = fnGetLine(itTokenEndLine).second;

        // Are the comments following the statement?
        bool bCommentsSameLine = fnCommentsOnly(std::make_pair(itTokenEnd, itTokenEndLine));

        // Is there only whitespace before or after the statement
        bool bWhitespaceBefore = fnEmptyLine(std::make_pair(itTokenBeginLine, itTokenBegin)) && !bIsParent;
        bool bWhitespaceAfter = fnEmptyLine(std::make_pair(itTokenEnd, itTokenEndLine));

        // Get the indentation length of the line (whether there is other comments before or not).
        size_t nIndentLen = fnGetIndentation(std::make_pair(itTokenBeginLine, itTokenEnd));

        // Extend the range to include the beginning of the line and the end of the line.
        TRange prExtended = std::make_pair(bWhitespaceBefore ? itTokenBeginLine : itTokenBegin,
            (bCommentsSameLine || bWhitespaceAfter) ? itTokenEndLine : itTokenEnd);

        // Deal with whitespace and optionally comments before
        TRange prLine = prExtended;
        bool bEmptyLineBefore = false; 
        if (bWhitespaceBefore)
        {
            // Check for comments preceeding the range. If previous lines are having comments only (and optionally whitespace), and
            // the indentation is identical or less than the indentation of the range, the comment belongs to the range.
            while (fnGetPrevLine(prLine) && fnCommentsOnly(prLine) && fnGetIndentation(prLine) <= nIndentLen)
                prExtended.first = prLine.first;

            // Check whether there is an empty line before the range or the range starts at the first token in the list.
            bEmptyLineBefore = prLine.first != prExtended.first ? fnEmptyLine(prLine) : prLine.first == m_lstTokens.begin();
        }

        // Deal with whitespace and optionally comments following
        if (bWhitespaceAfter)
        {
            // Check for comments following the range. But only when there are comments at the same line and the indentation of the
            // next line is larger than the range indentation or there is an empty line following.
            TRange prPotential = prExtended;
            bool bUsePotential = false;
            prLine = prExtended;
            while (bCommentsSameLine && fnGetNextLine(prLine) && fnCommentsOnly(prLine))
            {
                if (bUsePotential || fnGetIndentation(prLine) > nIndentLen)
                {
                    bUsePotential = true;
                    prPotential.second = prLine.second;
                }
                else
                    prExtended.second = prLine.second;
            }

            // Check whether there is an empty line following the range or the range ends at the end of the list.
            bool bEmptyLineBeyond = prLine.second == m_lstTokens.end() ? true : fnEmptyLine(prLine);

            // If an empty line is following and a potential extension was detected, extend the range
            if (bEmptyLineBeyond && bUsePotential)
                prExtended.second = prPotential.second;

            // If there is an empty line before, include any empty lines until the next token or the end of the list.
            while (bEmptyLineBefore && bEmptyLineBeyond && fnGetNextLine(prLine) && fnEmptyLine(prLine))
                prExtended.second = prLine.second;
        }

        rTokenRange.ExtendedNode(CTokenRange(fnToken(prExtended.first), fnToken(prExtended.second)));

        // Determine if there are any more nodes before the end of the list.
        auto itFinal = prExtended.second;
        bool bNextNodeFound = false;
        while (itFinal != m_lstTokens.end() && !bNextNodeFound)
        {
            switch (itFinal->Category())
            {
            case ETokenCategory::token_comment:
            case ETokenCategory::token_whitespace:
            case ETokenCategory::token_syntax_new_line:
                break;
            default:
                bNextNodeFound = true;
                break;
            }

            // Next token
            itFinal++;
        }
        if (!bNextNodeFound) // There is no following node. Include the code behind the node
            rTokenRange.LinesBehindNode(fnToken(m_lstTokens.end()));
    }

    void CLexer::GenerateTokens(CCharacterReaderUTF8& rReader)
    {
        while (!rReader.IsEOF())
        {
            rReader.SetBookmark();
            CToken token = ReadBareKey(rReader);
            if (!token) token = ReadBasicQuotedKey(rReader);
            if (!token) token = ReadLiteralQuotedKey(rReader);
            if (!token) token = ReadBasicMultilineString(rReader);
            if (!token) token = ReadBasicString(rReader);
            if (!token) token = ReadLiteralMultilineString(rReader);
            if (!token) token = ReadLiteralString(rReader);
            if (!token) token = ReadInteger(rReader);
            if (!token) token = ReadFloat(rReader);
            if (!token) token = ReadBool(rReader);
            if (!token) token = ReadWhitespace(rReader);
            if (!token) token = ReadSyntaxElement(rReader);
            if (!token) token = ReadComment(rReader);
            if (!token)
                token = ReadUnknownSequence(rReader);
            auto itLocation = m_lstTokens.insert(m_lstTokens.end(), token);
            m_lstTokens.back().RawDataInfo(rReader.StringFromBookmark(), m_lstTokens, itLocation);
        }
    }

    CToken CLexer::ReadBasicQuotedKey(CCharacterReaderUTF8& rReader) const
    {
        if (m_stackExpectations.top() != EExpectation::expect_key || rReader.Peek() != "\"") return {};
        try
        {
            bool bEndOfQuote = false;
            std::string ssContent;
            rReader.Consume();
            while (!bEndOfQuote)
            {
                if (rReader.IsEOF())
                    throw XTOMLParseException(
                        "Unexpected End of File reached while reading multiline string");
                std::string ssCharacter = rReader.Consume();
                if (ssCharacter == "\\")
                    ssContent += Unescape(rReader);
                else if (ssCharacter == "\"")
                    bEndOfQuote = true;
                else
                    ssContent += ssCharacter;
            }
            return CToken(ETokenCategory::token_key, ssContent, ETokenStringType::quoted_string);
        }
        catch (const sdv::toml::XTOMLParseException& rexcept)
        {
            return CToken(ETokenCategory::token_error, rexcept);
        }
    }

    CToken CLexer::ReadLiteralQuotedKey(CCharacterReaderUTF8& rReader) const
    {
        if (m_stackExpectations.top() != EExpectation::expect_key || rReader.Peek() != "\'") return {};
        bool bEndOfQuote = false;
        std::string ssContent;
        rReader.Consume(); // get the initial "'"
        while (!bEndOfQuote)
        {
            std::string ssCharacter = rReader.Consume();
            if (ssCharacter == "\'")
                bEndOfQuote = true;
            else
                ssContent += ssCharacter;
        }
        return CToken(ETokenCategory::token_key, ssContent, ETokenStringType::literal_string);
    }

    CToken CLexer::ReadBareKey(CCharacterReaderUTF8& rReader) const
    {
        std::string ssCharacter = rReader.Peek();
        if (m_stackExpectations.top() != EExpectation::expect_key || ssCharacter.empty() ||
            ((ssCharacter[0] < 'A' || ssCharacter[0] > 'Z') && (ssCharacter[0] < 'a' || ssCharacter[0] > 'z')
                && (ssCharacter[0] < '0' || ssCharacter[0] > '9') && ssCharacter[0] != '-' && ssCharacter[0] != '_'))
            return {};

        bool bEndOfKey = false;
        std::string ssContent;
        bool bError = false;
        while (!bEndOfKey)
        {
            ssCharacter = rReader.Peek();
            if (!ssCharacter.empty() && ssCharacter[0] != '.' && ssCharacter[0] != '=' && ssCharacter[0] != ' '
                && ssCharacter[0] != '\t' && ssCharacter[0] != ']')
            {
                if ((ssCharacter[0] < '0' || ssCharacter[0] > '9') && (ssCharacter[0] < 'A' || ssCharacter[0] > 'Z')
                    && (ssCharacter[0] < 'a' || ssCharacter[0] > 'z') && (ssCharacter[0] != '_') && (ssCharacter[0] != '-'))
                    bError = true;
                rReader.Consume();
                ssContent += ssCharacter;
            }
            else
                bEndOfKey = true;
        }
        if (bError)
            return CToken(ETokenCategory::token_error, "Invalid bare key '" + ssContent + "'");
        else
            return CToken(ETokenCategory::token_key, ssContent);
    }

    CToken CLexer::ReadBasicString(CCharacterReaderUTF8& rReader)
    {
        if (m_stackExpectations.top() == EExpectation::expect_key || rReader.Peek() != "\"" || 
            (rReader.Peek(1) == "\"" && rReader.Peek(2) == "\""))
            return {};

        CToken token;
        try
        {
            bool bEndOfQuote = false;
            std::string ssContent;
            rReader.Consume();
            while (!bEndOfQuote)
            {
                if (rReader.IsEOF())
                    throw XTOMLParseException("Unexpected End of File reached while reading string");
                std::string ssCharacter = rReader.Consume();
                if (ssCharacter == "\\")
                    ssContent += Unescape(rReader);
                else if (ssCharacter == "\"")
                    bEndOfQuote = true;
                else
                    ssContent += ssCharacter;
            }
            token = CToken(ETokenCategory::token_string, ssContent, ETokenStringType::quoted_string);
        }
        catch (const sdv::toml::XTOMLParseException& rexcept)
        {
            token = CToken(ETokenCategory::token_error, rexcept);
        }
        if (m_stackExpectations.top() == EExpectation::expect_value_once)
            m_stackExpectations.pop();
        return token;
    }

    CToken CLexer::ReadBasicMultilineString(CCharacterReaderUTF8& rReader)
    {
        if (m_stackExpectations.top() == EExpectation::expect_key || rReader.Peek() != "\"" || rReader.Peek(1) != "\""
            || rReader.Peek(2) != "\"")
            return {};

        std::string ssContent;
        bool bEndOfQuote = false;

        auto fnIgnoreNewLineAtTheBeginning = [&rReader]()
        {
            if (rReader.Peek() == "\n")
                rReader.Consume();
            else if (rReader.Peek() == "\r" && rReader.Peek(1) == "\n")
                rReader.Consume(1);
        };
        auto fnHandleBackslashFunctionality = [this, &rReader, &ssContent]()
        {
            if (rReader.Peek() == "\n" || (rReader.Peek() == "\r" && rReader.Peek(1) == "\n"))
            {
                std::string next = rReader.Peek();
                while (next == "\n" || (next == "\r" && rReader.Peek(1) == "\n") || next == " " || next == "\t")
                {
                    rReader.Consume();
                    next = rReader.Peek();
                }
                return;
            }
            ssContent += Unescape(rReader);
        };
        auto fnHandleTrippleDoublequotes = [this, &rReader, &ssContent, &bEndOfQuote]()
        {
            if (rReader.Peek(2) == "\"")
            {
                ssContent += '\"';
                return;
            }
            bEndOfQuote = true;
            rReader.Consume(1);
        };

        CToken token;
        try
        {
            rReader.Consume(2);
            fnIgnoreNewLineAtTheBeginning();
            while (!bEndOfQuote)
            {
                if (rReader.IsEOF())
                    throw XTOMLParseException("Unexpected End of File reached while reading multiline string");
                std::string ssCharacter = rReader.Consume();
                if (ssCharacter == "\\")
                    fnHandleBackslashFunctionality();
                else if (ssCharacter == "\"" && rReader.Peek() == "\"" && rReader.Peek(1) == "\"")
                    fnHandleTrippleDoublequotes();
                else
                    ssContent += ssCharacter;
            }
            token = CToken(ETokenCategory::token_string, ssContent, ETokenStringType::multi_line_quoted);
        }
        catch (const sdv::toml::XTOMLParseException& rexcept)
        {
            token = CToken(ETokenCategory::token_error, rexcept);
        }
        if (m_stackExpectations.top() == EExpectation::expect_value_once)
            m_stackExpectations.pop();
        return token;
    }

    CToken CLexer::ReadLiteralString(CCharacterReaderUTF8& rReader)
    {
        if (m_stackExpectations.top() == EExpectation::expect_key || rReader.Peek() != "\'"
            || (rReader.Peek(1) == "\'" && rReader.Peek(2) == "\'"))
            return {};

        CToken token;
        try
        {
            bool bEndOfQuote{false};
            std::string ssContent;
            rReader.Consume();
            while (!bEndOfQuote)
            {
                if (rReader.IsEOF())
                    throw XTOMLParseException("Unexpected End of File reached while reading string");
                std::string ssCharacter = rReader.Consume();
                if (ssCharacter == "'")
                    bEndOfQuote = true;
                else
                    ssContent += ssCharacter;
            }
            token = CToken(ETokenCategory::token_string, ssContent, ETokenStringType::literal_string);
        }
        catch (const sdv::toml::XTOMLParseException& rexcept)
        {
            token = CToken(ETokenCategory::token_error, rexcept);
        }
        if (m_stackExpectations.top() == EExpectation::expect_value_once)
            m_stackExpectations.pop();
        return token;
    }

    CToken CLexer::ReadLiteralMultilineString(CCharacterReaderUTF8& rReader)
    {
        if (m_stackExpectations.top() == EExpectation::expect_key || rReader.Peek() != "\'" || rReader.Peek(1) != "\'"
            || rReader.Peek(2) != "\'")
            return {};

        CToken token;
        try
        {
            bool bEndOfQuote = false;
            std::string ssContent;
            rReader.Consume(2);
            if (rReader.Peek() == "\n")
                rReader.Consume();
            else if (rReader.Peek() == "\r" && rReader.Peek(1) == "\n")
                rReader.Consume(1);
            bool bConsumeWhitespace = false;
            while (!bEndOfQuote)
            {
                if (rReader.IsEOF())
                    throw XTOMLParseException("Unexpected End of File reached while reading multiline string");
                std::string ssCharacter = rReader.Consume();
                if (ssCharacter == "'" && rReader.Peek() == "'" && rReader.Peek(1) == "'")
                {
                    if (rReader.Peek(2) == "'")
                        ssContent += '\'';
                    else
                    {
                        bEndOfQuote = true;
                        rReader.Consume(1);
                    }
                }
                else if (ssCharacter == "\\" && rReader.Peek() == "\n")
                {
                    bConsumeWhitespace = true;
                    rReader.Consume();
                }
                else if (ssCharacter == "\\" && rReader.Peek() == "\r" && rReader.Peek(1) == "\n")
                {
                    bConsumeWhitespace = true;
                    rReader.Consume();
                }
                else
                {
                    if (!std::isspace(ssCharacter[0]) || !bConsumeWhitespace)
                    {
                        ssContent += ssCharacter;
                        bConsumeWhitespace = false;
                    }
                }
            }
            token = CToken(ETokenCategory::token_string, ssContent, ETokenStringType::multi_line_literal);
        }
        catch (const sdv::toml::XTOMLParseException& rexcept)
        {
            token = CToken(ETokenCategory::token_error, rexcept);
        }
        if (m_stackExpectations.top() == EExpectation::expect_value_once)
            m_stackExpectations.pop();
        return token;
    }

    CToken CLexer::ReadInteger(CCharacterReaderUTF8& rReader)
    {
        if (m_stackExpectations.top() == EExpectation::expect_key)
            return {};
        std::string ssCharacter = rReader.PeekUntil(m_vecValueDelimiters);
        if (ssCharacter.empty()) return {};
        if (ssCharacter[0] != '-' && ssCharacter[0] != '+' && !std::isdigit(ssCharacter[0]))
            return {};  // Is not a number
        if (ssCharacter.substr(0, 2) != "0x" && ssCharacter.find_first_of("eE.") != std::string::npos)
            return {}; // Is float
        if ((ssCharacter[0] == '-' || ssCharacter[0] == '+') && ssCharacter.size() > 1 && !std::isdigit(ssCharacter[1]))
            return {}; // Likely float

        std::string ssIntegerString = rReader.ConsumeUntil(m_vecValueDelimiters);

        enum class EEncoding : int64_t
        {
            Decimal		= 10,
            Hexadecimal = 16,
            Octal		= 8,
            Binary		= 2
        };
        EEncoding eEncoding = EEncoding::Decimal;

        auto fnConvertToDecimal = [this, &eEncoding](char c) -> uint32_t
        {
            switch (eEncoding)
            {
            case EEncoding::Hexadecimal:
                return HexadecimalToDecimal(std::string(1, c));
            case EEncoding::Decimal:
                return DecimalToDecimal(std::string(1, c));
            case EEncoding::Octal:
                return OctalToDecimal(std::string(1, c));
            case EEncoding::Binary:
                return BinaryToDecimal(std::string(1, c));
            default:
                return 0;
            }
        };

        CToken token;
        try
        {
            std::size_t nIndex = 0;
            int64_t	iSign = 1;
            int64_t	iValue = 0;
            if (ssIntegerString[0] == '-')
            {
                iSign = -1;
                ++nIndex;
            }
            else if (ssIntegerString[0] == '+')
                ++nIndex;
            else if (ssIntegerString.substr(0, 2) == "0x")
            {
                eEncoding = EEncoding::Hexadecimal;
                nIndex += 2;
            }
            else if (ssIntegerString.substr(0, 2) == "0o")
            {
                eEncoding = EEncoding::Octal;
                nIndex += 2;
            }
            else if (ssIntegerString.substr(0, 2) == "0b")
            {
                eEncoding = EEncoding::Binary;
                nIndex += 2;
            }

            if (eEncoding == EEncoding::Decimal && ssIntegerString[nIndex] == '0' && ssIntegerString.size() > nIndex + 1)
                throw XTOMLParseException("No leading zeros allowed!");

            size_t nLastUnderscore = 999;
            for (std::size_t n = nIndex; n < ssIntegerString.size(); ++n)
            {
                if (ssIntegerString[n] == '_')
                {
                    if (n == (nLastUnderscore + 1) || n == nIndex || n == (ssIntegerString.size() - 1))
                        throw XTOMLParseException("Underscore has to be enclosed in digits!");
                    nLastUnderscore = n;
                    continue;
                }
                iValue = iValue * static_cast<int64_t>(eEncoding) + fnConvertToDecimal(ssIntegerString[n]);
            }
            token = CToken(ETokenCategory::token_integer, iSign * iValue);
        }
        catch (const sdv::toml::XTOMLParseException& rexcept)
        {
            token = CToken(ETokenCategory::token_error, rexcept);
        }
        if (m_stackExpectations.top() == EExpectation::expect_value_once)
            m_stackExpectations.pop();
        return token;
    }

    CToken CLexer::ReadFloat(CCharacterReaderUTF8& rReader)
    {
        if (m_stackExpectations.top() == EExpectation::expect_key)
            return {};
        std::string ssFloatChars = rReader.PeekUntil(m_vecValueDelimiters);
        if (ssFloatChars.empty())
            return {};

        bool bIsFloat = ssFloatChars == "inf" || ssFloatChars == "+inf" || ssFloatChars == "-inf" || ssFloatChars == "nan"
            || ssFloatChars == "+nan" || ssFloatChars == "-nan";
        for (char c : ssFloatChars)
        {
            if (bIsFloat)
                break;
            if ((c < '0' || c > '9') && c != '_' && c != '-' && c != '+'
                && c != 'e' && c != 'E' && c != '.')
                break;
            if (c == 'e' || c == '.' || c == 'E')
                bIsFloat = true;
        }
        if (!bIsFloat) return {};
    
        rReader.ConsumeUntil(m_vecValueDelimiters);
        std::string ssFloatingpointString;

        // Remove any underscores. Each underscore must be surrounded by one digit
        for (size_t n = 0; n < ssFloatChars.size(); n++)
        {
            if (ssFloatChars[n] == '_')
            {
                if (n == 0 || n == (ssFloatChars.size() - 1) || !std::isdigit(ssFloatChars[n-1]) || !std::isdigit(ssFloatChars[n+1]))
                {
                    if (m_stackExpectations.top() == EExpectation::expect_value_once)
                        m_stackExpectations.pop();
                    return CToken(ETokenCategory::token_error, "Underscore has to be enclosed in digits!");
                }
            }
            else
                ssFloatingpointString += ssFloatChars[n];
        }
    
        CToken token;
        size_t nDotPosition = ssFloatingpointString.find('.');
        if (nDotPosition == 0 || (nDotPosition == ssFloatingpointString.size() - 1)
            || (nDotPosition != std::string::npos
                && ((ssFloatingpointString[nDotPosition - 1] < '0' || ssFloatingpointString[nDotPosition - 1] > '9')
                    || (ssFloatingpointString[nDotPosition + 1] < '0' || ssFloatingpointString[nDotPosition + 1] > '9'))))
            token = CToken(ETokenCategory::token_error, "The '.' has to be between two digits");
        else
        {
            try
            {
                token = CToken(ETokenCategory::token_float, std::stod(ssFloatingpointString));
            }
            catch ([[maybe_unused]] const std::exception&)
            {
                token = CToken(ETokenCategory::token_error,
                    std::string("Unable to convert '") + ssFloatingpointString + "' to floating point value");
            }
        }
        if (m_stackExpectations.top() == EExpectation::expect_value_once)
            m_stackExpectations.pop();
        return token;
    }

    CToken CLexer::ReadBool(CCharacterReaderUTF8& rReader)
    {
        if (m_stackExpectations.top() == EExpectation::expect_key)
            return {};
        std::string ssBoolChars = rReader.PeekUntil(m_vecValueDelimiters);
        if (ssBoolChars != "true" && ssBoolChars != "false")
            return {};

        CToken token;
        ssBoolChars = rReader.ConsumeUntil(m_vecValueDelimiters);
        if (ssBoolChars == "true")
            token = CToken(ETokenCategory::token_boolean, true);
        else if (ssBoolChars == "false")
            token = CToken(ETokenCategory::token_boolean, false);
        else
            token = CToken(ETokenCategory::token_error, std::string("Unable to convert '") + ssBoolChars + "' to floating point value");
        if (m_stackExpectations.top() == EExpectation::expect_value_once)
            m_stackExpectations.pop();
        return token;
    }

    CToken CLexer::ReadWhitespace(CCharacterReaderUTF8& rReader) const
    {
        bool bWhitespace = false;
        while (rReader.Peek()[0] == ' ' || rReader.Peek()[0] == '\t')
        {
            // Only read whitespace
            rReader.Consume();
            bWhitespace = true;
        }
        if (!bWhitespace)
            return {};

        return CToken(ETokenCategory::token_whitespace);
    }

    CToken CLexer::ReadSyntaxElement(CCharacterReaderUTF8& rReader)
    {
        std::string ssElement = rReader.Peek();
        if (ssElement.empty())
            return {};
    
        CToken token;
        switch (ssElement[0])
        {
        case '\n':
            rReader.Consume();
            token = CToken(ETokenCategory::token_syntax_new_line);
            break;
        case '\r':
            rReader.Consume();
            rReader.Consume();
            token = CToken(ETokenCategory::token_syntax_new_line);
            break;
        case '[':
            rReader.Consume();
            if (m_stackExpectations.top() != EExpectation::expect_key)
            {
                token = CToken(ETokenCategory::token_syntax_array_open);
                m_stackExpectations.push(EExpectation::expect_value);
            }
            else
            {
                if (rReader.Peek() == "[")
                {
                    token = CToken(ETokenCategory::token_syntax_table_array_open);
                    rReader.Consume();
                }
                else
                    token = CToken(ETokenCategory::token_syntax_table_open);
            }
            break;
        case ']':
            rReader.Consume();
            if (m_stackExpectations.top() != EExpectation::expect_key)
            {
                token = CToken(ETokenCategory::token_syntax_array_close);
                m_stackExpectations.pop();
                if (m_stackExpectations.top() == EExpectation::expect_value_once)
                    m_stackExpectations.pop();
            }
            else
            {
                if (rReader.Peek() == "]")
                {
                    token = CToken(ETokenCategory::token_syntax_table_array_close);
                    rReader.Consume();
                }
                else
                    token = CToken(ETokenCategory::token_syntax_table_close);
            }
            break;
        case '{':
            rReader.Consume();
            token = CToken(ETokenCategory::token_syntax_inline_table_open);
            m_stackExpectations.push(EExpectation::expect_key);
            break;
        case '}':
            rReader.Consume();
            token = CToken(ETokenCategory::token_syntax_inline_table_close);
            m_stackExpectations.pop();
            if (m_stackExpectations.top() == EExpectation::expect_value_once)
                m_stackExpectations.pop();
            break;
        case ',':
            rReader.Consume();
            token = CToken(ETokenCategory::token_syntax_comma);
            break;
        case '.':
            rReader.Consume();
            token = CToken(ETokenCategory::token_syntax_dot);
            break;
        case '=':
            rReader.Consume();
            token = CToken(ETokenCategory::token_syntax_assignment);
            m_stackExpectations.push(EExpectation::expect_value_once);
            break;
        default:
            return {};
        }
        return token;
    }

    CToken CLexer::ReadComment(CCharacterReaderUTF8& rReader)
    {
        if (rReader.Peek()[0] != '#') return {};
        rReader.ConsumeUntil({"\n"});
        return CToken(ETokenCategory::token_comment);
    }

    CToken CLexer::ReadUnknownSequence(CCharacterReaderUTF8& rReader)
    {
        std::string sequence = rReader.ConsumeUntil(m_vecValueDelimiters);
        CToken token = CToken(ETokenCategory::token_error, "Invalid Sequence '" + sequence + "'");
        if (m_stackExpectations.top() == EExpectation::expect_value_once)
            m_stackExpectations.pop();
        return token;
    }

    std::string CLexer::Unescape(CCharacterReaderUTF8& rReader)
    {
        std::string ssEscapeChar = rReader.Consume();
        switch (ssEscapeChar[0])
        {
        case 'b':
            return "\b";
        case 't':
            return "\t";
        case 'n':
            return "\n";
        case 'f':
            return "\f";
        case 'r':
            return "\r";
        case '"':
            return "\"";
        case '\\':
            return "\\";
        case 'u':
            return EscapedUnicodeCharacterToUTF8(rReader, 4);
        case 'U':
            return EscapedUnicodeCharacterToUTF8(rReader, 8);
        default:
            throw XTOMLParseException(("Invalid escape sequence: \\" + ssEscapeChar).c_str());
        }
    }

    std::string CLexer::EscapedUnicodeCharacterToUTF8(CCharacterReaderUTF8& rReader, size_t nDigits)
    {
        // Read the characters
        std::string ssHexValue;
        for (size_t n = 1; n <= nDigits; ++n)
        {
            std::string ssChar = rReader.Consume();
            if (ssChar.size() != 1 || !std::isxdigit(ssChar[0]))
                throw XTOMLParseException("Invalid digit in UNICODE escape string.");
            ssHexValue += ssChar;
        }
        return toml_parser::EscapedUnicodeCharacterToUTF8(ssHexValue);
    }

} // namespace toml_parser