#include "lexer_toml.h"
#include <interfaces/toml.h>
#include "exception.h"

CLexerTOML::CLexerTOML(const std::string& rssString)
{
    Feed(rssString);
}

void CLexerTOML::Feed(const std::string& rssString)
{
    Reset();
    try
    {
        m_reader.Feed(rssString);
        GenerateTokens();
    }
    catch (const sdv::toml::XTOMLParseException& rexcept)
    {
        SToken newToken{ETokenCategory::token_terminated};
        newToken.ssContentString = rexcept.what();
        m_vecTokens.push_back(newToken);
        throw;
    }
}

void CLexerTOML::Reset()
{
    m_reader.Reset();
    m_vecTokens.clear();
    m_nCursor = 0;
}

CLexerTOML::SToken CLexerTOML::Peek() const
{
    if (m_vecTokens.empty())
    {
        return SToken{ETokenCategory::token_eof};
    }
    return m_vecTokens[m_nCursor];
}

CLexerTOML::SToken CLexerTOML::Consume()
{
    if (m_nCursor >= m_vecTokens.size() - 1)
    {
        m_nCursor = m_vecTokens.size() - 1;
        if (m_vecTokens.empty())
        {
            return CLexerTOML::SToken{ETokenCategory::token_eof};
        }
        return m_vecTokens.back();
    }
    return m_vecTokens[m_nCursor++];
}


CLexerTOML::SToken CLexerTOML::Peek(int32_t n)
{
    if (n < 1)
    {
        return CLexerTOML::SToken{CLexerTOML::ETokenCategory::token_empty};
    }
    if (m_nCursor + n >= m_vecTokens.size())
    {
        if (m_vecTokens.empty())
        {
            return CLexerTOML::SToken{ETokenCategory::token_eof};
        }
        return m_vecTokens.back();
    }
    return m_vecTokens[m_nCursor + n - 1];
}

CLexerTOML::SToken CLexerTOML::Consume(int32_t n)
{
    if (n < 1)
    {
        return CLexerTOML::SToken{CLexerTOML::ETokenCategory::token_empty};
    }
    if (m_nCursor + n >= m_vecTokens.size())
    {
        m_nCursor = m_vecTokens.size() - 1;
        if (m_vecTokens.empty())
        {
            return CLexerTOML::SToken{ETokenCategory::token_eof};
        }
        return m_vecTokens.back();
    }

    m_nCursor += n;
    return m_vecTokens[m_nCursor - 1];
}

bool CLexerTOML::IsEnd() const
{
    return m_nCursor >= m_vecTokens.size() - 1;
}

void CLexerTOML::GenerateTokens()
{
    m_stackExpectations.push(EExpectation::expect_key);
    while (!m_reader.IsEOF())
    {
        if (IsBareKey())
        {
            ReadBareKey();
        }
        else if (IsBasicQuotedKey())
        {
            ReadBasicQuotedKey();
        }
        else if (IsLiteralQuotedKey())
        {
            ReadLiteralQuotedKey();
        }
        else if (IsBasicMultilineString())
        {
            ReadBasicMultilineString();
        }
        else if (IsBasicString())
        {
            ReadBasicString();
        }
        else if (IsLiteralMultilineString())
        {
            ReadLiteralMultilineString();
        }
        else if (IsLiteralString())
        {
            ReadLiteralString();
        }
        else if (IsInteger())
        {
            ReadInteger();
        }
        else if (IsFloat())
        {
            ReadFloat();
        }
        else if (IsBool())
        {
            ReadBool();
        }
        else if (IsWhitespace())
        {
            ReadWhitespace();
        }
        else if (IsSyntaxElement())
        {
            ReadSyntaxElement();
        }
        else if (IsComment())
        {
            ReadComment();
        }
        else
        {
            ReadUnknownSequence();
        }
    }
    m_vecTokens.emplace_back(ETokenCategory::token_eof);
}

bool CLexerTOML::IsBasicQuotedKey()
{
    return (m_stackExpectations.top() == EExpectation::expect_key && m_reader.Peek(1) == "\"");
}

