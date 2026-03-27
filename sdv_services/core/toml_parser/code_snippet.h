/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Erik Verhoeven - initial API and implementation
 ********************************************************************************/

#ifndef CODE_SNIPPET_H
#define CODE_SNIPPET_H

#include <list>
#include <string>
#include <memory>
#include <interfaces/toml.h>
#include "lexer_toml.h"

/// The TOML parser namespace
namespace toml_parser
{
    // Forward declaration
    class CGenContext;

    /**
     * @brief Comment or code snippet structure.
     * @details Each node has multiple code snippets used to reproduce the exact code. For example with an assignment:
     * @code
     *
     * # This is out of scope comment before
     *
     * # This is comment before
     *     var_a   =     "abc"  # comment behind
     *                          # more comment behind
     *
     * # This is out of scope comment behind
     *
     * @endcode
     *
     * The code snippets are identified as follows:
     * @code
     * <out of scope comment before>
     * <comment before>
     * <space before><key><space>=<space><value><comment behind>
     * <out of scope comment behind>
     * @endcode
     *
     * Each code snippet has token a list and comment string.
     *  - If the tokenlist is not empty and comment string is empty, the tokenlist determines the comment.
     *  - If the tokenlist is empty, the comment string determines the comment. The following steps are taken during the
     *    GetComment with the raw flag enabled:
     *      - For the comment behind, the current position for the comment is determined by examining the length of the sibling
     *        nodes. If the position is larger than 80, the start position is either 80 or one space character more than the
     *        node length.
     *      - For all other comment types, the start position is zero.
     *          - Stream spaces until the start position is reached; then stream the comment character '#' followed by a space.
     *          - The comment text is streamed word by word. If between the words a space exists, this can be replaced by a
     *            newline when the current position with the new word would exceed column 132. After the new line spaces fill up
     *            until the start position. Then the comment character and a space is streamed followed by more words from the
     *            comment.
     *          - If within the comment a newline is available, this is followed by an additional newline within the raw comment
     *            stream. Also if multiple empty lines occur (lines with no character sor only whitespace), this is also
     *            streamed with one additional newline at the beginning.
     *          - The last word is followed by a newline.
     *  - If tokenlist is not empty and comment string is not empty, the comments plus the newline following within the
     *    tokenlist are replaced by the new comment text like described above, with the exception that the start position is
     *    determined by the current position within the tokenlist.
     */
    class CCodeSnippet
    {
    public:
    
        /**
         * @brief Set the token list containing the code for this code snippet.
         * @param[in] rlstTokens Reference to the token list to be set.
         */
        void SetTokenList(const std::list<CToken>& rlstTokens);
    
        /**
         * @brief Set the token list containing the code for this code snippet.
         * @param[in] rlstTokens Reference to the token list to be set.
         */
        void SetTokenList(std::list<CToken>&& rlstTokens);
    
        /**
         * @brief Mode the code snippet composer should run in.
         */
        enum class EComposeMode
        {
            compose_inline,             ///< Compose as inline whitespace and comment. If there is no token list and no comment
                                        ///< string, compose as one space. If there is only a comment string, insert a space, add
                                        ///< the comment followed by an obligatory newline, and insert spaces until the next
                                        ///< provided position. If there are tokens with a comment token, replace the comment. If
                                        ///< there are tokens without comment, add the comment, newline and spaces.
            compose_before,             ///< Compose as comment assigned to and located before the node. If there is no token list
                                        ///< and no comment string, doesn't add anything. If there is only a comment string, adds
                                        ///< the comment followed by the obligatory newline. If there are tokens with a comment
                                        ///< token, replace the comment. If there are tokens without the comment, place the comment
                                        ///< before the last newline or when not available, at the end of the tokens followed by a
                                        ///< new newline.
            compose_behind,             ///< Compose as comment assigned to and located behind the node. If there is no token list
                                        ///< and no comment string, add a newline. If there is a comment string and no tokens,
                                        ///< add a space, the comment string followed by the obligatory newline. If there is a token
                                        ///< list without comment, add a comment before the newline or at the end with an additional
                                        ///< newline.
            compose_standalone_before,  ///< Compose as stand-alone comment before the node. Replace any token list if a comment
                                        ///< string is available.
            compose_standalone_behind,  ///< Compose as stand-alone comment behind the node. Replace any token list if a comment
                                        ///< string is available.
        };
    
