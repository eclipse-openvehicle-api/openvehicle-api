#include "parser.h"
#include "exception.h"

CParser::CParser(const char* szCode, const CIdlCompilerEnvironment& renv /*= CIdlCompilerEnvironment()*/) :
    CPreprocessor(*this), m_lexer(this, renv.CaseSensitiveTypeExtension()), m_environment(renv)
{
    // Create soure code context object
    CContextPtr ptrContext = std::make_shared<CContext>(szCode);

    // Add additional keywords to the lexer.
    if (renv.InterfaceTypeExtension())
    {
        m_lexer.AddKeyword("interface_id");
        m_lexer.AddKeyword("interface_t");
        m_lexer.AddKeyword("null");
    }
    if (renv.ExceptionTypeExtension())
    {
        m_lexer.AddKeyword("exception_id");
    }
    if (renv.PointerTypeExtension())
    {
        m_lexer.AddKeyword("pointer");
    }
    if (renv.UnicodeExtension())
    {
        m_lexer.AddKeyword("char16");
        m_lexer.AddKeyword("char32");
        m_lexer.AddKeyword("u8string");
        m_lexer.AddKeyword("u16string");
        m_lexer.AddKeyword("u32string");
    }

    // Parse the source
    ParserPrepare(ptrContext);
}

CParser::CParser(const std::filesystem::path& rpath, const CIdlCompilerEnvironment& renv /*= CIdlCompilerEnvironment()*/) :
    CPreprocessor(*this), m_lexer(this, renv.CaseSensitiveTypeExtension()), m_environment(renv), m_pathFile(rpath)
{
    // Create soure code context object
    CContextPtr ptrContext = std::make_shared<CContext>(rpath);

    // Add additional keywords to the lexer.
    if (renv.InterfaceTypeExtension())
    {
        m_lexer.AddKeyword("interface_id");
        m_lexer.AddKeyword("interface_t");
        m_lexer.AddKeyword("null");
    }
    if (renv.ExceptionTypeExtension())
    {
        m_lexer.AddKeyword("exception_id");
    }
    if (renv.PointerTypeExtension())
    {
        m_lexer.AddKeyword("pointer");
    }
    if (renv.UnicodeExtension())
    {
        m_lexer.AddKeyword("char16");
        m_lexer.AddKeyword("char32");
        m_lexer.AddKeyword("u8string");
        m_lexer.AddKeyword("u16string");
        m_lexer.AddKeyword("u32string");
    }

    // Parse the source
    ParserPrepare(ptrContext);
}

sdv::interface_t CParser::GetInterface(sdv::interface_id idInterface)
{
    // Own interfaces first
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::ICompilerInfo>())
        return static_cast<sdv::idl::ICompilerInfo*>(this);

    // Then ask the environment
    sdv::interface_t ifc = m_environment.GetInterface(idInterface);

    // Then as the root
    if (ifc == nullptr && m_ptrRoot != nullptr)
        ifc = m_ptrRoot->GetInterface(idInterface);

    return ifc;
}

sdv::u8string CParser::GetFilePath() const
{
    return m_pathFile.u8string();
}

sdv::u8string CParser::GetOutputDir() const
{
    return m_environment.GetOutputDir().generic_u8string();
}

const CLexer& CParser::GetLexer() const
{
    return m_lexer;
}

void CParser::IncludeFile(const std::filesystem::path& rpath, bool bLocal)
{
    // Create soure code context object
    CContextPtr ptrContext = std::make_shared<CContext>(rpath,
        bLocal ? sdv::idl::IEntityContext::ELocation::local_include : sdv::idl::IEntityContext::ELocation::global_include);

    // Prepare for parsing the file
    ParserPrepare(ptrContext);
}

CIdlCompilerEnvironment& CParser::GetEnvironment()
{
    return m_environment;
}

CToken CParser::GetToken()
{
    // Check the token cache.
    if (!m_lstCacheTokenList.empty())
    {
        CToken token = m_lstCacheTokenList.front();
        m_lstCacheTokenList.pop_front();
        return token;
    }

    // No token in the cache. Get the token from the lexer.
    CToken token = GetTokenFromLexer(false);

    return token;
}

CToken CParser::GetLastValidToken() const
{
    return m_lexer.GetLastValidToken();
}

