#include "environment.h"
#include "lexer.h"
#include "codepos.h"
#include "token.h"
#include "codepos.h"
#include "lexer.h"
#include <cuchar>
#include <iostream>
#include <cstdio>
#include <algorithm>

CIdlCompilerEnvironment::CIdlCompilerEnvironment()
{}

CIdlCompilerEnvironment::CIdlCompilerEnvironment(const std::vector<std::string>& rvecArgs) :
    m_cmdln(static_cast<uint32_t>(CCommandLine::EParseFlags::no_assignment_character))
{
    // Create a classic argument array
    std::vector<const char*> vecArgPtrs;
    for (const std::string& rssArg : rvecArgs)
        vecArgPtrs.push_back(rssArg.c_str());

    // Call the constructor
    *this = CIdlCompilerEnvironment(vecArgPtrs.size(), &vecArgPtrs.front());
}

sdv::interface_t CIdlCompilerEnvironment::GetInterface(sdv::interface_id idInterface)
{
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::ICompilerOption>())
        return static_cast<sdv::idl::ICompilerOption*>(this);
    return nullptr;
}

std::filesystem::path CIdlCompilerEnvironment::GetNextFile()
{
    m_iFileIndex++;
    if (static_cast<size_t>(m_iFileIndex) >= m_vecFileNames.size()) return std::filesystem::path();
    return m_vecFileNames[static_cast<size_t>(m_iFileIndex)];
}

void CIdlCompilerEnvironment::AddDefinition(const char* szDefinition)
{
    CCodePos code(szDefinition);
    SLexerDummyCallback sCallback;
    CLexer lexer(&sCallback, CaseSensitiveTypeExtension());

    // Read the macro name
    CToken sNameToken = lexer.GetToken(code, nullptr);
    if (!sNameToken || sNameToken.GetType() != ETokenType::token_identifier)
        throw CCompileException(code.GetLocation(), "Definition name missing.");

    // Check for the next character. If there is no space in between and the next token is a left parenthesis, there are
    // parameters.
    bool bHasParam = false;
    std::vector<std::string> vecParams;
    CToken sSymbolToken;
    if (*code == '(')
    {
        // Get the left parenthesis as a token.
        sSymbolToken = lexer.GetToken(code, nullptr);

         // Parameter bracket has been provided
        bHasParam = true;
        if (sSymbolToken != "(")
            throw CCompileException(sSymbolToken, "Invalid character for macro definition; expecting '('.");

        // Read zero or more parameter
        bool bDone = false;
        bool bInitial = true;
        while (!bDone)
        {
            // Check for closing bracket
            sSymbolToken = lexer.GetToken(code, nullptr);
            if (bInitial && sSymbolToken == ")")
                bDone = true;
            else if (sSymbolToken.GetType() != ETokenType::token_identifier)
                throw CCompileException(sSymbolToken, "Expecting a parameter name.");

            bInitial = false;
            if (!bDone)
            {
                // The token should represent an identifier
                CToken sIdentifierToken = sSymbolToken;

                // Check for duplicates
                if (std::find(vecParams.begin(), vecParams.end(), static_cast<std::string>(sIdentifierToken)) != vecParams.end())
                    throw CCompileException(sSymbolToken, "Duplicate parameter names for macro definition.");

                // Add the parameter to the list
                vecParams.push_back(sIdentifierToken);

                // Check for a comma or a right parenthesis
                sSymbolToken = lexer.GetToken(code, nullptr);
                if (sSymbolToken == ")")
                    bDone = true;
                else if (sSymbolToken != ",")
                    throw CCompileException(sSymbolToken, "Unexpected symbol in parameter list.");
            }
        }
    }

    // Check for an equal sign
    sSymbolToken = lexer.GetToken(code, nullptr);
    std::string ssValue;
    if (sSymbolToken)
    {
        // Expect assignment symbol
        if (sSymbolToken != "=")
            throw CCompileException(sSymbolToken, "Invalid format.");
        ssValue = static_cast<std::string>(code.GetLocation());
    } else
    {
        // Expect no more code
        if (!code.HasEOF())
           throw CCompileException(sSymbolToken, "Invalid format.");
    }

    // Create and store the macro structure
    CMacro macro(static_cast<std::string>(sNameToken).c_str(), bHasParam ? &vecParams : nullptr, ssValue.c_str());

    // Add the macro
    AddDefinition(code.GetLocation(), macro);
}

void CIdlCompilerEnvironment::AddDefinition(const CToken& rtoken, const CMacro& rMacro)
{
    // Check for the macro to exist already; if so and different, throw exception. If not, add to macro map.
    CMacroMap::iterator itMacro = m_mapMacros.find(rMacro.GetName());
    if (itMacro != m_mapMacros.end())
    {
        if (itMacro->second != rMacro)
            throw CCompileException(rtoken, "Redefinition of macro with different content.");
    } else
        m_mapMacros.emplace(std::move(CMacroMap::value_type(rMacro.GetName(), rMacro)));
}

void CIdlCompilerEnvironment::RemoveDefinition(const char* szMacro)
{
    if (!szMacro) return;
    CMacroMap::iterator itMacro = m_mapMacros.find(szMacro);
    if (itMacro != m_mapMacros.end())
        m_mapMacros.erase(itMacro);
}

bool CIdlCompilerEnvironment::Defined(const char* szMacro) const
{
    if (!szMacro) return false;
    return m_mapMacros.find(szMacro) != m_mapMacros.end();
}

