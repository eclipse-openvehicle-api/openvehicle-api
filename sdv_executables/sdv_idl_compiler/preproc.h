#ifndef PREPROC_H
#define PREPROC_H

#include "token.h"
#include "codepos.h"
#include "environment.h"
#include <filesystem>

// Forward declaration
class CLexer;
class CParser;

/**
 * @brief Preprocessing class
 */
class CPreprocessor
{
public:
    /**
     * @brief Constructor
     * @param[in] rParser Reference to the parser.
     */
    CPreprocessor(CParser& rParser);

    /**
     * @brief Process preprocessor line
     * @param[in] rCode Reference to the code holding the line the might contain a preprocessor direction.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     * @return Returns a meta token if one is available; an empty token otherwise.
     */
    CToken ProcessPreproc(CCodePos& rCode, const CContextPtr& rptrContext);

    /**
     * @brief Checks whether currently a conditional section is being processed. If so, this function will trigger an exception
     * for a missing @c \#endif' directive.
     * @param[in] rCode Reference to the code holding the line the might contain a preprocessor direction.
     */
    void FinalProcessing(const CCodePos& rCode);

    /**
     * @brief Processing enabled. Part of conditional inclusion.
     * @remarks If processing is disabled, lines should be skipped. Preprocessor directives should be processed anyway.
     * @return Returns whether to process current code.
     */
    bool CurrentSectionEnabled() const;

    /**
     * @brief Include a file. Function prototype to be implemented by the parser.
     * @param[in] rpath Reference to the source file path. Must be a valid path.
     * @param[in] bLocal When set, the include file is a local include. Otherwise it represents a global include.
     */
    virtual void IncludeFile(const std::filesystem::path& rpath, bool bLocal) = 0;

    /**
     * @brief Get the environment. Function prototype to be implemented by the parser.
     * @return Reference to the environment.
     */
    virtual CIdlCompilerEnvironment& GetEnvironment() = 0;

private:
    /**
     * @brief Process the define macro directive.
     * @details The code contains the 'define' directive followed by a macro name and optionally by zero or more parameters
     * between parenthesis and optionally the value the macro should be replaced with. The value could contain the operator '#'
     * preceding the parameter to stringificate the parameter content and the operator '##' preceding or succeeding the parameter
     * to concatenate the result to the identifier adjacent identifier.
     * @param[in, out] rLexer Reference to the lexer to use for parsing.
     * @param[in, out] rCode Reference to the code following the define keyword.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ProcessDefine(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext);

    /**
     * @brief Process the undef macro directive.
     * @details The code contains the 'undef' directive followed by a macro name.
     * @param[in, out] rLexer Reference to the lexer to use for parsing.
     * @param[in, out] rCode Reference to the code following the define keyword.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ProcessUndef(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext);

    /**
     * @brief Process the include directive.
     * @details The code contains 'include' directive followed by the file to include between quotes or between angle-brackets.
     * Filenames between quotes can be relative using the current directory as a start or absolute. First the current directory
     * is used to search for the file, then the directories of the provided search path is used to search for the file. Filenames
     * between angle-brackets must be relative. The search path is used to search for the file.
     * @param[in, out] rLexer Reference to the lexer to use for parsing.
     * @param[in, out] rCode Reference to the code following the define keyword.
     * @param[out] rbLocal When set, the include statement was referring to a local file; otherwise global include.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ProcessInclude(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext, bool& rbLocal);

    /**
     * @brief Process the if directive.
     * @details The code contains the 'if' directive followed by a conditional expression.
     * @param[in, out] rLexer Reference to the lexer to use for parsing.
     * @param[in, out] rCode Reference to the code following the if or elif keyword.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ProcessIf(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext);

    /**
     * @brief Process the ifdef or ifndef directive.
     * @details The code contains the 'ifdef' or the 'ifndef' directive followed by a macro name.
     * @param[in, out] rLexer Reference to the lexer to use for parsing.
     * @param[in, out] rCode Reference to the code following the ifdef or ifndef keyword.
     * @param[in] bInverted When set, the ifndef directive should be used; otherwise the ifdef directive.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ProcessIfDef(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext, bool bInverted);

    /**
     * @brief Process the elif directive.
     * @details The code contains the 'elif' directive followed by a conditional expression.
     * @param[in, out] rLexer Reference to the lexer to use for parsing.
     * @param[in, out] rCode Reference to the code following the if or elif keyword.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ProcessElif(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext);

    /**
     * @brief Process the else directive.
     * @details The code contains the 'else' directive.
     * @param[in, out] rLexer Reference to the lexer to use for parsing.
     * @param[in, out] rCode Reference to the code following the else keyword.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ProcessElse(CLexer& rLexer, const CCodePos& rCode, const CContextPtr& rptrContext);

    /**
     * @brief Process the endif directive.
     * @details The code contains the 'endif' directive.
     * @param[in, out] rLexer Reference to the lexer to use for parsing.
     * @param[in, out] rCode Reference to the code following the endif keyword.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ProcessEndif(CLexer& rLexer, const CCodePos& rCode, const CContextPtr& rptrContext);

    /**
     * @brief Process the verbatim directive.
     * @details The code contains the 'verbatim' directive.
     * @param[in, out] rLexer Reference to the lexer to use for parsing.
     * @param[in, out] rCode Reference to the code following the endif keyword.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ProcessVerbatim(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext);

    /**
     * @brief Process the verbatim block directives.
     * @details The code contains the 'verbatim_begin' and "verbatim_end" directives.
     * @param[in, out] rLexer Reference to the lexer to use for parsing.
     * @param[in, out] rCode Reference to the code following the endif keyword.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     */
    void ProcessVerbatimBlock(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext);