CToken CParser::PeekToken(size_t nIndex /*= 0*/)
{
    CToken token;

    // Check the token cache. If not large enough, fill it up.
    while (m_lstCacheTokenList.size() <= nIndex)
    {
        token = GetTokenFromLexer(true);
        if (!token) return token;
        m_lstCacheTokenList.push_back(token);
    }

    CTokenList::iterator itPos = m_lstCacheTokenList.begin();
    for (size_t nCnt = 0; nCnt < nIndex; ++nCnt) ++itPos;
    token = *itPos;
    return token;
}

CTokenList CParser::GetComments()
{
    // If there are no comments, request the next token from the lexer (peeking only); this will request comments.
    if (m_lstComments.empty())
        PeekToken();
    return m_lstComments;
}

void CParser::PrependToken(const CToken& rToken)
{
    // Add the token at the front of to the cache list
    if (!rToken) return;
    m_lstCacheTokenList.push_front(rToken);
}

void CParser::SkipAdjacentComments()
{
    PeekToken();
    uint32_t uiLine = 0;
    while (!m_lstComments.empty())
    {
        const CToken token = m_lstComments.front();
        if (!uiLine || token.GetLine() <= uiLine + 1)
        {
            uiLine = token.GetLine();
            m_lstComments.pop_front();
        }
        else
            break;
    }
}

CParser& CParser::LexicalCheck()
{
    while (GetToken());
    return *this;
}

CParser& CParser::Parse()
{
    // Stack is empty if there is no code available (or everything was parsed already). This is not an error.
    if (m_stackCode.empty()) return *this;

    // Create the root entity.
    m_ptrRoot = std::make_shared<CRootEntity>(*this, m_stackCode.top());
    CRootEntity* pRoot = m_ptrRoot->Get<CRootEntity>();
    if (!pRoot) throw CCompileException("Internal error: cannot get access to the root object.");

    try
    {
        // Parse the root entity.
        pRoot->Process();
    }
    catch (sdv::idl::XCompileError& rexcept)
    {
        // Use the class for easy access.
        CCompileException exception(rexcept);

        // Add the line number if not existing
        if (!exception.GetLineNo())
            exception.SetLocation(m_lexer.GetLastValidToken());

        // Add the path of the code causing the exception to the exception.
        if (exception.GetPath().empty())
            exception.SetPath(m_stackCode.empty() ? m_pathFile : m_stackCode.top()->Source().GetPathRef());

        // Assign the changed values. Needed to keep the additional information that the derived class might have.
        rexcept = exception;

        // Rethrow the original derived class.
        throw;
    }

    return *this;
}

const CRootEntity* CParser::Root() const
{
    return m_ptrRoot ? m_ptrRoot->Get<CRootEntity>() : nullptr;
}

std::string CParser::GenerateAnonymousEntityName(const std::string& rssPrefix)
{
    // Find the prefix in the map
    auto itPrefix = m_mapAutoNameCount.find(rssPrefix);
    if (itPrefix == m_mapAutoNameCount.end()) // Prefix not know; add a new prefix
        itPrefix = m_mapAutoNameCount.emplace(rssPrefix, 0).first;
    return rssPrefix + "_" + std::to_string(itPrefix->second++);
}

std::list<CParser::SMetaToken> CParser::GetAndRemoveMeta()
{
    return std::move(m_lstMeta);
}

void CParser::ParserPrepare(CContextPtr& rptrContext)
{
    // Check whether the file was already added to the parser (do not include the same file multipe times).
    if (m_setProcessedFiles.find(rptrContext->Source().GetPathRef()) != m_setProcessedFiles.end()) return;
    m_setProcessedFiles.insert(rptrContext->Source().GetPathRef());

    // Check for maximum parsing depth.
    if (m_stackCode.size() >= m_nMaxDepth)
        throw CCompileException("Passed maximum amount of nested files - circular inclusion of files?");

    // Add the source code context to the stack
    m_stackCode.push(rptrContext);

    // A new file will be processed. Enable preprocessing on the lexer.
    m_lexer.EnablePreprocProcessing();
}

void CParser::InsertWhitespace(const CToken&)
{}