void CLexerTOML::ReadBasicQuotedKey()
{
    try
    {
        bool endOfQuote = false;
        std::string content = "\"";
        m_reader.Consume();
        while (!endOfQuote)
        {
            if (m_reader.IsEOF())
            {
                throw XTOMLParseException(
                    "Unexpected End of File reached while reading multiline string");
            }
            std::string character = m_reader.Consume();
            if (character == "\\")
            {
                content += Unescape();
            }
            else if (character == "\"")
            {
                endOfQuote = true;
            }
            else
            {
                content += character;
            }
        }
        CLexerTOML::SToken newToken(CLexerTOML::ETokenCategory::token_key);
        content += "\"";
        newToken.ssContentString = content;
        m_vecTokens.push_back(newToken);
    }
    catch (const sdv::toml::XTOMLParseException& e)
    {
        CLexerTOML::SToken newToken(CLexerTOML::ETokenCategory::token_error);
        newToken.ssContentString = std::string(e.what());
        m_vecTokens.push_back(newToken);
    }
}

bool CLexerTOML::IsLiteralQuotedKey()
{
    return (m_stackExpectations.top() == EExpectation::expect_key && m_reader.Peek(1) == "\'");
}

void CLexerTOML::ReadLiteralQuotedKey()
{
    bool endOfQuote = false;
    std::string content = "'";
    m_reader.Consume(); // get the initial "'"
    while (!endOfQuote)
    {
        std::string character = m_reader.Consume();
        if (character == "\'")
        {
            endOfQuote = true;
        }
        else
        {
            content += character;
        }
    }
    CLexerTOML::SToken newToken(CLexerTOML::ETokenCategory::token_key);
    content += "'";
    newToken.ssContentString = content;
    m_vecTokens.push_back(newToken);
}

bool CLexerTOML::IsBareKey()
{
    std::string character = m_reader.Peek();
    return (m_stackExpectations.top() == EExpectation::expect_key
        && ((character[0] >= 'A' && character[0] <= 'Z') || (character[0] >= 'a' && character[0] <= 'z')
            || (character[0] >= '0' && character[0] <= '9') || character[0] == '-' || character[0] == '_'));
}

void CLexerTOML::ReadBareKey()
{
    bool		endOfKey = false;
    std::string content;
    bool		error{false};
    while (!endOfKey)
    {
        std::string character = m_reader.Peek();
        if (!character.empty() && character[0] != '.' && character[0] != '=' && character[0] != ' '
            && character[0] != '\t' && character[0] != ']')
        {
            if ((character[0] < '0' || character[0] > '9') && (character[0] < 'A' || character[0] > 'Z')
                && (character[0] < 'a' || character[0] > 'z') && (character[0] != '_') && (character[0] != '-'))
            {
                error = true;
            }
            m_reader.Consume();
            content += character;
        }
        else
        {
            endOfKey = true;
        }
    }
    SToken newToken(ETokenCategory::token_key);
    newToken.ssContentString = content;
    if (error)
    {
        newToken.eCategory	   = ETokenCategory::token_error;
        newToken.ssContentString = "Invalid bare key '" + content + "'";
    }
    m_vecTokens.push_back(newToken);
}

bool CLexerTOML::IsBasicString()
{
    return (m_stackExpectations.top() != EExpectation::expect_key && m_reader.Peek(1) == "\""
        && !(m_reader.Peek(2) == "\"" && m_reader.Peek(3) == "\""));
}

void CLexerTOML::ReadBasicString()
{
    try
    {
        bool		endOfQuote = false;
        std::string content;
        m_reader.Consume();
        while (!endOfQuote)
        {
            if (m_reader.IsEOF())
            {
                throw XTOMLParseException("Unexpected End of File reached while reading string");
            }
            std::string character = m_reader.Consume();
            if (character == "\\")
            {
                content += Unescape();
            }
            else if (character == "\"")
            {
                endOfQuote = true;
            }
            else
            {
                content += character;
            }
        }
        SToken newToken(ETokenCategory::token_string);
        newToken.ssContentString = content;
        m_vecTokens.push_back(newToken);
    }
    catch (const sdv::toml::XTOMLParseException& e)
    {
        CLexerTOML::SToken newToken(CLexerTOML::ETokenCategory::token_error);
        newToken.ssContentString = std::string(e.what());
        m_vecTokens.push_back(newToken);
    }
    if (m_stackExpectations.top() == EExpectation::expect_value_once)
    {
        m_stackExpectations.pop();
    }
}

