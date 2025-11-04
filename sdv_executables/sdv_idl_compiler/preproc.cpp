#include "preproc.h"
#include "lexer.h"
#include "parser.h"
#include "exception.h"
#include "source.h"
#include <functional>
#include "logger.h"

CPreprocessor::CPreprocessor(CParser& rParser) : m_rParser(rParser)
{}

CToken CPreprocessor::ProcessPreproc(CCodePos& rCode, const CContextPtr& rptrContext)
{
    // Use a local copy of the code (used when parsing preprocessor commands)
    CCodePos codePreproc(rCode);
    SLexerDummyCallback sCallback;
    CLexer lexerPreproc(&sCallback, m_rParser.GetEnvironment().CaseSensitiveTypeExtension(), CLexer::ELexingMode::lexing_preproc);
    CToken tokenMeta;

    // // Check for a preprocessor directive
    // CToken sPreprocLine = m_rParser.GetLexer().HasPreprocessor(rCode);
    // if (!sPreprocLine) return;

    // Expecting the number sign
    if (lexerPreproc.GetToken(codePreproc, rptrContext) != "#")
        throw CCompileException(codePreproc.GetLocation(), "Expected preprocessor directive sign.");

    // Get an identifier
    CToken sKeyword = lexerPreproc.GetToken(codePreproc, rptrContext);
    if (sKeyword == "define")
    {
        // Skip whitespace (space and tab)
        while (*rCode == ' ' || *rCode == '\t') ++rCode;

        tokenMeta = codePreproc.GetLocation(ETokenType::token_meta);
        tokenMeta.SetContext(rptrContext);
        ProcessDefine(lexerPreproc, codePreproc, rptrContext);
        codePreproc.UpdateLocation(tokenMeta, ETokenMetaType::token_meta_define);
    }
    else if (sKeyword == "if")
    {
        ProcessIf(lexerPreproc, codePreproc, rptrContext);
    }
    else if (sKeyword == "ifdef")
    {
        ProcessIfDef(lexerPreproc, codePreproc, rptrContext, false);
    }
    else if (sKeyword == "ifndef")
    {
        ProcessIfDef(lexerPreproc, codePreproc, rptrContext, true);
    }
    else if (sKeyword == "elif")
    {
        ProcessElif(lexerPreproc, codePreproc, rptrContext);
    }
    else if (sKeyword == "else")
    {
        ProcessElse(lexerPreproc, codePreproc, rptrContext);
    }
    else if (sKeyword == "endif")
    {
        ProcessEndif(lexerPreproc, codePreproc, rptrContext);
    }
    else if (sKeyword == "undef")
    {
        // Skip whitespace (space and tab)
        while (*rCode == ' ' || *rCode == '\t') ++rCode;

        tokenMeta = codePreproc.GetLocation(ETokenType::token_meta);
        tokenMeta.SetContext(rptrContext);
        ProcessUndef(lexerPreproc, codePreproc, rptrContext);
        codePreproc.UpdateLocation(tokenMeta, ETokenMetaType::token_meta_undef);
    }
    else if (sKeyword == "include")
    {
        bool bLocal = false;
        tokenMeta = codePreproc.GetLocation(ETokenType::token_meta);
        tokenMeta.SetContext(rptrContext);
        ProcessInclude(lexerPreproc, codePreproc, rptrContext, bLocal);
        codePreproc.UpdateLocation(tokenMeta,
            bLocal ? ETokenMetaType::token_meta_include_local : ETokenMetaType::token_meta_include_global);
    }
    else if (sKeyword == "verbatim")
    {
        // Skip whitespace (space and tab)
        while (*codePreproc == ' ' || *codePreproc == '\t') ++codePreproc;

        // The rest of the line is verbatim text.
        tokenMeta = codePreproc.GetLocation(ETokenType::token_meta);
        tokenMeta.SetContext(rptrContext);
        ProcessVerbatim(lexerPreproc, codePreproc, rptrContext);
        codePreproc.UpdateLocation(tokenMeta, ETokenMetaType::token_meta_verbatim);
    }
    else if (sKeyword == "verbatim_begin")
    {
        // Skip the rest of the line and the newline
        lexerPreproc.SkipLine(codePreproc);
        if (*codePreproc == '\r') ++codePreproc;
        if (*codePreproc == '\n') ++codePreproc;

        // The rest until the end of the block is verbatim.
        tokenMeta = codePreproc.GetLocation(ETokenType::token_meta);
        tokenMeta.SetContext(rptrContext);
        ProcessVerbatimBlock(lexerPreproc, codePreproc, rptrContext);
        codePreproc.UpdateLocation(tokenMeta, ETokenMetaType::token_meta_verbatim);

        // Skip the "#verbatim_end" directive.
        lexerPreproc.SkipLine(codePreproc);
    }
    else if (sKeyword == "verbatim_end")
    {
        // Check whether to ignore directive based on the current section.
        if (CurrentSectionEnabled())
            throw CCompileException(sKeyword, "Preprocessor directive \"#verbatim_end\" without \"verbatim_begin\" found.");
    }
    else if (sKeyword == "pragma")
    {
        // TODO: Add line to code generating target
        // Check whether to ignore directive based on the current section.
        if (CurrentSectionEnabled())
            throw CCompileException(sKeyword, "Unsupported preprocessor directive \"#pragma\" found.");
    }
    else
    {
        // Check whether to ignore directive based on the current section.
        if (CurrentSectionEnabled())
            throw CCompileException(sKeyword, "Unknown preprocessor directive.");
    }

    // Skip the rest of the line
    lexerPreproc.SkipLine(codePreproc);

    // Store the new position
    rCode = codePreproc;

    // Return the meta token if available
    return CurrentSectionEnabled() ? tokenMeta : CToken();
}

