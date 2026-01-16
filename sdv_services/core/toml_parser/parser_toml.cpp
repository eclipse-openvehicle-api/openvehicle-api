#include "parser_toml.h"
#include <iostream>
#include "miscellaneous.h"
#include "exception.h"

/// The TOML parser namespace
namespace toml_parser
{
    CParser::CParser(const std::string& rssString)
    {
        Process(rssString);
    }

    void CParser::Clear()
    {
        m_ptrRoot.reset();
        m_ptrCurrentCollection.reset();
        m_lexer.Reset();
        while (!m_stackEnvironment.empty())
            m_stackEnvironment.pop();
    }

    bool CParser::Process(/*in*/ const sdv::u8string& ssContent)
    {
        // Process the TOML string
        Clear();
        m_lexer.Feed(ssContent);

        // Create the root node.
        m_ptrRoot = std::make_shared<CRootTable>(*this);
        m_ptrCurrentCollection = m_ptrRoot;

        // Nodes available at all?
        if (m_lexer.IsEnd()) return true;

        // The initial start position for the node token is the begin of the token list.
        std::reference_wrapper<const CToken> refStartNodeToken = m_lexer.Peek().JumpToBegin();

        CNodeTokenRange rangeRootTokens(refStartNodeToken);

        try
        {
            // Run through all tokens of the lexer and process the tokens.
            bool bEOF = false; // Explicit test, since the peek could return EOF, but the cursor might not be at the end yet.
            while (!bEOF && !m_lexer.IsEnd())
            {
                // The node has its own token range
                CNodeTokenRange rangeNode(refStartNodeToken.get());

                const CToken& rCurrent = m_lexer.Peek();
                if (!rCurrent)
                {
                    bEOF = true;
                    break;
                }
                switch (rCurrent.Category())
                {
                case ETokenCategory::token_syntax_table_open:
                    ProcessTable(rangeNode);
                    break;
                case ETokenCategory::token_syntax_table_array_open:
                    ProcessTableArray(rangeNode);
                    break;
                case ETokenCategory::token_key:
                    ProcessValueKey(rangeNode);
                    break;
                case ETokenCategory::token_syntax_new_line:
                    m_lexer.Consume();
                    break;
                case ETokenCategory::token_terminated:
                case ETokenCategory::token_error:
                    throw XTOMLParseException(rCurrent.StringValue());
                    break;
                default:
                    throw XTOMLParseException("Invalid Syntax; not a Key, Table or Tablearray");
                }

                // Update the start of the next value range
                refStartNodeToken = rangeNode.LinesBehindNode().End();
            }
        }
        catch (const sdv::toml::XTOMLParseException& e)
        {
            std::cout << e.what() << '\n';
            throw;
        }

        // In case there are no nodes any more, but still comments and whitespace, attach this to the root node.
        if (refStartNodeToken.get() != m_lexer.Peek())
        {
            rangeRootTokens.NodeMain(CTokenRange(refStartNodeToken, refStartNodeToken));
            rangeRootTokens.LinesBehindNode(m_lexer.Peek());
            m_ptrRoot->UpdateNodeCode(rangeRootTokens);
        }
        
        return true;
    }

    CLexer& CParser::Lexer()
    {
        return m_lexer;
    }

    const CNodeCollection& CParser::Root() const
    {
        auto ptrCollection = m_ptrRoot->Cast<CTable>();
        return *ptrCollection.get();
    }

    CNodeCollection& CParser::Root()
    {
        auto ptrCollection = m_ptrRoot->Cast<CTable>();
        return *ptrCollection.get();
    }

    std::string CParser::GenerateTOML(const std::string& rssPrefixKey) const
    {
        return m_ptrRoot->GenerateTOML(rssPrefixKey);
    }