bool CLexerTOML::IsBasicMultilineString()
{
    return (m_stackExpectations.top() != EExpectation::expect_key && m_reader.Peek(1) == "\""
        && m_reader.Peek(2) == "\"" && m_reader.Peek(3) == "\"");
}

void CLexerTOML::ReadBasicMultilineString()
{
    std::string content;
    bool		endOfQuote					= false;
    auto		ignoreNewLineAtTheBeginning = [this]()
    {
        if (m_reader.Peek() == "\n")
        {
            m_reader.Consume();
        }
        else if (m_reader.Peek() == "\r" && m_reader.Peek(2) == "\n")
        {
            m_reader.Consume(2);
        }
    };
    auto handleBackslashFunctionality = [this, &content]()
    {
        if (m_reader.Peek() == "\n" || (m_reader.Peek() == "\r" && m_reader.Peek(2) == "\n"))
        {
            std::string next = m_reader.Peek();
            while (next == "\n" || (next == "\r" && m_reader.Peek(2) == "\n") || next == " " || next == "\t")
            {
                m_reader.Consume();
                next = m_reader.Peek();
            }
            return;
        }
        content += Unescape();
    };
    auto handleTrippleDoublequotes = [this, &content, &endOfQuote]()
    {
        if (m_reader.Peek(3) == "\"")
        {
            content += '\"';
            return;
        }
        endOfQuote = true;
        m_reader.Consume(2);
    };
    try
    {
        m_reader.Consume(3);
        ignoreNewLineAtTheBeginning();
        while (!endOfQuote)
        {
            if (m_reader.IsEOF())
            {
                throw XTOMLParseException(
                    "Unexpected End of File reached while reading multiline string");
            }
            std::string character = m_reader.Consume();
            if (character == "\\")
            {
                handleBackslashFunctionality();
            }
            else if (character == "\"" && m_reader.Peek() == "\"" && m_reader.Peek(2) == "\"")
            {
                handleTrippleDoublequotes();
            }
            else
            {
                content += character;
            }
        }
        SToken newToken(ETokenCategory::token_string);
        newToken.ssContentString = content;
        m_vecTokens.push_back(newToken);
    }
    catch (const sdv::toml::XTOMLParseException& e)
    {
        CLexerTOML::SToken newToken(CLexerTOML::ETokenCategory::token_error);
        newToken.ssContentString = std::string(e.what());
        m_vecTokens.push_back(newToken);
    }
    if (m_stackExpectations.top() == EExpectation::expect_value_once)
    {
        m_stackExpectations.pop();
    }
}

bool CLexerTOML::IsLiteralString()
{
    return (m_stackExpectations.top() != EExpectation::expect_key && m_reader.Peek(1) == "\'"
        && !(m_reader.Peek(2) == "\'" && m_reader.Peek(3) == "\'"));
}

void CLexerTOML::ReadLiteralString()
{
    try
    {
        bool		endOfQuote{false};
        std::string content;
        m_reader.Consume();
        while (!endOfQuote)
        {
            if (m_reader.IsEOF())
            {
                throw XTOMLParseException("Unexpected End of File reached while reading string");
            }
            std::string character = m_reader.Consume();
            if (character == "'")
            {
                endOfQuote = true;
            }
            else
            {
                content += character;
            }
        }
        SToken newToken(ETokenCategory::token_string);
        newToken.ssContentString = content;
        m_vecTokens.push_back(newToken);
    }
    catch (const sdv::toml::XTOMLParseException& e)
    {
        CLexerTOML::SToken newToken(CLexerTOML::ETokenCategory::token_error);
        newToken.ssContentString = std::string(e.what());
        m_vecTokens.push_back(newToken);
    }
    if (m_stackExpectations.top() == EExpectation::expect_value_once)
    {
        m_stackExpectations.pop();
    }
}