void CPreprocessor::FinalProcessing(const CCodePos& rCode)
{
    if (!m_stackConditional.empty())
        throw CCompileException(rCode.GetLocation(), "Missing '#endif' directive.");
}

bool CPreprocessor::CurrentSectionEnabled() const
{
    return m_stackConditional.empty() || m_stackConditional.top().m_eProcessingControl == EConditionalProcessing::current;
}

// Incorrect cppcheck warning concerning using const reference for the rCode parameter. The code position is updated and therefore
// the parameter cannot be a const reference.
// cppcheck-suppress constParameterReference
void CPreprocessor::ProcessDefine(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext)
{
    CLog log("Preprocess #define...");

    // Check whether to ignore directive based on the current section.
    if (!CurrentSectionEnabled())
    {
        log << "Not included in the current section..." << std::endl;
        return;
    }

    // Read the macro name
    CToken tokenName = rLexer.GetToken(rCode, rptrContext);
    if (!tokenName || tokenName.GetType() != ETokenType::token_identifier)
        throw CCompileException(rCode.GetLocation(), "Definition name missing.");

    // Check for the next character. If there is no space in between and the next token is a left parenthesis, there are
    // parameters.
    bool bHasParam = false;
    std::vector<std::string> vecParams;
    CToken tokenSymbol;
    if (*rCode == '(')
    {
        // Get the left parenthesis as a token.
        tokenSymbol = rLexer.GetToken(rCode, rptrContext);

        // Parameter bracket has been provided
        bHasParam = true;
        if (tokenSymbol != "(")
            throw CCompileException(tokenSymbol, "Invalid character for macro definition; expecting '('.");

        // Read zero or more parameter
        bool bDone = false;
        bool bInitial = true;
        while (!bDone)
        {
            // Check for closing bracket
            tokenSymbol = rLexer.GetToken(rCode, rptrContext);
            if (bInitial && tokenSymbol == ")")
                bDone = true;
            else if (tokenSymbol.GetType() != ETokenType::token_identifier)
                throw CCompileException(tokenSymbol, "Expecting a parameter name.");

            bInitial = false;
            if (!bDone)
            {
                // The token should represent an identifier
                CToken sIdentifierToken = tokenSymbol;

                // Check for duplicates
                if (std::find(vecParams.begin(), vecParams.end(), static_cast<std::string>(sIdentifierToken)) != vecParams.end())
                    throw CCompileException(tokenSymbol, "Duplicate parameter names for macro definition.");

                // Add the parameter to the list
                vecParams.push_back(sIdentifierToken);

                // Check for a comma or a right parenthesis
                tokenSymbol = rLexer.GetToken(rCode, rptrContext);
                if (tokenSymbol == ")")
                    bDone = true;
                else if (tokenSymbol != ",")
                    throw CCompileException(tokenSymbol, "Unexpected symbol in parameter list.");
            }
        }
    }

    // Create and store the macro structure
    CToken tokenValue = rCode.GetLocation();
    rLexer.SkipLine(rCode);
    rCode.UpdateLocation(tokenValue);
    CMacro macro(static_cast<std::string>(tokenName).c_str(),
        bHasParam ? &vecParams : nullptr, static_cast<std::string>(tokenValue).c_str());

    // Log information
    log << "Macro definition = " << static_cast<std::string>(tokenName);
    if (bHasParam)
    {
        log << "(";
        bool bInitial = true;
        for (const std::string& rssParam : vecParams)
        {
            if (bInitial) log << ", ";
            log << rssParam;
            bInitial = false;
        }
        log << ")";
    }
    log << " " << static_cast<std::string>(tokenValue) << std::endl;

    // Add the macro
    GetEnvironment().AddDefinition(rCode.GetLocation(), macro);
}

