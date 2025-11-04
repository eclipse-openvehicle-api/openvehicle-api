#include "macro.h"
#include "exception.h"
#include "lexer.h"
#include "environment.h"
#include <algorithm>

/**
 * @brief Callback class for the lexer allowing to insert the whitespace and comments into the provided stream.
 */
class CMacroResolveCallback : public ILexerCallback
{
public:
    /**
     * @brief Constructor providing a reference of the resolved string to use for concatenating comments and whitepace.
     * @param[in] rssName Reference to the macro name.
     * @param[in] rssTargetValue Reference to the value.
     * @param[in] rbSuppressWhitespace Reference to a boolean stating whether to suppress whitespace.
     */
    CMacroResolveCallback(const std::string& rssName, std::string& rssTargetValue, const bool& rbSuppressWhitespace) :
        m_ssName(rssName), m_rssTargetValue(rssTargetValue), m_rbSuppressWhitespace(rbSuppressWhitespace)
    {}

    /**
     * @brief Insert whitespace. Overload of ILexerCallback::InsertWhitespace.
     */
    virtual void InsertWhitespace(const CToken&) override
    {
        // Multiple spaces are reduced to one space.
        if (!m_rbSuppressWhitespace)
            m_rssTargetValue += ' ';
    }

    /**
     * @brief Insert a comment. Overload of ILexerCallback::InsertComment.
     */
    virtual void InsertComment(const CToken&) override
    {
        // Comments are ignored
    }

    /**
     * @brief Process a preprocessor directive. Overload of ILexerCallback::ProcessPreprocDirective.
     */
    virtual void ProcessPreprocDirective(CCodePos& /*rCode*/) override
    {
        // Should not be called.
        throw CCompileException("Internal error trying to resolve the ", m_ssName, " macro.");
    }

private:
    std::string     m_ssName;               //!< Macro name.
    std::string&    m_rssTargetValue;       //!< Reference to the string to concatenate the comments and whitespace onto.
    const bool&     m_rbSuppressWhitespace; //!< Reference to a boolean stating whether to prevent concatination of whitespace.
};

CMacro::CMacro(const char* szName, const std::vector<std::string>* pvecParams, const char* szValue) :
    m_ssName(szName),
    m_bExpectParams(pvecParams ? true : false),
    m_vecParamDefs(pvecParams ? *pvecParams : std::vector<std::string>()),
    m_ssValue(szValue ? szValue : "")
{
    // Remove any whitespace from the end of the value.
    while (m_ssValue.size() && std::isspace(m_ssValue.back()))
        m_ssValue.erase(m_ssValue.size() - 1);

    // Remove any whitespace from the beginning of the value.
    while (m_ssValue.size() && std::isspace(m_ssValue.front()))
        m_ssValue.erase(0, 1);
}

CMacro::CMacro(const CMacro& rMacro) :
    m_ssName(rMacro.m_ssName),
    m_bExpectParams(rMacro.m_bExpectParams),
    m_vecParamDefs(rMacro.m_vecParamDefs),
    m_ssValue(rMacro.m_ssValue)
{}

CMacro::CMacro(CMacro&& rMacro) noexcept :
    m_ssName(std::move(rMacro.m_ssName)),
    m_bExpectParams(rMacro.m_bExpectParams),
    m_vecParamDefs(std::move(rMacro.m_vecParamDefs)),
    m_ssValue(std::move(rMacro.m_ssValue))
{
    rMacro.m_bExpectParams = false;
}

CMacro& CMacro::operator=(const CMacro& rMacro)
{
    m_ssName = rMacro.m_ssName;
    m_bExpectParams = rMacro.m_bExpectParams;
    m_vecParamDefs = rMacro.m_vecParamDefs;
    m_ssValue = rMacro.m_ssValue;
    return *this;
}

CMacro& CMacro::operator=(CMacro&& rMacro) noexcept
{
    m_ssName = std::move(rMacro.m_ssName);
    m_bExpectParams = rMacro.m_bExpectParams;
    m_vecParamDefs = std::move(rMacro.m_vecParamDefs);
    m_ssValue = std::move(rMacro.m_ssValue);
    rMacro.m_bExpectParams = false;
    return *this;
}