bool CLexerTOML::IsLiteralMultilineString()
{
    return (m_stackExpectations.top() != EExpectation::expect_key && m_reader.Peek(1) == "\'"
        && m_reader.Peek(2) == "\'" && m_reader.Peek(3) == "\'");
}

void CLexerTOML::ReadLiteralMultilineString()
{
    try
    {
        bool endOfQuote = false;
        std::string content;
        m_reader.Consume(3);
        if (m_reader.Peek() == "\n")
        {
            m_reader.Consume();
        }
        else if (m_reader.Peek() == "\r" && m_reader.Peek(2) == "\n")
        {
            m_reader.Consume(2);
        }
        bool bConsumeWhitespace = false;
        while (!endOfQuote)
        {
            if (m_reader.IsEOF())
            {
                throw XTOMLParseException(
                    "Unexpected End of File reached while reading multiline string");
            }
            std::string character = m_reader.Consume();
            if (character == "'" && m_reader.Peek() == "'" && m_reader.Peek(2) == "'")
            {
                if (m_reader.Peek(3) == "'")
                {
                    content += '\'';
                }
                else
                {
                    endOfQuote = true;
                    m_reader.Consume(2);
                }
            }
            else if (character == "\\" && m_reader.Peek() == "\n")
            {
                bConsumeWhitespace = true;
                m_reader.Consume();
            }
            else if (character == "\\" && m_reader.Peek() == "\r" && m_reader.Peek(2) == "\n")
            {
                bConsumeWhitespace = true;
                m_reader.Consume();
            }
            else
            {
                if (!std::isspace(character[0]) || !bConsumeWhitespace)
                {
                    content += character;
                    bConsumeWhitespace = false;
                }
            }
        }
        SToken newToken(ETokenCategory::token_string);
        newToken.ssContentString = content;
        m_vecTokens.push_back(newToken);
    }
    catch (const sdv::toml::XTOMLParseException& e)
    {
        CLexerTOML::SToken newToken(CLexerTOML::ETokenCategory::token_error);
        newToken.ssContentString = std::string(e.what());
        m_vecTokens.push_back(newToken);
    }
    if (m_stackExpectations.top() == EExpectation::expect_value_once)
    {
        m_stackExpectations.pop();
    }
}

bool CLexerTOML::IsInteger()
{
    if (m_stackExpectations.top() == EExpectation::expect_key)
    {
        return false;
    }

    std::string character = m_reader.PeekUntil(m_vecValueDelimiters);

    if (character.empty())
    {
        return false;
    }

    if (character[0] == '0' && (character[1] == 'x' || character[1] == 'o' || character[1] == 'b'))
    {
        return true;
    }

    return std::all_of(character.begin(),
        character.end(),
        [](char c)
        { return ((c >= '0' && c <= '9') || c == '_' || c == '-' || c == '+'); });
    // for (const char& c : character)
    // {
    // 	if (!((c >= 0x30 && c <= 0x39) || c == '_' || c == '-' || c == '+'))
    // 	{
    // 		return false;
    // 	}
    // }
    // return true;
}