    void CParser::ProcessTable(CNodeTokenRange& rNodeRange)
    {
        // Get the table path (table name preceded by parent tables separated with dots).
        CTokenRange rangeMain(m_lexer.Consume());
        CTokenRange rangeKeyPath = ProcessKeyPath();
        const CToken& rToken = m_lexer.Consume();
        if (!rToken || rToken.Category() != ETokenCategory::token_syntax_table_close)
            throw XTOMLParseException("Invalid Table construct");
        rangeMain.AssignEndToken(rToken); // NOTE: This includes only the name and the brackets, not the values.

        // Assign the main token range to the node token range and let the lexer determine the extended token range
        rNodeRange.NodeMain(rangeMain);
        m_lexer.SmartExtendNodeRange(rNodeRange);

        // Add the table to the root
        auto ptrTable = m_ptrRoot->Insert<CTable>(sdv::toml::npos, rangeKeyPath, false);
        if (ptrTable)
        {
            m_ptrCurrentCollection = ptrTable->Cast<CTable>();
            ptrTable->UpdateNodeCode(rNodeRange);
        }
    }

    void CParser::ProcessTableArray(CNodeTokenRange& rNodeRange)
    {
        CTokenRange rangeMain(m_lexer.Consume());
        CTokenRange rangeKeyPath = ProcessKeyPath();
        const CToken& rToken = m_lexer.Consume();
        if (!rToken || rToken.Category() != ETokenCategory::token_syntax_table_array_close)
            throw XTOMLParseException("Invalid Table Array construct");
        rangeMain.AssignEndToken(rToken); // NOTE: This includes only the name and the brackets, not the values.

        // Assign the main token range to the node token range and let the lexer determine the extended token range
        rNodeRange.NodeMain(rangeMain);
        m_lexer.SmartExtendNodeRange(rNodeRange);

        // Add the table array to the root
        auto ptrTableArray = m_ptrRoot->Insert<CTableArray>(sdv::toml::npos, rangeKeyPath);
        if (ptrTableArray)
        {
            m_ptrCurrentCollection = ptrTableArray->Cast<CNodeCollection>();
            ptrTableArray->UpdateNodeCode(rNodeRange);
        }
    }

    void CParser::ProcessValueKey(CNodeTokenRange& rNodeRange)
    {
        // Initial part of the main token range containing the key and the assignment
        CTokenRange rangeMain(m_lexer.Peek());

        CTokenRange rangeKeyPath = ProcessKeyPath();

        const CToken& rToken = m_lexer.Consume();
        if (rToken.Category() != ETokenCategory::token_syntax_assignment)
            throw XTOMLParseException("Assignment expected");
        rangeMain.AssignEndToken(rToken);

        // Store this initial range
        rNodeRange.NodeMain(rangeMain);

        // Process the value assignment
        ProcessValue(rangeKeyPath, rNodeRange);
    }