bool CMacro::operator==(const CMacro& rMacro)
{
    if (m_bExpectParams != rMacro.m_bExpectParams) return false;
    if (m_ssName != rMacro.m_ssName) return false;
    if (m_bExpectParams && m_vecParamDefs != rMacro.m_vecParamDefs) return false;
    if (m_ssValue != rMacro.m_ssValue) return false;
    return true;
}

bool CMacro::operator!=(const CMacro& rMacro)
{
    return !operator==(rMacro);
}

const std::string& CMacro::GetName() const
{
    return m_ssName;
}

bool CMacro::ExpectParameters() const
{
    return m_bExpectParams;
}

std::string CMacro::Expand(const CIdlCompilerEnvironment& renv, const CToken& rtoken, const std::vector<std::string>& rvecParams,
    CUsedMacroSet& rsetUsedMacros) const
{
    // Parameters in function like macros are expanded before being inserted into the value if not part of a stringification
    // operation.
    // Commments are ignored in a macro value.
    // Multiple spaces in the value are reduced to one space.
    // The result of the macro is expanded before being returned.
    // Circular references to macros within the expanded code are prevented for
    //  - each paramerer separately with the used macro list provided by the function.
    //  - macro expansion result with the used macros from all the parameters and the used macro list provided to the function.
    // Macros used in the expansion of the paramters and the results are added to the used macro set provided to this function.

    // Check whether the amount of provided params corresponds to the amount of param definitions.
    if (rvecParams.size() < m_vecParamDefs.size())
        throw CCompileException(rtoken, "Missing parameters while calling macro");
    if (rvecParams.size() > m_vecParamDefs.size())
        throw CCompileException(rtoken, "Provided too many parameters while calling macro");

    std::string ssTargetValue;
    bool bConcatenateNext = false;
    bool bStringificateNext = false;
    CMacroResolveCallback callback(m_ssName, ssTargetValue, bConcatenateNext);
    CLexer lexer(&callback, renv.CaseSensitiveTypeExtension(), CLexer::ELexingMode::lexing_preproc);
    CCodePos codeValue(m_ssValue.c_str());

    // The set with used macros for all parameters should stay the same, wheresas the provided set should be extended.
    CUsedMacroSet setUsedMacrosStored = rsetUsedMacros;

    // Stringificate the supplied string.
    auto fnStringificate = [](const std::string &rss)
    {
        std::string ssTarget;
        ssTarget += '\"';
        for (char cVal : rss)
        {
            switch (cVal)
            {
            case '\"':
                ssTarget += "\\\"";
                break;
            case '\'':
                ssTarget += "\\\'";
                break;
            case '\a':
                ssTarget += "\\a";
                break;
            case '\b':
                ssTarget += "\\b";
                break;
            case '\f':
                ssTarget += "\\f";
                break;
            case '\n':
                ssTarget += "\\n";
                break;
            case '\r':
                ssTarget += "\\r";
                break;
            case '\t':
                ssTarget += "\\t";
                break;
            case '\v':
                ssTarget += "\\v";
                break;
            default:
                ssTarget += cVal;
                break;
            }
        }
        ssTarget += '\"';
        return ssTarget;
    };

    // Expand the supplied code
    auto fnExpand = [&](const char* szCode) -> std::string
    {
        // Expand the parameter before inserting into the value.
        std::string ssTarget;
        bool bConcatenateDummy = false;
        CMacroResolveCallback callbackParam(m_ssName, ssTarget, bConcatenateDummy);
        CLexer lexerLocal(&callbackParam, renv.CaseSensitiveTypeExtension(), CLexer::ELexingMode::lexing_preproc);
        CCodePos code(szCode);

        // Create a copy of the set of used macros to provide this to the parameter expansion. These are the stored
        // original used macros.
        CUsedMacroSet setUsedMacrosParam = setUsedMacrosStored;

        // Expand the parameter
        while (!code.HasEOF())
        {
            // If the current position is not part of the macro expansion, reset the set of macros used in a expansion.
            bool bInMacroExpansion = code.CurrentPositionInMacroExpansion();

            // In case the code is not part of the macro expansion, use the stored set of used macros provided to this function.
            // Otherwise the set still contains the macros used in the expansion.
            if (!bInMacroExpansion)
                setUsedMacrosParam = setUsedMacrosStored;

            // Get a token.
            CToken token = lexerLocal.GetToken(code, rtoken.GetContext());

            // Check whether the token is an identifier, if so, check for any macro
            if (token.GetType() == ETokenType::token_identifier)
            {
                // Test and expand the
                if (renv.TestAndExpand(static_cast<std::string>(token).c_str(), code, bInMacroExpansion, setUsedMacrosParam))
                {
                    // Add all the used macros of the parameter expansion to the set of used macros provided to this function.
                    for (const std::string& rssMacro : setUsedMacrosStored)
                        rsetUsedMacros.insert(rssMacro);

                    continue; // macro was replaced, get a token again.
                }
            }

            ssTarget += static_cast<std::string>(token);
        }
        return ssTarget;
    };

    // Parse through the value and deal with parameter names, with stringification operators '#' and concatenating operators '##'.
    while (!codeValue.HasEOF())
    {
        CToken token = lexer.GetToken(codeValue, rtoken.GetContext());

        // Check for concatinating token
        if (token == "##")
        {
            // Concatination and stringification is not allowed to be selected already before.
            if (bConcatenateNext)
                throw CCompileException(rtoken, "Double concatinating operator while resolving macro ", m_ssName);
            if (bStringificateNext)
                throw CCompileException(rtoken, "Cannot stringificate and then concatenate the result while resolving macro ",
                    m_ssName);

            // Remove whitespace from end of the target value
            std::string::reverse_iterator itEnd = std::find_if(ssTargetValue.rbegin(), ssTargetValue.rend(),
                                                               [](char c)
                                                               { return !std::isspace<char>(c, std::locale::classic()); });
            ssTargetValue.erase(itEnd.base(), ssTargetValue.end());

            // Set the concatination flag (this will also prevent concatinating whitespace to the target).
            bConcatenateNext = true;

            // Next processing
            continue;
        }

        // Check for stringification token
        if (token == "#")
        {
            // Multiple stringification tokens following each other is not allowed.
            if (bStringificateNext)
                throw CCompileException(rtoken, "Double stringification operator while resolving macro ", m_ssName);

            // Set the stringification flag and enable concatination to prevent whitespace to be inserted.
            bStringificateNext = true;
            bConcatenateNext = true;

            // Next processing
            continue;
        }

        // Check whether the token represents a parameter name
        if (token.GetType() == ETokenType::token_identifier)
        {
            // Check if the token corresponds to one of the parameters.
            size_t nParamPos = 0;
            for (; nParamPos < m_vecParamDefs.size(); nParamPos++)
                if (m_vecParamDefs[nParamPos] == static_cast<std::string>(token))
                    break;

            if (nParamPos < m_vecParamDefs.size())
            {
                // Add the parameter content to the target. Stringitificate if needed.
                if (bStringificateNext)
                    ssTargetValue += fnStringificate(rvecParams[nParamPos]);
                else
                    ssTargetValue += fnExpand(rvecParams[nParamPos].c_str());
            } else
            {
                // Stringification is not allowed when not using a parameter.
                if (bStringificateNext)
                    throw CCompileException(rtoken, "Cannot stringificate while resolving macro ", m_ssName);

                // Add the token to the target
                ssTargetValue += static_cast<std::string>(token);
            }
        } else
        {
            // Stringification is not allowed when not using a parameter.
            if (bConcatenateNext)
                throw CCompileException(rtoken, "Cannot stringificate while resolving macro ", m_ssName);

            // Add the token to the target
            ssTargetValue += static_cast<std::string>(token);
        }

        // Stringification and concatenating finalized.
        bConcatenateNext = false;
        bStringificateNext = false;
    }

    // The provided set of used macros now contain all the used macros by the parent, this macro and the macros of any of the
    // parameters. Use this set for the expansion of the result - this will prevent circular calling of macros already used
    // in parameters.
    setUsedMacrosStored = rsetUsedMacros;

    // Return the expanded the macro result. This will automatically add any macros expanded in the result to the used macro set.
    return fnExpand(ssTargetValue.c_str());
}