// Incorrect cppcheck warning concerning using const reference for the rCode parameter. The code position is updated and therefore
// the parameter cannot be a const reference.
// cppcheck-suppress constParameterReference
void CPreprocessor::ProcessUndef(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext)
{
    CLog log("Preprocess #undef...");

    // Check whether to ignore directive based on the current section.
    if (!CurrentSectionEnabled())
    {
        log << "Not included in the current section..." << std::endl;
        return;
    }

    // Read the macro name
    CToken tokenName = rLexer.GetToken(rCode, rptrContext);
    if (!tokenName || tokenName.GetType() != ETokenType::token_identifier)
        throw CCompileException(rCode.GetLocation(), "Definition name missing.");

    // Log information
    log << "Macro definition = " << static_cast<std::string>(tokenName) << std::endl;

    GetEnvironment().RemoveDefinition(static_cast<std::string>(tokenName).c_str());
}

void CPreprocessor::ProcessInclude(CLexer& /*rLexer*/, CCodePos& rCode, const CContextPtr& rptrContext, bool& rbLocal)
{
    CLog log("Preprocess #include...");

    // Check whether to ignore directive based on the current section.
    if (!CurrentSectionEnabled())
    {
        log << "Not included in the current section..." << std::endl;
        return;
    }

    // NOTE: The file name could contain characters that might be interpreted as escape character. Do not use the lexer to read
    // the path.

    // Skip whitespace (tab and space)
    while (*rCode == ' ' || *rCode == '\t')
        ++rCode;

    // Get the next symbol
    CToken tokenSymbol = rCode.GetLocation();
    tokenSymbol.SetContext(rptrContext);
    switch (*rCode)
    {
    case '<':   rbLocal = false;    break;
    case '\"':  rbLocal = true;    break;
    default:
        throw CCompileException(tokenSymbol, "Invalid symbol.");
    }
    ++rCode;

    // Get the path
    CToken tokenPath = rCode.GetLocation();
    tokenPath.SetContext(rptrContext);
    bool bDone = false;
    while (!bDone)
    {
        switch (*rCode)
        {
        case '\0':
            throw CCompileException(tokenPath, "Unexpected end of file.");
        case '\r':
        case '\n':
            throw CCompileException(tokenPath, "Unexpected end of line.");
        case '\"':
            if (rbLocal)
                bDone = true;
            else
                ++rCode;
            break;
        case '>':
            if (!rbLocal)
                bDone = true;
            else
                ++rCode;
            break;
        default:
            ++rCode;
            break;
        }
    }
    rCode.UpdateLocation(tokenPath);
    if (!tokenPath) throw CCompileException(tokenPath, "No filename supplied.");

    // Skipping closing symbol
    ++rCode;

    // Find the file
    std::filesystem::path pathConfirmedFile;

    // If a local filename has been provided, look locally first
    if (rbLocal)
    {
        // Determine the local path.
        std::filesystem::path pathSourceDir = tokenSymbol.GetContext()->Source().GetPathRef().parent_path();
        std::filesystem::path pathIncludeFile = static_cast<std::string>(tokenPath);
        std::filesystem::path pathLocalInclude = pathIncludeFile.is_absolute() ? pathIncludeFile : pathSourceDir / pathIncludeFile;

        if (std::filesystem::exists(pathLocalInclude))
            pathConfirmedFile = pathLocalInclude;
    }

    // If no file yet, run through the include directories and try to find a fitting file.
    if (pathConfirmedFile.empty())
    {
        std::vector<std::filesystem::path> vecIncludeDirs = GetEnvironment().GetIncludeDirs();
        for (const std::filesystem::path &rpathDir : vecIncludeDirs)
        {
            std::filesystem::path pathFile = rpathDir / static_cast<std::string>(tokenPath);
            if (std::filesystem::exists(pathFile))
            {
                pathConfirmedFile = pathFile;
                break;
            }
        }
    }

    // File found?
    if (pathConfirmedFile.empty()) throw CCompileException(tokenSymbol, "Include file not found.");

    // Log information
    log << "Statement = " << (rbLocal ? "\"" : "<") << static_cast<std::string>(tokenPath) << (rbLocal ? "\"" : ">") << std::endl;
    log << "Path = " << pathConfirmedFile.generic_u8string() << std::endl;

    // Parse the source file
    IncludeFile(pathConfirmedFile, rbLocal);
}