    void CParser::ProcessValue(const CTokenRange& rrangeKeyPath, CNodeTokenRange& rNodeRange)
    {
        // Extend the main range
        CTokenRange rangeMain = rNodeRange.NodeMain();

        // Skip newlines (other whitespace and comments are already skipped)
        while (m_lexer.Peek().Category() == ETokenCategory::token_syntax_new_line)
            m_lexer.Consume();

        // Get the value
        const CToken& rAssignmentValue = m_lexer.Consume();
        if (!rAssignmentValue)
            throw XTOMLParseException("Missing value");

        // Assign the end token for the main part
        rangeMain.AssignEndToken(rAssignmentValue);
        rNodeRange.NodeMain(rangeMain);

        // Process the value
        std::shared_ptr<CNode> ptrNode;
        switch (rAssignmentValue.Category())
        {
        case ETokenCategory::token_boolean:
            ptrNode = m_ptrCurrentCollection->Insert<CBooleanNode>(sdv::toml::npos, rrangeKeyPath, rAssignmentValue.BooleanValue(),
                rAssignmentValue.RawString());
            break;
        case ETokenCategory::token_integer:
            ptrNode = m_ptrCurrentCollection->Insert<CIntegerNode>(sdv::toml::npos, rrangeKeyPath, rAssignmentValue.IntegerValue(),
                rAssignmentValue.RawString());
            break;
        case ETokenCategory::token_float:
            ptrNode = m_ptrCurrentCollection->Insert<CFloatingPointNode>(sdv::toml::npos, rrangeKeyPath, rAssignmentValue.FloatValue(),
                 rAssignmentValue.RawString());
            break;
        case ETokenCategory::token_string:
            switch (rAssignmentValue.StringType())
            {
            case ETokenStringType::literal_string:
                ptrNode = m_ptrCurrentCollection->Insert<CStringNode>(sdv::toml::npos, rrangeKeyPath, rAssignmentValue.StringValue(),
                    CStringNode::EQuotationType::literal_string, rAssignmentValue.RawString());
                break;
            case ETokenStringType::multi_line_literal:
                ptrNode = m_ptrCurrentCollection->Insert<CStringNode>(
                    sdv::toml::npos, rrangeKeyPath, rAssignmentValue.StringValue(), CStringNode::EQuotationType::multi_line_literal,
                    rAssignmentValue.RawString());
                break;
            case ETokenStringType::multi_line_quoted:
                ptrNode = m_ptrCurrentCollection->Insert<CStringNode>(
                    sdv::toml::npos, rrangeKeyPath, rAssignmentValue.StringValue(), CStringNode::EQuotationType::multi_line_quoted,
                    rAssignmentValue.RawString());
                break;
            case ETokenStringType::quoted_string:
            default:
                ptrNode = m_ptrCurrentCollection->Insert<CStringNode>(
                    sdv::toml::npos, rrangeKeyPath, rAssignmentValue.StringValue(), CStringNode::EQuotationType::quoted_string,
                    rAssignmentValue.RawString());
                break;
            }
            break;
        case ETokenCategory::token_syntax_array_open:
            {
                auto ptrCurrentCollectionStored = m_ptrCurrentCollection;
                ptrNode = m_ptrCurrentCollection->Insert<CArray>(sdv::toml::npos, rrangeKeyPath);
                m_ptrCurrentCollection = ptrNode->Cast<CNodeCollection>();
                m_stackEnvironment.push(EEnvironment::env_array);
                ProcessArray(rNodeRange);
                m_stackEnvironment.pop();
                m_ptrCurrentCollection = ptrCurrentCollectionStored;
            }
            break;
        case ETokenCategory::token_syntax_inline_table_open:
            {
                auto ptrCurrentCollectionStored = m_ptrCurrentCollection;
                ptrNode = m_ptrCurrentCollection->Insert<CTable>(sdv::toml::npos, rrangeKeyPath, true);
                m_ptrCurrentCollection = ptrNode->Cast<CNodeCollection>();
                m_stackEnvironment.push(EEnvironment::env_inline_table);
                ProcessInlineTable(rNodeRange);
                m_stackEnvironment.pop();
                m_ptrCurrentCollection = ptrCurrentCollectionStored;
            }
            break;
        default:
            throw XTOMLParseException("Missing value");
            break;
        }

        // let the lexer determine the extended token range and update the node
        m_lexer.SmartExtendNodeRange(rNodeRange);

        // Deal with the next value if expecting
        std::reference_wrapper<const CToken> refToken = m_lexer.Peek();
        if (!m_stackEnvironment.empty())
        {
            // Skip newlines
            while (refToken.get().Category() == ETokenCategory::token_syntax_new_line)
            {
                m_lexer.Consume();
                refToken = m_lexer.Peek();
            }

            switch (m_stackEnvironment.top())
            {
            case EEnvironment::env_array:
                {
                    int32_t index = 1;
                    while (refToken.get() && refToken.get().Category() == ETokenCategory::token_syntax_new_line)
                    {
                        refToken = m_lexer.Peek(index++);
                    }
                    if (!refToken.get()
                        || (refToken.get().Category() != ETokenCategory::token_syntax_comma
                            && refToken.get().Category() != ETokenCategory::token_syntax_array_close))
                    {
                        throw XTOMLParseException("Invalid Token after value assignment in array; ',' or ']' needed");
                    }
                }
                break;
            case EEnvironment::env_inline_table:
                if (!refToken.get()
                    || (refToken.get().Category() != ETokenCategory::token_syntax_comma
                        && refToken.get().Category() != ETokenCategory::token_syntax_inline_table_close))
                {
                    throw XTOMLParseException("Invalid Token after value assignment in inline table; ',' or '}' needed ");
                }
                break;
            default:
                break;
            }
            rNodeRange.LinesBehindNode(refToken);
        }
        else
        {
            if (refToken.get() && refToken.get().Category() != ETokenCategory::token_syntax_new_line)
            {
                throw XTOMLParseException("Invalid Token after value assignment; newline needed");
            }
        }
        if (ptrNode) ptrNode->UpdateNodeCode(rNodeRange);
    }

