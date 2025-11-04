#include "parser_toml.h"
#include <iostream>
#include "exception.h"

CParserTOML::CParserTOML(const std::string& rssString) : m_lexer(rssString)
{
    Process(rssString);
}

void CParserTOML::Clear()
{
    m_ptrRoot = std::make_shared<CRootTable>();
    m_ssCurrentTable.clear();
    m_lexer.Reset();
    while (!m_stackEnvironment.empty()) m_stackEnvironment.pop();
}

bool CParserTOML::Process(/*in*/ const sdv::u8string& ssContent)
{
    Clear();
    m_lexer.Feed(ssContent);
    try
    {
        // Run through all tokens of the lexer and process the tokens.
        while (!m_lexer.IsEnd())
        {
            CLexerTOML::SToken current = m_lexer.Peek();
            switch (current.eCategory)
            {
            case CLexerTOML::ETokenCategory::token_syntax_table_open:
                ProcessTable();
                break;
            case CLexerTOML::ETokenCategory::token_syntax_table_array_open:
                ProcessTableArray();
                break;
            case CLexerTOML::ETokenCategory::token_key:
                ProcessValueKey();
                break;
            case CLexerTOML::ETokenCategory::token_syntax_new_line:
                m_lexer.Consume();
                break;
            case CLexerTOML::ETokenCategory::token_terminated:
            case CLexerTOML::ETokenCategory::token_error:
                throw XTOMLParseException(current.ssContentString);
                break;
            default:
                throw XTOMLParseException("Invalid Syntax; not a Key, Table or Tablearray");
            }
        }
    }
    catch (const sdv::toml::XTOMLParseException& e)
    {
        std::cout << e.what() << '\n';
        throw;
    }
    return true;
}

const CNodeCollection& CParserTOML::GetRoot() const
{
    auto ptrCollection = m_ptrRoot->GetTable();
    return *ptrCollection.get();
}

CNodeCollection& CParserTOML::GetRoot()
{
    auto ptrCollection = m_ptrRoot->GetTable();
    return *ptrCollection.get();
}

std::string CParserTOML::CreateTOMLText(const std::string& rssParent) const
{
    std::string ssLastPrintedTable;
    return m_ptrRoot->CreateTOMLText(rssParent, ssLastPrintedTable);
}

bool CParserTOML::Add(const std::string& rssPath, bool bVal)
{
    size_t nOffset = FindLast(rssPath);
    if (nOffset == std::string::npos)
        nOffset = 0;
    else
        nOffset++;
    std::string ssName = rssPath.substr(nOffset);

    m_ptrRoot->Add(rssPath, std::make_shared<CBooleanNode>(ssName, bVal));
    return true;
}

bool CParserTOML::Add(const std::string& rssPath, int64_t iVal)
{
    size_t nOffset = FindLast(rssPath);
    if (nOffset == std::string::npos)
        nOffset = 0;
    else
        nOffset++;
    std::string ssName = rssPath.substr(nOffset);

    m_ptrRoot->Add(rssPath, std::make_shared<CIntegerNode>(ssName, iVal));
    return true;
}

bool CParserTOML::Add(const std::string& rssPath, double dVal)
{
    size_t nOffset = FindLast(rssPath);
    if (nOffset == std::string::npos)
        nOffset = 0;
    else
        nOffset++;
    std::string ssName = rssPath.substr(nOffset);

    m_ptrRoot->Add(rssPath, std::make_shared<CFloatingPointNode>(ssName, dVal));
    return true;
}

bool CParserTOML::Add(const std::string& rssPath, const std::string& rssVal)
{
    size_t nOffset = FindLast(rssPath);
    if (nOffset == std::string::npos)
        nOffset = 0;
    else
        nOffset++;
    std::string ssName = rssPath.substr(nOffset);

    m_ptrRoot->Add(rssPath, std::make_shared<CStringNode>(ssName, rssVal));
    return true;
}