void CPreprocessor::ProcessIf(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext)
{
    CLog log("Preprocess #if...");

    // Check whether to ignore directive based on the current section.
    EConditionalProcessing eProcessingControl = EConditionalProcessing::disabled;
    if (CurrentSectionEnabled())
    {
        // Calculate the conditional value
        CToken token = rCode.GetLocation();
        bool bResult = ConditionalCalc(rLexer, rCode, rptrContext) ? true : false;
        rCode.UpdateLocation(token);
        eProcessingControl = bResult ? EConditionalProcessing::current : EConditionalProcessing::future;

        log << "Condition = " << static_cast<std::string>(token) << std::endl;
        log << "State = " << (bResult ? "currently processed" : "currently not processed");
    } else
        log << "Not included in the current section..." << std::endl;

    // Create a new section control
    m_stackConditional.push(SConditionalControl{EConditionalInclusion::if_section, eProcessingControl});
}

// Incorrect cppcheck warning concerning using const reference for the rCode parameter. The code position is updated and therefore
// the parameter cannot be a const reference.
// cppcheck-suppress constParameterReference
void CPreprocessor::ProcessIfDef(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext, bool bInverted)
{
    CLog log("Preprocess #ifdef...");

    // Check whether to ignore directive based on the current section.
    EConditionalProcessing eProcessingControl = EConditionalProcessing::disabled;
    if (CurrentSectionEnabled())
    {
        // Check for the macro name
        CToken tokenName = rLexer.GetToken(rCode, rptrContext);
        if (!tokenName || tokenName.GetType() != ETokenType::token_identifier)
            throw CCompileException(rCode.GetLocation(), "Definition name missing.");
        bool bDefined = GetEnvironment().Defined(static_cast<std::string>(tokenName).c_str());
        eProcessingControl = (bInverted ? !bDefined : bDefined) ?
            EConditionalProcessing::current : EConditionalProcessing::future;

        log << "Definition name = " << static_cast<std::string>(tokenName) << std::endl;
        log << "Definition " << (bDefined ? "exists" : "doesn't exist") << std::endl;
    } else
        log << "Not included in the current section..." << std::endl;

    // Create a new section control
    m_stackConditional.push(SConditionalControl{EConditionalInclusion::if_section, eProcessingControl});
}