    void CParser::ProcessArray(CNodeTokenRange& rNodeRange)
    {
        /*
        Arrays are defined as follow: array_name = [value, value, ...]
        Arrays can have new-lines between their values.
        And can end with a comma.
        For example:
        integers = [ 1, 2, 3 ]
        colors = [ "red", "yellow", "green", ]
        nested_arrays_of_ints = [ [ 1, 2 ],
            [3, 4, 5] ]
        nested_mixed_array = [ [ 1, 2 ], ["a", "b", "c"] ]
        string_array = [ "all", 'strings', """are the same""", '''type''' ]
        */

        // The initial start position for the value token is the end of the first main part.
        std::reference_wrapper<const CToken> refStartValueToken = rNodeRange.LinesBehindNode().End();

        // Iterator token
        std::reference_wrapper<const CToken> refToken = m_lexer.Peek();

        // State machine
        enum class EExpect
        {
            value_or_end,
            comma_or_end // Trailing comma is allowed
        } eExpect = EExpect::value_or_end;

        // Iterate through the values
        size_t nIndex = 0;
        bool bAdditionalCommaBeforeEnd = false;
        std::reference_wrapper<const CToken> refCommaToken = m_lexer.Peek();
        while (refToken.get() && refToken.get().Category() != ETokenCategory::token_syntax_array_close)
        {
            // The value has its own token range
            CNodeTokenRange rangeNode(refStartValueToken.get());

            switch (refToken.get().Category())
            {
            case ETokenCategory::token_syntax_new_line:
                m_lexer.Consume();
                break;
            case ETokenCategory::token_syntax_comma:
                if (eExpect == EExpect::value_or_end) throw XTOMLParseException("Expecting value or array end.");
                m_lexer.Consume();
                eExpect = EExpect::value_or_end;
                refCommaToken = refToken;
                bAdditionalCommaBeforeEnd = true;
                break;
            default:
                {
                    bAdditionalCommaBeforeEnd = false;
                    if (eExpect == EExpect::comma_or_end) throw XTOMLParseException("Expecting comma or array end.");
                    CLexer lexerLocal("[" + std::to_string(nIndex++) + "]", true);
                    CTokenRange rangeIndexKey(lexerLocal.Peek(), lexerLocal.Peek().JumpToEnd());
                    rangeNode.NodeMain(CTokenRange(refToken, refToken.get().Next()));
                    ProcessValue(rangeIndexKey, rangeNode);
                    eExpect = EExpect::comma_or_end;

                    // Update the start of the next value range
                    refStartValueToken = rangeNode.LinesBehindNode().End();
 
                    break;
                }
            }

            // Get the next value
            refToken = m_lexer.Peek();
        }

        // This is the node token range finishing main part.
        rNodeRange.NodeMainFinish(
            CTokenRange(bAdditionalCommaBeforeEnd ? refStartValueToken.get() : refToken.get(), refToken.get().Next()));

        // Consume the token
        m_lexer.Consume();
    }