    /**
     * @brief Calculate the conditional value based on the precedence of operators.
     * @details Parse through the condition string and calculate a value. Values and arithmetic operators are read and a result
     * is calculated as long as the operator precedence doesn't undercut the current precedence.
     *  - macros are expanded
     *  - defined operator can be used to check for the existence of a macro
     *  - unknown identifiers are zero
     *  - integers are casted to int64_t
     *  - chars are casted to int64_t
     *  - operators that are allowed + - / % * & | ^ ~ << >> && || ! > >= < <= == !=
     *  - parenthesis are allowed
     *
     * Operator precedence:
     * +------------+-----------------+-------------------------------------------+
     * | precedence | operators       |  description                              |
     * +============+=================+===========================================+
     * |     0      | ~ ! ( ) defined |  Bitwise NOT, logical NOT and parenthesis |
     * +------------+-----------------+-------------------------------------------+
     * |     1      | * / %           |  Multiplication, division, and remainder  |
     * +------------+-----------------+-------------------------------------------+
     * |     2      | + -             |  Addition and subtraction                 |
     * +------------+-----------------+-------------------------------------------+
     * |     3      | << >>           |  Bitwise left shift and right shift       |
     * +------------+-----------------+-------------------------------------------+
     * |     4      | < <= > >=       |  Relational operators                     |
     * +------------+-----------------+-------------------------------------------+
     * |     5      | == !=           |  Equality operators                       |
     * +------------+-----------------+-------------------------------------------+
     * |     6      | &               |  Bitwise AND                              |
     * +------------+-----------------+-------------------------------------------+
     * |     7      | ^               |  Bitwise XOR                              |
     * +------------+-----------------+-------------------------------------------+
     * |     8      | |               |  Bitwise OR                               |
     * +------------+-----------------+-------------------------------------------+
     * |     9      | &&              |  Logical AND                              |
     * +------------+-----------------+-------------------------------------------+
     * |     10     | ||              |  Logical OR                               |
     * +------------+-----------------+-------------------------------------------+
     * @param[in, out] rLexer Reference to the lexer.
     * @param[in, out] rCode Reference to the code. The code position will be updated.
     * @param[in] rptrContext Reference to the smart pointer to the source code context.
     * @param[in] uiPrecedence [in] Current precedence level (default 100).
     * @return Returns the calculated value
     */
    int64_t ConditionalCalc(CLexer& rLexer, CCodePos& rCode, const CContextPtr& rptrContext, uint32_t uiPrecedence = 100);

    /**
     * @brief Conditional inclusion control
     */
    enum class EConditionalInclusion
    {
        if_section,     ///< Within if section: endif, elif, else could be defined
        else_section    ///< Within else section: endif could be defined
    };

    /**
     * @brief Conditional processing control
     */
    enum class EConditionalProcessing
    {
        disabled,       ///< Disabled by a parent conditional section.
        current,        ///< The current conditional section is being processed.
        previous,       ///< A previous conditional section was processed.
        future          ///< A future conditional section might be processed.
    };

    /**
     * @brief Conditional processing control.
     */
    struct SConditionalControl
    {
        EConditionalInclusion   m_eInclusionControl;    ///< The current level of inclusion
        EConditionalProcessing  m_eProcessingControl;   ///< The current level of processing.
    };

    CParser&                            m_rParser;                  ///< Reference to the parser
    std::stack<SConditionalControl>     m_stackConditional;         ///< Stack with the current valid
};


#endif // !defined PREPROC_H