void CLexerTOML::ReadInteger()
{
    std::string integerString = m_reader.ConsumeUntil(m_vecValueDelimiters);
    enum class EFormat : int64_t
    {
        Decimal		= 10,
        Hexadecimal = 16,
        Octal		= 8,
        Binary		= 2
    };
    EFormat encoding{EFormat::Decimal};
    SToken	newToken{ETokenCategory::token_integer};

    auto convertToDecimal = [this, &encoding](char character) -> uint32_t
    {
        switch (encoding)
        {
        case EFormat::Hexadecimal:
            return HexToDecimal(character);
        case EFormat::Decimal:
            return DecimalToDecimal(character);
        case EFormat::Octal:
            return OctalToDecimal(character);
        case EFormat::Binary:
            return BinaryToDecimal(character);
        default:
            return 0;
        }
    };

    try
    {
        std::size_t index{0};
        int64_t		sign{1};
        int64_t		value{0};
        if (integerString[0] == '-')
        {
            sign = -1;
            ++index;
        }
        else if (integerString[0] == '+')
        {
            ++index;
        }

        if (integerString[index] == '0' && integerString.size() != index + 1)
        {
            switch (integerString[index + 1])
            {
            case 'x':
                encoding = EFormat::Hexadecimal;
                break;
            case 'o':
                encoding = EFormat::Octal;
                break;
            case 'b':
                encoding = EFormat::Binary;
                break;
            default:
                throw XTOMLParseException("No leading zeros allowed!");
            }
            index += 2;
        }

        for (std::size_t i = index; i < integerString.size(); ++i)
        {
            if (integerString[i] == '_')
            {
                if (i == index || i == integerString.size() - 1)
                {
                    throw XTOMLParseException("Underscore has to be enclosed in digits!");
                }
                continue;
            }
            value = value * static_cast<int64_t>(encoding) + convertToDecimal(integerString[i]);
        }
        newToken.iContentInteger = sign * value;
    }
    catch (const sdv::toml::XTOMLParseException& e)
    {
        newToken.eCategory	   = ETokenCategory::token_error;
        newToken.ssContentString = e.what();
    }
    m_vecTokens.push_back(newToken);
    if (m_stackExpectations.top() == EExpectation::expect_value_once)
    {
        m_stackExpectations.pop();
    }
}

bool CLexerTOML::IsFloat()
{
    if (m_stackExpectations.top() == EExpectation::expect_key)
    {
        return false;
    }
    bool		isFloat	   = false;
    std::string floatChars = m_reader.PeekUntil(m_vecValueDelimiters);

    if (floatChars == "inf" || floatChars == "+inf" || floatChars == "-inf" || floatChars == "nan"
        || floatChars == "+nan" || floatChars == "-nan")
    {
        return true;
    }

    for (const char& character : floatChars)
    {
        if ((character < '0' || character > '9') && character != '_' && character != '-' && character != '+'
            && character != 'e' && character != 'E' && character != '.')
            // if (!((c >= 0x30 && c <= 0x39) || c == '_' || c == '-' || c == '+' || c == 'e' || c == 'E' || c ==
            // '.'))
        {
            return false;
        }
        if (character == 'e' || character == '.' || character == 'E')
        {
            isFloat = true;
        }
    }

    return isFloat;
}

void CLexerTOML::ReadFloat()
{
    std::string floatingpointString = m_reader.ConsumeUntil(m_vecValueDelimiters);
    SToken		newToken{ETokenCategory::token_float};

    floatingpointString.erase(std::remove(floatingpointString.begin(), floatingpointString.end(), '_'),
        floatingpointString.end());

    size_t dotPosition = floatingpointString.find('.');
    if (dotPosition == 0 || (dotPosition == floatingpointString.size() - 1)
        || (dotPosition != std::string::npos
            && ((floatingpointString[dotPosition - 1] < '0' || floatingpointString[dotPosition - 1] > '9')
                || (floatingpointString[dotPosition + 1] < '0' || floatingpointString[dotPosition + 1] > '9'))))
    {
        newToken.eCategory	   = ETokenCategory::token_error;
        newToken.ssContentString = "The '.' has to be between two digits";
    }

    try
    {
        newToken.dContentFloatingpoint = std::stod(floatingpointString);
    }
    catch ([[maybe_unused]] const std::exception& e)
    {
        newToken.eCategory	   = ETokenCategory::token_error;
        newToken.ssContentString = "Unable to convert '" + floatingpointString + "' to floating point value";
    }
    m_vecTokens.push_back(newToken);
    if (m_stackExpectations.top() == EExpectation::expect_value_once)
    {
        m_stackExpectations.pop();
    }
}

bool CLexerTOML::IsBool()
{
    if (m_stackExpectations.top() == EExpectation::expect_key)
    {
        return false;
    }
    std::string boolChars = m_reader.PeekUntil(m_vecValueDelimiters);
    return (boolChars == "true" || boolChars == "false");
}