bool CIdlCompilerEnvironment::TestAndExpand(const std::string& rssIdentifier, CCodePos& rcode, bool bInMacroExpansion /*= false*/,
    CUsedMacroSet& rsetPreviousExpanded /*= CUsedMacroSet()*/) const
{
    if (rssIdentifier.empty()) return false;

    // Which macro set to use?
    CUsedMacroSet& rsetUsedMacros = rsetPreviousExpanded.empty() ? m_setMacrosUsedInExpansion : rsetPreviousExpanded;

    // If the provided identifier was created as part of a previous macro expansion (bInMacroExpansion is true) or the macro is
    // part of the current expansion (rsetPreviousExpanded is not empty), check whether the identifier defines a macro that was
    // used in the previous expansion, which is not allowed.
    // If no previous expansion took place or current expansion takes place, clear the expansion set.
    if (bInMacroExpansion || !rsetPreviousExpanded.empty())
    {
        if (rsetUsedMacros.find(rssIdentifier) != rsetUsedMacros.end())
            return false;
    } else
        rsetUsedMacros.clear();

    // Test for the existence of a macro.
    CMacroMap::const_iterator itMacro = m_mapMacros.find(rssIdentifier);
    if (itMacro == m_mapMacros.end()) return false;

    // Does the macro need parameters?
    CToken token = rcode.GetLocation();
    std::vector<std::string> vecParams;
    if (itMacro->second.ExpectParameters())
    {
        // Peek for left parenthesis. If not available, this is not an error. Do not deal with the macro
        CCodePos codeTemp(rcode);
        SLexerDummyCallback sCallback;
        CLexer lexer(&sCallback, CaseSensitiveTypeExtension());
        if (lexer.GetToken(codeTemp, nullptr) != "(")
            return false;

        // Get the parenthesis once more.
        lexer.GetToken(rcode, nullptr);

        std::string ssParam;
        CToken locationParam = rcode.GetLocation();
        auto fnTrimCheckAndAdd = [&]()
        {
            // Erase the whitespace from the beginning of the string
            std::string::iterator itStart = std::find_if(ssParam.begin(), ssParam.end(),
                                                         [](char c)
                                                         { return !std::isspace<char>(c, std::locale::classic()); });
            ssParam.erase(ssParam.begin(), itStart);

            // Ersase the whitespace form the end of the string
            std::string::reverse_iterator itEnd = std::find_if(ssParam.rbegin(), ssParam.rend(),
                                                       [](char c)
                                                       { return !std::isspace<char>(c, std::locale::classic()); });
            ssParam.erase(itEnd.base(), ssParam.end());

            // Check for a non-empty string
            if (ssParam.empty())
                throw CCompileException(locationParam, "Missing parameter for macro");

            // Add the parameter to the vector.
            vecParams.emplace_back(std::move(ssParam));

            // Set the token for the new parameter
            locationParam = rcode.GetLocation();
        };

        bool bDone = false;
        size_t nInsideParenthesis = 0;
        while (!bDone)
        {
            switch(*rcode)
            {
            case '(':
                nInsideParenthesis++;
                ssParam += *rcode;
                token = rcode.GetLocation();
                break;
            case ')':
                if (nInsideParenthesis)
                {
                    nInsideParenthesis--;
                    ssParam += *rcode;
                } else
                {
                    fnTrimCheckAndAdd();
                    bDone = true;
                }
                break;
            case ',':
                if (nInsideParenthesis)
                    ssParam += *rcode;
                else
                    fnTrimCheckAndAdd();
                break;
            case '\0':
                throw CCompileException(rcode.GetLocation(), "Unexpected end of file while parsing macro parameters.");
                break;
            default:
                ssParam += *rcode;
                break;
            }
            ++rcode;
        }
    }

    // The macro will be expanded, add it to the expansion set
    rsetUsedMacros.insert(rssIdentifier);

    // Expand the macro and prepend the string to the code.
    rcode.PrependCode(itMacro->second.Expand(*this, token, vecParams, rsetUsedMacros));

    return true;
}

bool CIdlCompilerEnvironment::ResolveConst() const
{
    return m_bResolveConst;
}

bool CIdlCompilerEnvironment::NoProxyStub() const
{
    return m_bNoPS;
}

const std::string& CIdlCompilerEnvironment::GetProxStubCMakeTarget() const
{
    return m_ssProxyStubLibName;
}

sdv::u8string CIdlCompilerEnvironment::GetOption(const sdv::u8string& rssOption) const
{
    return GetOptionN(rssOption, 0);
}

uint32_t CIdlCompilerEnvironment::GetOptionCnt(const sdv::u8string& rssOption) const
{
    if (rssOption.empty()) return 0;
    if (rssOption == sdv::idl::ssOptionDevEnvDir)
        return 1;
    else if (rssOption == sdv::idl::ssOptionOutDir)
        return 1;
    else if (rssOption == sdv::idl::ssOptionFilename)
        return 1;
    else if (rssOption == sdv::idl::ssOptionFilePath)
        return 1;
    else if (rssOption == sdv::idl::ssOptionCodeGen)
        return 10;
    return 0;
}

sdv::u8string CIdlCompilerEnvironment::GetOptionN(const sdv::u8string& rssOption, uint32_t nIndex) const
{
    if (rssOption.empty()) return sdv::u8string();
    if (nIndex >= GetOptionCnt(rssOption)) return sdv::u8string();

    sdv::u8string ssValue;
    if (rssOption == sdv::idl::ssOptionDevEnvDir)
        ssValue = m_pathCompilerPath.parent_path().generic_u8string();
    else if (rssOption == sdv::idl::ssOptionOutDir)
        ssValue = "";
    else if (rssOption == sdv::idl::ssOptionFilename)
        ssValue = "";
    else if (rssOption == sdv::idl::ssOptionFilePath)
        ssValue = "";
    else if (rssOption == sdv::idl::ssOptionCodeGen)
        ssValue = "";
    return ssValue;
}