void CParser::InsertComment(const CToken& rtoken)
{
    // Assign the context to the token (this prevents the context from unloading when the token is still in use).
    CToken tokenComment = rtoken;
    tokenComment.SetContext(m_stackCode.top());

    // Comments can be invalid after the next token is read (in case the next token is in another file). Cache the comment
    // text by requesting the string.
    m_lstComments.push_back(std::move(tokenComment));
}

void CParser::ProcessPreprocDirective(CCodePos& rCode)
{
    // Get the current parse context
    CContextPtr ptrContext = m_stackCode.top();

    // Callback inserting preprocessor directives
    CToken tokenMeta = ProcessPreproc(rCode, ptrContext);
    if (tokenMeta)
    {
        // Check for adjacent preceding comments
        uint32_t uiLine = tokenMeta.GetLine();
        CTokenList lstMetaComments;
        while (m_lstComments.size())
        {
            uint32_t uiCommentLine = m_lstComments.back().GetEndLine();
            if (uiCommentLine < uiLine - 1) break;  // Not connecting to the meta data
            if (uiCommentLine > uiLine) break;      // Not preceding the meta data
            uiLine = uiCommentLine;
            lstMetaComments.push_front(m_lstComments.back());
            m_lstComments.pop_back();
        }

        m_lstMeta.push_back(SMetaToken{ tokenMeta, lstMetaComments });
    }
}

CToken CParser::GetTokenFromLexer(bool /*bPeekOnly*/)
{
    // Clear the comments
    m_lstComments.clear();

    // Check for code
    if (m_stackCode.empty())
        return CToken();

    // Get the token from the code
    CToken token;
    try
    {
        // Get a token as long as no token was found or the stack
        while (!m_stackCode.empty())
        {
            CCodePos &rCode = m_stackCode.top()->Code();

            // If the current position is not part of the macro expansion, reset the set of macros used in a expansion.
            bool bInMacroExpansion = rCode.CurrentPositionInMacroExpansion();

            // Get a token. This will automatically trigger preprocessing control.
            // Assign the context to the token (this prevents the context from unloading when the token is still in use).
            token = m_lexer.GetToken(rCode, m_stackCode.top());

            // Should this code be processed. If not, skip a line and try again...
            if (!CurrentSectionEnabled())
            {
                // Check for EOF
                if (token)  // Not EOF
                    m_lexer.SkipLine(rCode);
                else    // EOF, process the next file
                {
                    // Check for EOF. If so, close the file.
                    // ATTENTION Do not use rCode, since the stack might have changed through a preprocessor include directive.
                    if (m_stackCode.top()->Code().HasEOF())
                    {
                        // Trigger final preproc processing (to check for correct conditional inclusion boundaries).
                        FinalProcessing(m_stackCode.top()->Code());

                        // Remove the top file from the stack
                        m_stackCode.pop();
                    }
                }

                continue;
            }

            // Check whether the token is an identifier, if so, check for any macro
            if (token.GetType() == ETokenType::token_identifier || token.GetType() == ETokenType::token_keyword)
            {
                if (GetEnvironment().TestAndExpand(static_cast<std::string>(token).c_str(), rCode, bInMacroExpansion))
                    continue; // macro was replaced, get a token again.
            }

            // Return the token if one was returned.
            if (token)
                return token;

            // Check for EOF. If so, close the file.
            // ATTENTION Do not use rCode, since the stack might have changed through a preprocessor include directive.
            if (!m_stackCode.empty() && m_stackCode.top()->Code().HasEOF())
            {
                // Trigger final preproc processing (to check for correct conditional inclusion boundaries).
                FinalProcessing(m_stackCode.top()->Code());

                // Remove the top file from the stack
                m_stackCode.pop();
            }
        }
    }
    catch (sdv::idl::XCompileError& rexcept)
    {
        // Use the class for easy access.
        CCompileException exception(rexcept);

        // Add the token to the code if no token was assigned.
        if (token) exception.SetLocation(token);

        // Add the path of the code causing the exception to the exception.
        exception.SetPath(m_stackCode.top()->Source().GetPathRef());

        // Assign the changed values. Needed to keep the additional information that the derived class might have.
        rexcept = exception;

        // Rethrow the original derived class.
        throw;
    }

    // At this point, all files were processed. There is no more token.
    return CToken();
}