void CParserTOML::ProcessTable()
{
    // Get the table path (table name preceded by parent tables separated with dots).
    m_lexer.Consume();
    std::string ssPath = ComposePath();
    CLexerTOML::SToken sToken = m_lexer.Consume();
    if (sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_table_close)
    {
        throw XTOMLParseException("invalid Table construct");
    }

    // Find the last dot - the name follows
    size_t nOffset = FindLast(ssPath);
    if (nOffset == std::string::npos)
        nOffset = 0; // No dot found, the whole path is one table name
    else
        nOffset++;  // Skip the dot
    std::string ssName = ssPath.substr(nOffset);

    // Add the table to the root
    m_ptrRoot->Add(ssPath, std::make_shared<CNormalTable>(ssName), false);

    m_ssCurrentTable = ssPath;
}

void CParserTOML::ProcessTableArray()
{
    m_lexer.Consume();
    std::string rssKeyPath = ComposePath();
    auto ptrNode = m_ptrRoot->Find(rssKeyPath);
    if (!ptrNode)
    {
        Add<CTableArray>(rssKeyPath);
        ptrNode = m_ptrRoot->Find(rssKeyPath);
    }
    if (!ptrNode) return;
    if (dynamic_cast<CTableArray*>(ptrNode.get()))
        ptrNode->GetArray()->AddElement(std::make_shared<CNormalTable>(""));
    else
        throw XTOMLParseException(("'" + rssKeyPath + "' already defined as a non-table-array").c_str());
    m_ssCurrentTable = rssKeyPath;

    CLexerTOML::SToken sToken = m_lexer.Consume();
    if (sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_table_array_close)
    {
        throw XTOMLParseException("invalid Table Array construct");
    }
}

void CParserTOML::ProcessValueKey()
{
    std::string rssKeyPath = (m_ssCurrentTable.empty() ? "" : (m_ssCurrentTable + ".")) + ComposePath();

    CLexerTOML::SToken sToken = m_lexer.Consume();
    if (sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_assignment)
    {
        throw XTOMLParseException("Assignment expected");
    }

    ProcessValue(rssKeyPath);
}

void CParserTOML::ProcessValue(const std::string& rssKeyPath)
{
    CLexerTOML::SToken assignmentValue = m_lexer.Consume();
    switch (assignmentValue.eCategory)
    {
    case CLexerTOML::ETokenCategory::token_boolean:
        Add(rssKeyPath, assignmentValue.bContentBoolean);
        break;
    case CLexerTOML::ETokenCategory::token_integer:
        Add(rssKeyPath, assignmentValue.iContentInteger);
        break;
    case CLexerTOML::ETokenCategory::token_float:
        Add(rssKeyPath, assignmentValue.dContentFloatingpoint);
        break;
    case CLexerTOML::ETokenCategory::token_string:
        Add(rssKeyPath, assignmentValue.ssContentString);
        break;
    case CLexerTOML::ETokenCategory::token_syntax_array_open:
        Add<CNormalArray>(rssKeyPath);
        m_stackEnvironment.push(EEnvironment::env_array);
        ProcessArray(rssKeyPath);
        m_stackEnvironment.pop();
        break;
    case CLexerTOML::ETokenCategory::token_syntax_inline_table_open:
        Add<CInlineTable>(rssKeyPath);
        m_stackEnvironment.push(EEnvironment::env_inline_table);
        ProcessInlineTable(rssKeyPath);
        m_stackEnvironment.pop();
        break;
    default:
        throw XTOMLParseException("Missing value");
        break;
    }

    CLexerTOML::SToken sToken = m_lexer.Peek();
    if (!m_stackEnvironment.empty())
    {
        switch (m_stackEnvironment.top())
        {
        case EEnvironment::env_array:
        {
            int32_t index = 2;
            while (sToken.eCategory == CLexerTOML::ETokenCategory::token_syntax_new_line)
            {
                sToken = m_lexer.Peek(index++);
            }
            if (sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_comma
                && sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_array_close)
            {
                throw XTOMLParseException(
                    "Invalid Token after value assignment in array; ',' or ']' needed");
            }
        }
        break;
        case EEnvironment::env_inline_table:
            if (sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_comma
                && sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_inline_table_close)
            {
                throw XTOMLParseException(
                    "Invalid Token after value assignment in inline table; ',' or '}' needed ");
            }
            break;
        default:
            break;
        }
    }
    else
    {
        if (sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_new_line && sToken.eCategory != CLexerTOML::ETokenCategory::token_eof)
        {
            throw XTOMLParseException("Invalid Token after value assignment; newline needed");
        }
    }
}