void CLexerTOML::ReadBool()
{
    SToken		newToken{ETokenCategory::token_boolean};
    std::string boolChars = m_reader.ConsumeUntil(m_vecValueDelimiters);
    if (boolChars == "true")
    {
        newToken.bContentBoolean = true;
    }
    else if (boolChars == "false")
    {
        newToken.bContentBoolean = false;
    }
    else
    {
        newToken.eCategory = ETokenCategory::token_error;
    }
    m_vecTokens.push_back(newToken);
    if (m_stackExpectations.top() == EExpectation::expect_value_once)
    {
        m_stackExpectations.pop();
    }
}

bool CLexerTOML::IsWhitespace()
{
    return m_reader.Peek()[0] == ' ' || m_reader.Peek()[0] == '\t';
}

void CLexerTOML::ReadWhitespace()
{
    // Only read whitespace
    m_reader.Consume();
}

bool CLexerTOML::IsSyntaxElement()
{
    switch (m_reader.Peek()[0])
    {
    case '\n':
    case '[':
    case ']':
    case '{':
    case '}':
    case ',':
    case '.':
    case '=':
        return true;
    case '\r':
        return (m_reader.Peek(2)[0] == '\n');
    default:
        return false;
    }
}

void CLexerTOML::ReadSyntaxElement()
{
    SToken newToken;
    switch (m_reader.Consume()[0])
    {
    case '\n':
        newToken.eCategory = ETokenCategory::token_syntax_new_line;
        break;
    case '\r':
        m_reader.Consume();
        newToken.eCategory = ETokenCategory::token_syntax_new_line;
        break;
    case '[':
        if (m_stackExpectations.top() != EExpectation::expect_key)
        {
            newToken.eCategory = ETokenCategory::token_syntax_array_open;
            m_stackExpectations.push(EExpectation::expect_value);
        }
        else
        {
            if (m_reader.Peek() == "[")
            {
                newToken.eCategory = ETokenCategory::token_syntax_table_array_open;
                m_reader.Consume();
            }
            else
            {
                newToken.eCategory = ETokenCategory::token_syntax_table_open;
            }
        }
        break;
    case ']':
        if (m_stackExpectations.top() != EExpectation::expect_key)
        {
            newToken.eCategory = ETokenCategory::token_syntax_array_close;
            m_stackExpectations.pop();
            if (m_stackExpectations.top() == EExpectation::expect_value_once)
            {
                m_stackExpectations.pop();
            }
        }
        else
        {
            if (m_reader.Peek() == "]")
            {
                newToken.eCategory = ETokenCategory::token_syntax_table_array_close;
                m_reader.Consume();
            }
            else
            {
                newToken.eCategory = ETokenCategory::token_syntax_table_close;
            }
        }
        break;
    case '{':
        newToken.eCategory = ETokenCategory::token_syntax_inline_table_open;
        m_stackExpectations.push(EExpectation::expect_key);
        break;
    case '}':
        newToken.eCategory = ETokenCategory::token_syntax_inline_table_close;
        m_stackExpectations.pop();
        if (m_stackExpectations.top() == EExpectation::expect_value_once)
        {
            m_stackExpectations.pop();
        }
        break;
    case ',':
        newToken.eCategory = ETokenCategory::token_syntax_comma;
        break;
    case '.':
        newToken.eCategory = ETokenCategory::token_syntax_dot;
        break;
    case '=':
        newToken.eCategory = ETokenCategory::token_syntax_assignment;
        m_stackExpectations.push(EExpectation::expect_value_once);
        break;
    default:
        newToken.eCategory = ETokenCategory::token_error;
    }
    m_vecTokens.push_back(newToken);
}

bool CLexerTOML::IsComment()
{
    return m_reader.Peek()[0] == '#';
}

void CLexerTOML::ReadComment()
{
    m_reader.ConsumeUntil({"\n"});
}

void CLexerTOML::ReadUnknownSequence()
{
    std::string sequence = m_reader.ConsumeUntil(m_vecValueDelimiters);
    SToken		newToken{ETokenCategory::token_error};
    newToken.ssContentString = "Invalid Sequence '" + sequence + "'";
    m_vecTokens.push_back(newToken);
    if (m_stackExpectations.top() == EExpectation::expect_value_once)
    {
        m_stackExpectations.pop();
    }
}