void CPreprocessor::ProcessElif(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext)
{
    CLog log("Preprocess #elif...");

    // Currently in a conditional section?
    if (m_stackConditional.empty())
        throw CCompileException(rCode.GetLocation(), "Unexpected '#elif' directive.");

    // Currently in an else-section?
    if (m_stackConditional.top().m_eInclusionControl == EConditionalInclusion::else_section)
        throw CCompileException(rCode.GetLocation(), "Duplicate '#elif' directive.");

    switch (m_stackConditional.top().m_eProcessingControl)
    {
    case EConditionalProcessing::future:
    {
        CToken token = rCode.GetLocation();
        bool bResult = ConditionalCalc(rLexer, rCode, rptrContext) ? true : false;
        rCode.UpdateLocation(token);
        log << "Condition = " << static_cast<std::string>(token) << std::endl;
        // Check whether the condition is true
        if (bResult)
        {
            m_stackConditional.top().m_eProcessingControl = EConditionalProcessing::current;
            log << "State = currently processed" << std::endl;
        }
        else
        {
            log << "State = currently not processed" << std::endl;
        }
        break;
    }
    case EConditionalProcessing::current:
        m_stackConditional.top().m_eProcessingControl = EConditionalProcessing::previous;
        log << "State = currently not processed" << std::endl;
        break;
    default:    // Do nothing... disabled and previous are valid values.
        log << "Not included in the current section..." << std::endl;
        break;
    }
}

void CPreprocessor::ProcessElse(CLexer& /*rLexer*/, const CCodePos& rCode, const CContextPtr& /*rptrContext*/)
{
    CLog log("Preprocess #else...");

    // Currently in a conditional section?
    if (m_stackConditional.empty())
        throw CCompileException(rCode.GetLocation(), "Unexpected '#else' directive.");

    // Currently in an else-section?
    if (m_stackConditional.top().m_eInclusionControl == EConditionalInclusion::else_section)
        throw CCompileException(rCode.GetLocation(), "Duplicate '#else' directive.");

    // Update the current state
    m_stackConditional.top().m_eInclusionControl = EConditionalInclusion::else_section;
    switch (m_stackConditional.top().m_eProcessingControl)
    {
    case EConditionalProcessing::future:
        m_stackConditional.top().m_eProcessingControl = EConditionalProcessing::current;
        log << "State = currently processed" << std::endl;
        break;
    case EConditionalProcessing::current:
        m_stackConditional.top().m_eProcessingControl = EConditionalProcessing::previous;
        log << "State = currently not processed" << std::endl;
        break;
    default:    // Do nothing... disabled and previous are valid values.
        log << "Not included in the current section..." << std::endl;
        break;
    }
}

void CPreprocessor::ProcessEndif(CLexer& /*rLexer*/, const CCodePos& rCode, const CContextPtr& /*rptrContext*/)
{
    CLog log("Preprocess #endif...");

    // Currently in a conditional section?
    if (m_stackConditional.empty())
        throw CCompileException(rCode.GetLocation(), "Unexpected '#endif' directive.");

    // End the section
    m_stackConditional.pop();
}

// Incorrect cppcheck warning concerning using const reference for the rCode parameter. The code position is updated and therefore
// the parameter cannot be a const reference.
// Older version: const parameter for rLexer is also not possible.
// cppcheck-suppress constParameterReference
// cppcheck-suppress constParameter
void CPreprocessor::ProcessVerbatim(CLexer& rLexer, CCodePos& rCode, const CContextPtr& /*rptrContext*/)
{
    CLog log("Preprocess #verbatim...");

    // Check whether to ignore directive based on the current section.
    if (!CurrentSectionEnabled())
    {
        log << "Not included in the current section..." << std::endl;
        return;
    }

    rLexer.SkipLine(rCode);
}

// Incorrect cppcheck warning concerning using const reference for the rCode parameter. The code position is updated and therefore
// the parameter cannot be a const reference.
// cppcheck-suppress constParameterReference
void CPreprocessor::ProcessVerbatimBlock(CLexer& rLexer, CCodePos& rCode, const CContextPtr& /*rptrContext*/)
{
    CLog log("Preprocess #verbatim_begin - #verbatim_end...");

    // Check whether to ignore directive based on the current section.
    if (!CurrentSectionEnabled())
    {
        log << "Not included in the current section..." << std::endl;
        return;
    }

    // Check until end of code or end of file
    while (true)
    {
        // Check when done
        if (!static_cast<const char*>(rCode)) throw CCompileException(rCode.GetLocation(), "Missing '#verbatim_end' directive.");
        if (!*rCode) throw CCompileException(rCode.GetLocation(), "Missing '#verbatim_end' directive.");

        // Store the current position at the beginning of the line
        CCodePos posBackup = rCode;

        // Check for #verbatim_end (whitespace could exist before and after the number sign '#').
        while (*rCode == ' ' || *rCode == '\t') ++rCode;
        if (*rCode == '#')
        {
            ++rCode;
            while (*rCode == ' ' || *rCode == '\t') ++rCode;
            if (std::strncmp(rCode, "verbatim_end", 12) == 0)
            {
                rCode = posBackup;
                break;
            }
        }

        // Skip the current line
        rLexer.SkipLine(rCode);
        while (*rCode == '\r' || *rCode == '\n') ++rCode;
    }
}