        /**
         * @brief Compose a conde string from the stored tokens and/or string.
         * @details The comment text to code translation is as follows:
         * @code
         * The following text might be split into multiple TOML comment lines.
         * @endcode
         * @code
         * # The following text might be split into
         * # multiple TOML comment lines.
         * @endcode
         * and
         * @code
         * This is part 1 of a text which will be as two separated blocks.
         * This is part 2 of a text which will be as two separated blocks.
         * @endcode
         * @code
         * # This is part 1 of a text which will be as
         * # two separated blocks.
         * #
         * # This is part 2 of a text which will be as
         * # two separated blocks.
         * @endcode
         * and
         * @code
         * This is a list or an indented text, which is streated as a list.
         *   First indented text which belongs as a separated line.
         *   Second indented text which belongs as a separate line.
         * @endcode
         * @code
         * # This is a list or an indented text, which is
         * # streated as a list.
         * #   First indented text which belongs as a
         * #   separated line.
         * #   Second indented text which belongs as a
         * #   separate line.
         * @endcode
         * @param[in] eMode The mode the composer should run in.
         * @param[in] rContext Reference to the node context used for the code generation.
         * @param[in] nAssignmentOffset The offset for a next assignent; only used for inline composition.
         * @param[in] nCommentOffset The offset to insert a multi-line comment; only used for inline and behind composition.
         * @return The composed code string.
         */
        std::string Compose(EComposeMode eMode, const CGenContext& rContext, size_t nAssignmentOffset = 0, size_t nCommentOffset = 0) const;
    
        /**
         * @brief Get the stored comment from the code snippet. A stored comment string supersedes a comment string from the
         * token list. Comment lines that belong together are combined together as a text without line-breaks.
         * @return Returns the stored comment string.
         */
        std::string GetComment() const;
    
        /**
         * @brief Set a comment string. The comment will be formatted in the compose function. Lines that should stay together
         * should not be separated by a line-break. Comments should be provided without the TOML comment character '#'.
         * @param[in] rssComment Reference to the string containing the comment text.
         */
        void SetComment(const std::string& rssComment);

        /**
         * @brief Remove the content of the code snippet.
         */
        void Clear();

        /**
         * @brief Does the snippet contain code?
         * @return Returns whether the snippet has code or not.
         */
        bool HasCode() const;
    
        /**
         * @brief This function will remove the formatting from the code.
         */
        void RemoveFormat();

        /**
         * @brief Does the code snippet have a comma in the token list?
         * @return Returns whether the token list is active and contains a comma token.
         */
        bool HasComma() const;
    
        /**
         * @brief Insert a code snippet before the existing comment.
         * @remarks In case the code has been replaced by a comment text of the current or the to be inserted code snippet, the
         * result will be a combined comment text, disregarding the format.
         * @param[in] rCode Reference to the code snippet to insert before the current code.
         */
        void Insert(const CCodeSnippet& rCode);

        /**
         * @brief Append a code snippet behind the existing comment.
         * @remarks In case the code has been replaced by a comment text of the current or the to be inserted code snippet, the
         * result will be a combined comment text, disregarding the format.
         * @param[in] rCode Reference to the code snippet to appended behind the current code.
         */
        void Append(const CCodeSnippet& rCode);

    private:
        std::list<CToken>   m_lstTokens;                ///< Token list for the code snippet in raw format.
        std::string         m_ssComment;                ///< The comment text for the code snippet in text format.
        bool                m_bCommentReplaced = false; ///< Comment replaced with SetComment function.
        bool                m_bUseDefault = false;      ///< When set, the format was removed from the code.
    };
} // namespace toml_parser

#endif // !defined CODE_SNIPPET_H