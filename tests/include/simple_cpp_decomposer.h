#ifndef SIMPLE_CPP_DECOMPOSER_H
#define SIMPLE_CPP_DECOMPOSER_H

#include <list>
#include <string>

/**
 * @brief Decomposes C++ code into text chunks. Supports simple grammar.
*/
class CSimpleCppDecomposer
{
public:
    /**
     * @brief Constructor
     * @param[in] rssCppCode Reference to the source code to decompose.
     */
    CSimpleCppDecomposer(const std::string& rssCppCode);

    /**
     * @brief Get the list of decomposed source code chunks.
     * @return A reference to the list of source code chunks.
     */
    const std::list<std::string>& GetDecomposedCode() const;

    /**
    * @brief Compare with another decomposed code for equality.
    * @param[in] rdecompChunk Reference to the decomposed chunk of code to use for this comparison.
    * @return Returns 'true' when equal; 'false' when not.
    */
    bool operator==(const CSimpleCppDecomposer& rdecompChunk) const;

    /**
    * @brief Compare with another decomposed code for un-equality.
    * @param[in] rdecompChunk Reference to the decomposed chunk of code to use for this comparison.
    * @return Returns 'true' when not equal; 'false' when equal.
    */
    bool operator!=(const CSimpleCppDecomposer& rdecompChunk) const;

private:
    /**
     * @brief Iterate through the source code collecting the source code chunks.
     */
    void Process();

    /**
     * @brief Skip whitespace
     */
    void SkipWhitespace();

    /**
     * @brief Skip the rest of the line.
     */
    void SkipLine();

    /**
     *@brief Skip C-comments.
     */
    void SkipComment();

    /**
     * @brief Process text and numbers
     */
    void ProcessText();

    /**
     * @brief Process strings
    */
    void ProcessString();

    /**
     * @brief Process symbols.
     */
    void ProcessSymbol();

    std::string             m_ssCode;           ///< Current position within the source code.
    size_t                  m_nPos = 0;         ///< The current position.
    std::list<std::string>  m_lstCode;          ///< Decomposed code.
    bool                    m_bNewLine = true;  ///< Set when a new line has occurred (ignoring any whitespace).
};

namespace testing::internal
{
    // Additional helpers for testing C++ code
    GTEST_API_ AssertionResult CmpHelperCPPEQ(const char* szLeftExpression, const char* szRightExpression,
        const char* szLeft, const char* szRight);
    GTEST_API_ AssertionResult CmpHelperCPPNE(const char* szLeftExpression, const char* szRightExpression,
        const char* szLeft, const char* szRight);
    GTEST_API_ AssertionResult CmpHelperCPPEQ(const char* szLeftExpression, const char* szRightExpression,
        const std::string& rssLeft, const std::string& rssRight);
    GTEST_API_ AssertionResult CmpHelperCPPNE(const char* szLeftExpression, const char* szRightExpression,
        const std::string& rssLeft, const std::string& rssRight);
}

#include "simple_cpp_decomposer.inl"

#endif //! SIMPLE_CPP_DECOMPOSER_H