void CParserTOML::ProcessArray(const std::string& rssKeyPath)
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

    CLexerTOML::SToken sToken = m_lexer.Peek();

    size_t nIndex = 0;
    enum class EExpect {value_comma_end, comma_end} eExpect = EExpect::value_comma_end;
    while (sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_array_close)
    {
        switch (sToken.eCategory)
        {
        //case CLexerTOML::ETokenCategory::token_syntax_array_open:   // Embedded array
        //    if (eExpect == comma_end) throw XTOMLParseException("Expecting comma or table end.");
        //    m_lexer.Consume();
        //    ProcessArray(rssKeyPath + "." + std::to_string(nIndex++));
        //    eExpect = comma_end;
        //    break;
        case CLexerTOML::ETokenCategory::token_syntax_new_line:
            m_lexer.Consume();
            break;
        case CLexerTOML::ETokenCategory::token_syntax_comma:
            m_lexer.Consume();
            eExpect = EExpect::value_comma_end;
            break;
        default:
            if (eExpect == EExpect::comma_end)
                throw XTOMLParseException("Expecting comma or table end.");
            ProcessValue(rssKeyPath + "." + std::to_string(nIndex++));
            eExpect = EExpect::comma_end;
            break;
        }
        sToken = m_lexer.Peek();
    }
    m_lexer.Consume();
}

void CParserTOML::ProcessInlineTable(const std::string& rssKeyPath)
{
    /*
    Inline tables are defined as follow: table_name = {value, value, ...}
    For example:
        name = { first = "Tom", last = "Preston-Werner" }
        point = { x = 1, y = 2 }
        animal = { type.name = "pug" }
    */

    CLexerTOML::SToken sToken = m_lexer.Peek();

    std::string ssCurrentTableTemp = m_ssCurrentTable;
    m_ssCurrentTable = rssKeyPath;
    enum class EExpect { value_comma_end, value, comma_end } eExpect = EExpect::value_comma_end;
    while (sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_inline_table_close)
    {
        switch (sToken.eCategory)
        {
        case CLexerTOML::ETokenCategory::token_syntax_new_line:
            throw XTOMLParseException("No newlines allowed in inline table");
            break;
        case CLexerTOML::ETokenCategory::token_syntax_comma:
            if (eExpect == EExpect::value)
                throw XTOMLParseException("Unexpected comma.");
            m_lexer.Consume();
            eExpect = EExpect::value;
            break;
        default:
            if (eExpect == EExpect::comma_end)
                throw XTOMLParseException("Expecting comma or table end.");
            ProcessValueKey();
            eExpect = EExpect::comma_end;
            break;
        }
        sToken = m_lexer.Peek();
    }
    if (eExpect == EExpect::value)
        throw XTOMLParseException("Expecting a value before inline table end.");

    m_ptrRoot->Find(rssKeyPath)->GetTable()->m_bOpenToAddChildren = false;

    m_lexer.Consume();
    m_ssCurrentTable = ssCurrentTableTemp;
}

std::string CParserTOML::ComposePath()
{
    std::string ssPath;
    CLexerTOML::SToken sToken = m_lexer.Peek();
    if (sToken.eCategory != CLexerTOML::ETokenCategory::token_syntax_dot
        && sToken.eCategory != CLexerTOML::ETokenCategory::token_key)
        throw XTOMLParseException("Invalid Token to assemble path from keys");

    while (sToken.eCategory == CLexerTOML::ETokenCategory::token_syntax_dot
        || sToken.eCategory == CLexerTOML::ETokenCategory::token_key)
    {
        m_lexer.Consume();
        if (sToken.eCategory == CLexerTOML::ETokenCategory::token_key)
            ssPath += sToken.ssContentString;
        else
            ssPath += ".";
        sToken = m_lexer.Peek();
    }
    return ssPath;
}