    void CParser::ProcessInlineTable(CNodeTokenRange& rNodeRange)
    {
        /*
        Inline tables are defined as follow: table_name = {value, value, ...}
        For example:
            name = { first = "Tom", last = "Preston-Werner" }
            point = { x = 1, y = 2 }
            animal = { type.name = "pug" }
        */

        // The initial start position for the value token is the end of the first main part.
        std::reference_wrapper<const CToken> refStartValueToken = rNodeRange.LinesBehindNode().End();

        // Iterator token
        std::reference_wrapper<const CToken> refToken = m_lexer.Peek();

        // State machine
        enum class EExpect
        {
            value_or_end,
            value,
            comma_or_end
        } eExpect = EExpect::value_or_end;

        // Iterate through the value
        while (refToken.get() && refToken.get().Category() != ETokenCategory::token_syntax_inline_table_close)
        {
            // The value has its own token range
            CNodeTokenRange rangeNode(refStartValueToken.get());

            switch (refToken.get().Category())
            {
            case ETokenCategory::token_syntax_new_line:
                m_lexer.Consume();
                break;
            case ETokenCategory::token_syntax_comma:
                if (eExpect != EExpect::comma_or_end) throw XTOMLParseException("Expecting value or table end.");
                m_lexer.Consume();
                eExpect = EExpect::value;
                break;
            default:
                if (eExpect == EExpect::comma_or_end) throw XTOMLParseException("Expecting comma or table end.");
                ProcessValueKey(rangeNode);
                eExpect = EExpect::comma_or_end;
                break;
            }

            // Update the start of the next value range
            refStartValueToken = rangeNode.LinesBehindNode().End();

            // Get the next value
            refToken = m_lexer.Peek();
        }
        if (eExpect == EExpect::value)
            throw XTOMLParseException("Expecting a value before inline table end.");

        // This is the node token range finishing main part.
        rNodeRange.NodeMainFinish(CTokenRange(refToken.get(), refToken.get().Next()));

        // Consume the token
        m_lexer.Consume();
    }

    CTokenRange CParser::ProcessKeyPath()
    {
        //std::string ssPath;
        std::reference_wrapper<const CToken> refToken = m_lexer.Peek();
        std::reference_wrapper<const CToken> refKeyStart = refToken;
        if (!refToken.get()
            || (refToken.get().Category() != ETokenCategory::token_syntax_dot
                   && refToken.get().Category() != ETokenCategory::token_key))
            throw XTOMLParseException("Invalid Token to assemble path from keys");

        while (refToken.get()
               && (refToken.get().Category() == ETokenCategory::token_syntax_dot
                   || refToken.get().Category() == ETokenCategory::token_key))
        {
            m_lexer.Consume();
            //if (refToken.get().Category() == ETokenCategory::token_key)
            //{
            //    EQuoteRequest eQuoteRequest = EQuoteRequest::smart_key;
            //    switch (refToken.get().StringType())
            //    {
            //    case ETokenStringType::literal_string:
            //        eQuoteRequest = EQuoteRequest::literal_text;
            //        break;
            //    case ETokenStringType::quoted_string:
            //        eQuoteRequest = EQuoteRequest::quoted_text;
            //        break;
            //    case ETokenStringType::multi_line_literal:
            //        eQuoteRequest = EQuoteRequest::multi_line_literal_text;
            //        break;
            //    case ETokenStringType::multi_line_quoted:
            //        eQuoteRequest = EQuoteRequest::multi_line_quoted_text;
            //        break;
            //    default:
            //        break;
            //    }
            //}
            refToken = m_lexer.Peek();
        }
        
        return CTokenRange(refKeyStart.get(), refToken);
    }
} // namespace toml_parser