std::string CLexerTOML::Unescape()
{
    std::string escapeChar = m_reader.Consume();
    switch (escapeChar[0])
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
        return Unicode4DigitToUTF8();
    case 'U':
        return Unicode8DigitToUTF8();
    default:
        throw XTOMLParseException(("Invalid escape sequence: \\" + escapeChar).c_str());
    }
}

std::string CLexerTOML::Unicode4DigitToUTF8()
{
    static constexpr uint8_t numDigits{4};
    return UnicodeToUTF8(numDigits);
}

std::string CLexerTOML::Unicode8DigitToUTF8()
{
    static constexpr uint8_t numDigits{8};
    return UnicodeToUTF8(numDigits);
}

std::string CLexerTOML::UnicodeToUTF8(uint8_t numCharacters)
{
    // Read the characters
    uint32_t uiUTFVal = 0;
    for (int i = 1; i <= numCharacters; ++i)
    {
        uiUTFVal = uiUTFVal * 16 + HexToDecimal(m_reader.Consume()[0]);
    }

    // One byte UTF-8 character
    if (uiUTFVal < 0x80)
        return std::string("") + static_cast<char>(uiUTFVal);

    // Two byte UTF-8 character
    if (uiUTFVal < 0x800)
        return std::string("") + static_cast<char>(uiUTFVal >> 6 | 0xc0) + static_cast<char>((uiUTFVal & 0b111111) | 0x80);

    // Three byte UTF-8 character
    if (uiUTFVal < 0x10000)
        return std::string("") + static_cast<char>(uiUTFVal >> 12 | 0xe0) + static_cast<char>(((uiUTFVal >> 6) & 0b111111) | 0x80) +
        static_cast<char>((uiUTFVal & 0b111111) | 0x80);

    // Four byte UTF-8 character
    if (uiUTFVal < 0x110000)
    {
        return std::string("") + static_cast<char>(uiUTFVal >> 18 | 0xf0) + static_cast<char>(((uiUTFVal >> 12) & 0b111111) | 0x80) +
            static_cast<char>(((uiUTFVal >> 6) & 0b111111) | 0x80) + static_cast<char>((uiUTFVal & 0b111111) | 0x80);
    }
    std::stringstream message("Invalid 8 digit unicode: ");
    message << std::hex << std::to_string(uiUTFVal) << std::dec;
    throw XTOMLParseException(message.str());
}

uint32_t CLexerTOML::HexToDecimal(const char character)
{
    static constexpr int8_t valueOfA{10};
    if (character >= '0' && character <= '9')
    {
        return character - '0';
    }
    if (character >= 'A' && character <= 'F')
    {
        return character - ('A' - valueOfA);
    }
    if (character >= 'a' && character <= 'f')
    {
        return character - ('a' - valueOfA);
    }
    throw XTOMLParseException((std::string("Invalid hex character: ") + character + "("
        + std::to_string(static_cast<int32_t>(character)) + ")")
        .c_str());
}

uint32_t CLexerTOML::DecimalToDecimal(const char character)
{
    if (character >= '0' && character <= '9')
    {
        return character - '0';
    }
    throw XTOMLParseException((std::string("Invalid decimal character: ") + character
        + std::string("(")
        + std::to_string(static_cast<int32_t>(character)) + ")")
        .c_str());
}

uint32_t CLexerTOML::OctalToDecimal(const char character)
{
    if (character >= '0' && character <= '7')
    {
        return character - '0';
    }
    throw XTOMLParseException((std::string("Invalid octal character: ") + character + "("
        + std::to_string(static_cast<int32_t>(character)) + ")")
        .c_str());
}

uint32_t CLexerTOML::BinaryToDecimal(const char character)
{
    if (character == '0' || character == '1')
    {
        return character - '0';
    }
    throw XTOMLParseException((std::string("Invalid binary character: ") + character + "("
        + std::to_string(static_cast<int32_t>(character)) + ")")
        .c_str());
}

