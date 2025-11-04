#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "preproc.h"
#include "source.h"
#include "environment.h"
#include <stack>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include "entities/root_entity.h"
#include "parsecontext.h"

enum class EEntityType : uint32_t
{
    entity_comment,
    entity_include,
    entity_module,
    entity_interface,   // Part of a module
    entity_valuetype,   // Part of an interface
    entity_function,    // part of an interface
    entity_typedef,     // Part of an interface
    entity_eof
};

/**
 * @brief SDV IDL lexer class
 */
class CParser : public CPreprocessor, public ILexerCallback, public sdv::idl::ICompilerInfo, public sdv::IInterfaceAccess
{
public:
    /**
     * @brief Constructor used to inject code directly into the parser (mainly for unit tests).
     * @param[in] szCode Zero terminated string containing the IDL code. Must not be NULL.
     * @param[in] renv Reference to the environment to start using as a base for search dirs and macros.
     * @attention The pointer to the source file code needs to stay valid during the duration of the processing.
     * @throws Throws CCompileException on parse error.
     */
    CParser(const char* szCode, const CIdlCompilerEnvironment& renv = CIdlCompilerEnvironment());

    /**
     * @brief Constructor
     * @param[in] rpath Reference to the source file path. Must be a valid path.
     * @param[in] renv Reference to the environment to start using as a base for search dirs and macros.
     * @throws Throws CCompileException on parse error.
     */
    CParser(const std::filesystem::path& rpath, const CIdlCompilerEnvironment& renv = CIdlCompilerEnvironment());

    /**
     * @brief Default destructor
     */
    virtual ~CParser() = default;

     /**
    * @brief Get access to another interface. Overload of IInterfaceAccess::GetInterface.
    *
    * @param[in] idInterface The interface id to get access to.
    * @return Returns a pointer to the interface or NULL when the interface is not supported.
    */
    virtual sdv::interface_t GetInterface(sdv::interface_id idInterface) override;

    /**
     * @brief Get the path of the processed file. Overload of sdv::idl::ICompilerInfo::GetFilePath.
     * @return The path string.
     */
    virtual sdv::u8string GetFilePath() const override;

    /**
     * @brief Return the path to the output directory. Overload of sdv::idl::ICompilerInfo::GetOutputDir.
     * @return The directory string.
     */
    virtual sdv::u8string GetOutputDir() const override;

    /**
     * @brief Get access to the underlying lexer.
     * @return Reference to the lexer.
     */
    const CLexer& GetLexer() const;

    /**
     * @brief Include a file. Overload of CPreprocessor::IncludeFile.
     * @param[in] rpath Reference to the source file path. Must be a valid path.
     * @param[in] bLocal When set, the include file is a local include. Otherwise it represents a global include.
     */
    virtual void IncludeFile(const std::filesystem::path& rpath, bool bLocal) override;

    /**
     * @brief Get the environment. Overload of CPreprocessor::GetEnvironment.
     * @return Reference to the environment.
     */
    virtual CIdlCompilerEnvironment& GetEnvironment() override;

    /**
     * @brief Get a token from the lexer with moving the current position.
     * @details Check the token cache for any unprocessed tokens. If empty, get a token from the lexer for the currently open
     * file. Preprocessor directives are automatically executed causing conditionally parsing the file, defining and undefining
     * macros and opening include files.
     * @throws Throws CCompileException on parse error.
     * @return Returns the token if there is any or an empty token when there is no token any more.
     */
    CToken GetToken();

    /**
    * @brief Get the last valid token.
     * @return Returns the last read token or an empty token when no token was read before.
    */
    CToken GetLastValidToken() const;

    /**
     * @brief Get a token from the parser without moving the current position.
     * @param[in] nIndex The amount of tokens to skip before returning the token.
     * @return Returns the token if there is any or an empty token when there is no token any more.
     */
    CToken PeekToken(size_t nIndex = 0);

    /**
     * @brief Get the comment list gathered until the next token.
     * @return Token list with comments.
     */
    CTokenList GetComments();

    /**
     * @brief Insert a token in from of the current token cache.
     * @details This function prepends the current token at the current position.
     * @param[in] rToken Reference to token object containing the code to insert.
     */
    void PrependToken(const CToken& rToken);