int64_t CPreprocessor::ConditionalCalc(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext, uint32_t uiPrecedence /*= 100*/)
{
    // Read a token
    CToken token = rLexer.GetToken(rCode, rptrContext);

    // Token can be either a left parenthesis, a unary operator (precedence 0), an identifier (defined, macro or unknown), or a
    // number.
    int64_t iLValue = 0;
    if (token == "(")
    {
        // Calculate the content between the parenthesis.
        iLValue = ConditionalCalc(rLexer, rCode, rptrContext);

        // Expecting a closing parenthesis
        token = rLexer.GetToken(rCode, rptrContext);
        if (token != ")")
            throw CCompileException(token, "Expecting a right parenthesis.");
    } else if (token == "!")
    {
        // Calculate the next token an use logical NOT on this.
        iLValue = ConditionalCalc(rLexer, rCode, 0) ? 0 : 1;
    } else if (token == "~")
    {
        // Calculate the next token an use bitwise NOT on this.
        iLValue = ~ConditionalCalc(rLexer, rCode, 0);
    } else if (token == "defined")
    {
        // The 'defined' identifier is used in two ways
        //  - function line: defined(macro)
        //  - unary operator: defined macro
        token = rLexer.GetToken(rCode, rptrContext);
        if (token == "(")
        {
            // Get the macro name
            token = rLexer.GetToken(rCode, rptrContext);
            if (token.GetType() != ETokenType::token_identifier)
                throw CCompileException(token, "Expecting macro name between parenthesis of defined operator");
            iLValue = GetEnvironment().Defined(static_cast<std::string>(token).c_str()) ? 1 : 0;
            token = rLexer.GetToken(rCode, rptrContext);
            if (token != ")") throw CCompileException(token, "Expecting right parenthesis");
        } else
        {
            if (token.GetType() != ETokenType::token_identifier)
                throw CCompileException(token, "Expecting macro name following defined operator");
            iLValue = GetEnvironment().Defined(static_cast<std::string>(token).c_str()) ? 1 : 0;
        }
    } else if (token == "true" || token == "TRUE")
    {
        iLValue = 1;
    } else if (token == "false" || token == "FALSE")
    {
        iLValue = 0;
    } else if (token.GetType() == ETokenType::token_identifier)
    {
        // Test and expand any macros
        if (GetEnvironment().TestAndExpand(token, rCode))
            iLValue = ConditionalCalc(rLexer, rCode, rptrContext, uiPrecedence);
        else
            iLValue = 0;
    } else if (token.GetType() == ETokenType::token_literal)
    {
        // Only integer and character literals can be interpreted. Additional interpretation characters are not supported.
        // Anything other will result in the value 0.
        if (token.c_str()[0] == '\'')
        {
            if (token.c_str()[1] == '\\')
            {
                switch (token.c_str()[2])
                {
                case '\"':  iLValue = static_cast<int64_t>('\"');   break;
                case '\'':  iLValue = static_cast<int64_t>('\'');   break;
                case '?':   iLValue = static_cast<int64_t>('?');    break;
                case '\\':  iLValue = static_cast<int64_t>('\\');   break;
                case 'a':   iLValue = static_cast<int64_t>('a');    break;
                case 'b':   iLValue = static_cast<int64_t>('b');    break;
                case 'f':   iLValue = static_cast<int64_t>('f');    break;
                case 'n':   iLValue = static_cast<int64_t>('n');    break;
                case 'r':   iLValue = static_cast<int64_t>('r');    break;
                case 't':   iLValue = static_cast<int64_t>('t');    break;
                case 'v':   iLValue = static_cast<int64_t>('v');    break;
                default:
                    iLValue = 0;
                    break;
                }
            } else
                iLValue = static_cast<int64_t>(token.c_str()[1]);
        } else
            iLValue = static_cast<int64_t>(std::atoll(token.c_str()));
    } else
        throw CCompileException(token, "Unexpected token '", static_cast<std::string>(token), "'.");

    // Run until all operations at this or any higher precedence have been executed
    bool bDone = false;
    while (!bDone)
    {
        // Peek for the next operator and decide on precedence...
        token = rLexer.GetToken(rCode, rptrContext);

        // If no token, we're done.
        if (!token) return iLValue;

        // If right parenthesis, replace the parenthesis and we're done.
        if (token == ")")
        {
            rCode.PrependCode(token);
            break;
        }

        // Conditionally calculate the result based on the supplied operator function and the precedence level.
        auto fnCalculate = [&](std::function<int64_t(int64_t, int64_t)> fnCalculation, uint32_t uiOperatorPrecedence,
            bool bCheckDiv = false)
        {
            // If operator has höhere oder gleiche precedence, replace operator and we're done.
            if (uiOperatorPrecedence >= uiPrecedence)
            {
                rCode.PrependCode(token);
                bDone = true;
                return;
            }

            // If operator has niedriger precedence, get next token and calculate result.
            CToken tokenTemp = token;
            int64_t iRValue = ConditionalCalc(rLexer, rCode, rptrContext, uiOperatorPrecedence);

            // Check for division by zero.
            if (bCheckDiv && !iRValue) throw CCompileException(tokenTemp, "Division by zero.");

            // Calculate the result.
            iLValue = fnCalculation(iLValue, iRValue);
        };

        // Check for the operator and calculate the result
        if (token == "*") { fnCalculate([](int64_t i1, int64_t i2){ return i1 * i2; }, 1);        continue; }
        if (token == "/") { fnCalculate([](int64_t i1, int64_t i2){ return i1 / i2; }, 1, true);  continue; }
        if (token == "%") { fnCalculate([](int64_t i1, int64_t i2){ return i1 % i2; }, 1, true);  continue; }
        if (token == "+") { fnCalculate([](int64_t i1, int64_t i2){ return i1 + i2; }, 2);        continue; }
        if (token == "-") { fnCalculate([](int64_t i1, int64_t i2){ return i1 - i2; }, 2);        continue; }
        if (token == "<<") { fnCalculate([](int64_t i1, int64_t i2){ return i1 << i2; }, 3);      continue; }
        if (token == ">>") { fnCalculate([](int64_t i1, int64_t i2){ return i1 >> i2; }, 3);      continue; }
        if (token == "<") { fnCalculate([](int64_t i1, int64_t i2){ return i1 < i2; }, 4);        continue; }
        if (token == "<=") { fnCalculate([](int64_t i1, int64_t i2){ return i1 <= i2; }, 4);      continue; }
        if (token == ">") { fnCalculate([](int64_t i1, int64_t i2){ return i1 > i2; }, 4);        continue; }
        if (token == ">=") { fnCalculate([](int64_t i1, int64_t i2){ return i1 >= i2; }, 4);      continue; }
        if (token == "==") { fnCalculate([](int64_t i1, int64_t i2){ return i1 == i2; }, 5);      continue; }
        if (token == "!=") { fnCalculate([](int64_t i1, int64_t i2){ return i1 != i2; }, 5);      continue; }
        if (token == "&") { fnCalculate([](int64_t i1, int64_t i2){ return i1 & i2; }, 6);        continue; }
        if (token == "^") { fnCalculate([](int64_t i1, int64_t i2){ return i1 ^ i2; }, 7);        continue; }
        if (token == "|") { fnCalculate([](int64_t i1, int64_t i2){ return i1 | i2; }, 8);        continue; }
        if (token == "&&") { fnCalculate([](int64_t i1, int64_t i2){ return i1 && i2; }, 9);      continue; }
        if (token == "||") { fnCalculate([](int64_t i1, int64_t i2){ return i1 || i2; }, 10);     continue; }

        // If not an operator, this is an error
        throw CCompileException(token, "Expecting operator");
    };

    return iLValue;
}