    /**
     * @brief Skip adjacent comments - this prevents succeeding comments to be interpreted as preceding comments for the next
     * token.
     */
    void SkipAdjacentComments();

    /**
     * @brief Lexical check.
     * @details Parse through the file and belonging include files without interpretation to check for lexical errors.
     * @throws Throws CCompileException on parse error.
     * @return Returns a reference to this parser.
     */
    CParser& LexicalCheck();

    /**
     * @brief Parse the code.
     * @details Parse through the file and belonging include files and build a parse tree.
     * @throws Throws CCompileException on parse error.
     * @return Returns a reference to this parser.
     */
    CParser& Parse();

    /**
     * @brief Get the root entity (after parsing).
     * @return Pointer to the root entity or NULL when there is no root entity.
     */
    const CRootEntity* Root() const;

    /**
     * @brief Generate a name for anonymous entities (structs, unions and enums with declarations can be anonymous; unions within
     * structs can be transparent - having an anonymous declaration).
     * @param[in] rssPrefix The prefix to use to generate an automatic name.
     * @return Returns the name of the anonymous entity consisting of a name part "prefix_" with a unique number.
     */
    std::string GenerateAnonymousEntityName(const std::string& rssPrefix);

    /**
     * @brief Structure containing the meta data token
    */
    struct SMetaToken
    {
        CToken      tokenMeta;      ///< Meta data token
        CTokenList  lstComments;    ///< If available list of preceding comments.
    };

    /**
     * @brief Get and remove the meta data collected until this point.
     * @return List of meta tokens.
     */
    std::list<SMetaToken> GetAndRemoveMeta();

private:
    /**
     * @brief Prepare the parsing
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ParserPrepare(CContextPtr& rptrContext);

    /**
     * @brief Insert whitespace. Overload of ILexerCallback::InsertWhitespace.
     * @param[in] rtoken Reference to the token structure pointing to the whitespace.
     */
    virtual void InsertWhitespace(const CToken &rtoken) override;

    /**
     * @brief Insert a comment, either standalone, belonging to the last statement or belonging to the next statement. Overload
     * of ILexerCallback::InsertComment.
     * @param[in] rtoken Reference to the token structure pointing to the comment.
     */
    virtual void InsertComment(const CToken& rtoken) override;

    /**
     * @brief Process a preprocessor directive. Overload of ILexerCallback::ProcessPreprocDirective.
     * @param[in] rCode Reference to the code.
     */
    virtual void ProcessPreprocDirective(CCodePos& rCode) override;

    /**
     * @brief Get a token from the lexer with moving the current position.
     * @details Get a token from the lexer for the currently open file. Preprocessor directives are automatically executed causing
     * conditionally parsing the file, defining and undefining macros and opening include files.
     * @throws Throws CCompileException on parse error.
     * @param[in] bPeekOnly When set, the function doesn't throw an error, but only returns an empty token when no code is
     * available any more.
     * @return Returns the token if there is any or an empty token when there is no token any more.
     */
    CToken GetTokenFromLexer(bool bPeekOnly);

    CLexer                                      m_lexer;                    ///< The lexer.
    std::stack<CContextPtr>                     m_stackCode;                ///< Stack with source code.
    std::set<std::filesystem::path>             m_setProcessedFiles;        ///< Set with processed files to prevent recurrency.
    std::vector<std::filesystem::path>          m_vecSearchDirs;            ///< Global search directories.
    const size_t                                m_nMaxDepth = 48;           ///< At the most 48 files can be nested.
    CIdlCompilerEnvironment                                m_environment;              ///< The environment that is used.
    CEntityPtr                                  m_ptrRoot;                  ///< Root entity pointer.
    std::map<std::string, size_t>               m_mapAutoNameCount;         ///< Automatic name generator counter (for a specific
                                                                            ///< prefix).
    CTokenList                                  m_lstCacheTokenList;        ///< Token list with pre-inserted tokens before the
                                                                            ///< current position.
    std::filesystem::path                       m_pathFile;                 ///< Path to the file.
    CTokenList                                  m_lstComments;              ///< Comments are collected. Collection starts newly
                                                                            ///< after each token.
    std::list<SMetaToken>                       m_lstMeta;                  ///< List of meta tokens and preceding comments.
};

#endif // !defined PARSER_H