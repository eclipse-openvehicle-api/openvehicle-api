#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <charconv>

#ifdef __GNUC__
// Some GCC compilers return an overflow warning on string_view functions that use npos as a parameter. This is a known issue
// described here:
// https://stackoverflow.com/questions/79738323/gcc-warning-memcpy-specified-bound-18446744073709551614-exceeds-maximum-objec
// Suppress the warning.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="
#pragma GCC diagnostic ignored "-Wstringop-overread"
#endif

/**
 * @brief Simple TOML parser to read keys and tables. This TOML parser can be used to parse a TOML string without the use of the
 * SDV framework (e.g. before system startup).
 */
namespace sdv::toml::simple_parser
{
    /**
     * @brief Enumeration for all supported TOML node types
     */
    enum class ENodeType
    {
        node_unknown,   ///< Node type not valid.
        node_table,     ///< Node represents a map of key-value pairs
        node_array,     ///< Node represents a list of elements
        node_value      ///< Node represents a scalar value (string, number, boolean, etc.)
    };

    // Forward declaration of the main data structure
    struct SNode;

    /// Table map alias
    using TTableMap  = std::map<std::string, SNode>;

    /// Array vector alias
    using TNodeArray = std::vector<SNode>;

    /**
     * @brief Main variant structure holding the configuration tree data
     */
    struct SNode
    {
        ENodeType       eType = ENodeType::node_unknown;    ///< Enumeration Type
        std::string     ssKey;                              ///< Name of the value key
        std::string     ssValueRaw = "";                    ///< String raw value
        TTableMap       mapTable = {};                      ///< Map Table
        TNodeArray      vecArray = {};                      ///< Vector Array

        bool bIsInline             = false; ///< Locked if defined via inline {...} syntax
        bool bIsExplicitlyHeadered = false; ///< Set to true when defined via [header] syntax

        /**
         * @brief Helper checking if node is valid.
         * @return Returns whether the node is valid (is not unknown).
         */
        operator bool() const noexcept
        {
            return eType != ENodeType::node_unknown;
        }

        /**
         * @brief Helper checking if node is valid.
         * @return Returns whether the node is valid (is not unknown).
         */
        bool IsValid() const noexcept
        {
            return eType != ENodeType::node_unknown;
        }

        /**
         * @brief Helper checking if node is a table.
         * @return Returns whether the node is a table.
         */
        bool IsTable() const noexcept
        {
            return eType == ENodeType::node_table;
        }

        /**
         * @brief Helper checking if node is an array.
         * @return Returns whether the node is an array.
         */
        bool IsArray() const noexcept
        {
            return eType == ENodeType::node_array;
        }

        /**
         * @brief Helper checking if node is a scalar value.
         * @return Returns whether the node is a value node.
         */
        bool IsValue() const noexcept
        {
            return eType == ENodeType::node_value;
        }

        /**
         * @brief Access function returning the type.
         * @return The type of the node.
         */
        ENodeType GetType() const noexcept
        {
            return eType;
        }

        /**
         * @brief Access function returning the key name.
         * @return Reference to the string containing the key name (if not an array member) or an empty string when there is no
         * key name.
         */
        const std::string& GetName() const noexcept
        {
            return ssKey;
        }

        /**
         * @brief Returns the raw value.
         * @return Reference to the string containing the raw value of the node. Or returns an empty string when the node is not
         * a value node or doesn't contain a value.
         */
        const std::string& GetValue() const noexcept
        {
            return ssValueRaw;
        }

        /**
         * @brief Return the value converted to the provided type.
         * @tparam TType Type to convert to. Supported are integral types, bool, floating point types and std::string.
         * @return The result from the conversion or an empty result if the conversion could not be done.
         */
        template <typename TType>
        TType GetValue() const
        {
            if constexpr (std::is_same_v<TType, bool>)
            {
                return ssValueRaw == "true" || GetValue<int>() != 0;
            }
            else if constexpr (std::is_integral_v<TType> || std::is_floating_point_v<TType>)
            {
                TType tValue{};
                auto [ptr, ec] = std::from_chars(ssValueRaw.data(), ssValueRaw.data() + ssValueRaw.size(), tValue);
                return ec == std::errc() ? tValue : TType{};
            }
            else if (std::is_same_v<TType, std::string>)
            {
                return ssValueRaw;
            }
            else
                return {};
        }

        /**
         * @brief Returns the table map.
         * @return If the node is a table, returns a reference to the table map. Or returns an empty map if the node is not a table
         * or doesn't contain child nodes.
         */
        const TTableMap& GetTable() const noexcept
        {
            return mapTable;
        }

        /**
         * @brief Return the array vector.
         * @return If the node is an array, returns a reference to the array vector. Or returns an empty vector if the node is not
         * an array or doesn't contain any elements.
         */
        const TNodeArray& GetArray() const noexcept
        {
            return vecArray;
        }

        /**
         * @brief Safe, non-recursive direct lookup using dot and array notation (e.g., "table.array[1].key")
         * @param[in] svPath String containing the value path to look for.
         * @return Returns a safe, read-only reference to the node of an empty node when not found.
         */
        const SNode GetDirect(std::string_view svPath) const noexcept
        {
            static const SNode sNodeEmpty;
            std::reference_wrapper<const SNode> refCurrentNode = *this;
            size_t nStart = 0; // Index counter for path segments

            while (nStart < svPath.size())
            {
                // 1. Find the next path segment up to the dot delimiter
                size_t nDot = svPath.find('.', nStart);
                std::string_view svPart = svPath.substr(nStart, nDot == std::string_view::npos ? nDot : nDot - nStart);

                if (svPart.empty()) return sNodeEmpty;

                // 2. Check if the segment contains array brackets [...] (e.g., "arr_mixed[4]")
                size_t nOpenBracket = svPart.find('[');
                std::string ssKeyLocal(svPart.substr(0, nOpenBracket));

                // Must be a table node to perform a key lookup
                if (!refCurrentNode.get().IsTable()) return sNodeEmpty;

                auto it = refCurrentNode.get().mapTable.find(ssKeyLocal);
                if (it == refCurrentNode.get().mapTable.end())
                    return sNodeEmpty;

                refCurrentNode = std::cref(it->second);

                // 3. Resolve array indices sequentially (supports multidimensional arrays or inline tables in arrays)
                size_t nBracketPos = nOpenBracket;
                while (nBracketPos != std::string_view::npos)
                {
                    size_t nCloseBracket = svPart.find(']', nBracketPos);
                    if (nCloseBracket == std::string_view::npos)
                        return sNodeEmpty; // Error: Malformed unclosed bracket

                    // Extract and parse the index string substring
                    std::string_view svIndex = svPart.substr(nBracketPos + 1, nCloseBracket - nBracketPos - 1);

                    // Ensure the index consists strictly of digits (prevents negative signs or alpha characters)
                    if (svIndex.empty() ||
                        !std::all_of(svIndex.begin(), svIndex.end(), [](unsigned char c) { return std::isdigit(c); }))
                        return sNodeEmpty;

                    size_t nIdx = 0;
                    try
                    {
                        nIdx = static_cast<size_t>(std::stoull(std::string(svIndex)));
                    }
                    catch (...)
                    {
                        return sNodeEmpty; // Safety catch against integer overflow attacks during parsing
                    }

                    // SNode type verification and strict bounds check (Safety-critical constraint)
                    if (!refCurrentNode.get().IsArray() || nIdx >= refCurrentNode.get().vecArray.size())
                        return sNodeEmpty;

                    // Advance target reference directly into the array index element
                    refCurrentNode = std::cref(refCurrentNode.get().vecArray[nIdx]);

                    // Check for a consecutive opening bracket immediately following (multidimensional arrays)
                    nBracketPos = svPart.find('[', nCloseBracket);
                }

                // Step over the dot delimiter to move onto the next token sequence
                if (nDot == std::string_view::npos) break;
                nStart = nDot + 1;
            }
            return refCurrentNode;
        }
    };

    /**
     * @brief Main processing unit containing the parsing logic
     */
    class CParser
    {
    public:
        /**
         * @brief Explicit constructor initializing the parser target view bounds
         * @param[in] svInput The TOML string.
         */
        // cppcheck-suppress passedByValue
        CParser(std::string_view svInput) : m_svSrc(svInput), m_nPos(0)
        {
            Parse();
        }

        /**
         * @brief Get the root node.
         * @return Return a reference to the root node.
         */
        const SNode& Root() const noexcept
        {
            return m_sRoot;
        }

    private:
        std::string_view    m_svSrc;   ///< Member String View
        size_t              m_nPos;    ///< Member Numerical index)
        SNode               m_sRoot;   ///< Member Object structure
        /// Safety Extension: Stores dot-joined path strings of tables defined explicitly via [headers]
        std::vector<std::string> m_vecExplicitlyDefinedTables;

        /**
         * @brief Executes the lexical analysis and builds the root node tree
         * @return The root node.
         */
        SNode Parse()
        {
            m_sRoot.eType = ENodeType::node_table;
            std::vector<std::string> vCurrentTablePath;

            while (!IsEof())
            {
                // Only skip inline whitespace and comments, preserve newlines to check line boundaries
                while (!IsEof())
                {
                    char c = Peek();
                    if (c == ' ' || c == '\t' || c == '\r')
                    {
                        Consume();
                    }
                    else if (c == '#')
                    {
                        while (!IsEof() && Peek() != '\n' && Peek() != '\r')
                            Consume();
                    }
                    else
                    {
                        break;
                    }
                }
                if (IsEof())
                    break;

                char cNext = Peek();
                if (cNext == '\n')
                {
                    Consume(); // Valid empty line separation
                    continue;
                }

                if (cNext == '[')
                {
                    vCurrentTablePath = ParseTableHeader();
                }
                else
                {
                    SNode& oActiveTable = NavigateToTable(vCurrentTablePath);
                    ParseKeyValue(oActiveTable);
                }

                // Safety Constraint: A key-value assignment or header MUST be followed immediately
                // by a newline, a comment, or the end of the file. No trailing gibberish allowed on the same line.
                while (!IsEof() && (Peek() == ' ' || Peek() == '\t'))
                    Consume();
                if (!IsEof() && Peek() == '#')
                {
                    while (!IsEof() && Peek() != '\n' && Peek() != '\r')
                        Consume();
                }
                if (!IsEof() && Peek() != '\n' && Peek() != '\r')
                {
                    throw std::runtime_error("Multiple declarations on a single line are strictly invalid in TOML.");
                }
            }
            return m_sRoot;
        }

        /**
         * @brief Internal safety helper navigating down a string path to return a mutable reference. Automatically diverts into the
         * newest element block if tracking a structural Table Array.
         * @param[in] rvecPath Reference to vector containing the path
         * @return Reference to the node represented by the path.
         */
        SNode& NavigateToTable(const std::vector<std::string>& rvecPath)
        {
            std::reference_wrapper<SNode> refCurr = m_sRoot;
            for (const auto& rssSection : rvecPath)
            {
                // Safe traversal step into the standard node map structure
                refCurr = std::ref(refCurr.get().mapTable[rssSection]);

                // Context Redirect: If target is a Table Array, dive into the last active table instance
                if (refCurr.get().IsArray() && !refCurr.get().vecArray.empty())
                    refCurr = std::ref(refCurr.get().vecArray.back());
            }
            return refCurr.get();
        }

        /**
        * @brief Safety checks ensuring operations remain within string limits
        * @return Returns whether the current position exceeds the string size.
        */
        bool IsEof() const noexcept
        {
            return m_nPos >= m_svSrc.size();
        }

        /**
         * @brief Non-destructive parsing look-ahead function
         * @return The character at the position or a null-character if EOF.
         */
        char Peek() const noexcept
        {
            return IsEof() ? '\0' : m_svSrc[m_nPos];
        }

        /**
         * @brief Advances processing cursor forward safely
         * @return The character at the position or a null-character if EOF.
         */
        char Consume() noexcept
        {
            return IsEof() ? '\0' : m_svSrc[m_nPos++];
        }

        /**
         * @brief Skips standard spacing gaps, multi-line format updates, and hash strings
         */
        void SkipWhitespaceAndComments() noexcept
        {
            while (!IsEof())
            {
                char c = Peek();
                if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                {
                    Consume();
                }
                else if (c == '#')
                {
                    while (!IsEof() && Peek() != '\n' && Peek() != '\r')
                    {
                        Consume();
                    }
                }
                else
                {
                    break;
                }
            }
        }

        /**
         * @brief Skips single inline whitespaces (spaces and tabs)
         */
        void SkipInlineWhitespace() noexcept
        {
            while (!IsEof() && (Peek() == ' ' || Peek() == '\t'))
                Consume();
        }

        /**
         * @brief Evaluates complex dot-separated key sequences safely
         * @return Returns a vector containing the dotted key parts.
         */
        std::vector<std::string> ParseDottedKey()
        {
            std::vector<std::string> vecKeys; // Vector of strings
            while (!IsEof())
            {
                SkipInlineWhitespace();
                std::string ssKey = ""; // String token
                char c = Peek();
                if (c == '"' || c == '\'')
                    ssKey = ParseStringNode();
                else
                {
                    while (!IsEof())
                    {
                        char bc = Peek(); // Bare Char
                        if (std::isalnum(static_cast<unsigned char>(bc)) || bc == '-' || bc == '_')
                        {
                            ssKey += Consume();
                        }
                        else
                        {
                            break;
                        }
                    }
                    if (ssKey.empty())
                        throw std::runtime_error("Empty or invalid key sequence.");
                }
                vecKeys.push_back(ssKey);
                SkipInlineWhitespace();
                if (Peek() == '.')
                {
                    Consume();
                }
                else
                {
                    break;
                }
            }
            return vecKeys;
        }

        /**
         * @brief Handles text string isolation and processes updated escape arrays
         * @return The string after parsing.
         */
        std::string ParseStringNode()
        {
            char cQuote = Consume(); // Character delimiter
            bool bIsMultiline = false;     // Boolean flag
            if (Peek() == cQuote)
            {
                Consume();
                // Warning of cppcheck for the condition to be always true. This is not the case due to the Consume function
                // execution. Suppress warning.
                // cppcheck-suppress knownConditionTrueFalse
                if (Peek() == cQuote)
                {
                    Consume();
                    bIsMultiline = true;
                }
                else
                {
                    return "";
                }
            }

            std::string ssResult = "";
            while (!IsEof())
            {
                if (bIsMultiline && Peek() == cQuote)
                {
                    if (m_nPos + 2 < m_svSrc.size() && m_svSrc[m_nPos + 1] == cQuote && m_svSrc[m_nPos + 2] == cQuote)
                    {
                        m_nPos += 3;
                        return ssResult;
                    }
                }
                else if (!bIsMultiline && Peek() == cQuote)
                {
                    Consume();
                    return ssResult;
                }

                char c = Consume();
                if (cQuote == '"' && c == '\\')
                {
                    if (IsEof())
                        throw std::runtime_error("Unfinished escape sequence.");
                    char cEsc = Consume(); // Character escape sequence identifier
                    if (cEsc == 'n')
                        ssResult += '\n';
                    else if (cEsc == 't')
                        ssResult += '\t';
                    else if (cEsc == 'r')
                        ssResult += '\r';
                    else if (cEsc == '"')
                        ssResult += '"';
                    else if (cEsc == '\\')
                        ssResult += '\\';
                    else if (cEsc == 'e')
                        ssResult += '\x1b';
                    else if (cEsc == 'x')
                    {
                        if (m_nPos + 1 >= m_svSrc.size())
                            throw std::runtime_error("Truncated hex character.");
                        std::string ssHex{Consume(), Consume()};
                        ssResult += static_cast<char>(std::stoi(ssHex, nullptr, 16));
                    }
                    else if (bIsMultiline && (cEsc == '\n' || cEsc == '\r'))
                    {
                        if (cEsc == '\r' && Peek() == '\n')
                            Consume();
                        SkipInlineWhitespace();
                    }
                }
                else
                {
                    ssResult += c;
                }
            }
            throw std::runtime_error("Unterminated TOML string sequence detected.");
        }

        /**
         * @brief Evaluates header configurations structural tables or table arrays ([[table.path]])
         * @return Returns a vector with table headers.
         */
        //std::vector<std::string> ParseTableHeader()
        //{
        //    Consume(); // Consume standard structural '['

        //    bool bIsTableArray = false;
        //    if (Peek() == '[')
        //    {
        //        Consume();
        //        bIsTableArray = true;
        //    }

        //    std::vector<std::string> vecSections = ParseDottedKey();
        //    SkipInlineWhitespace();

        //    if (bIsTableArray)
        //    {
        //        if (Consume() != ']' || Consume() != ']')
        //            throw std::runtime_error("Malformed table array closure.");
        //    }
        //    else
        //    {
        //        if (Consume() != ']')
        //            throw std::runtime_error("Malformed table closure.");
        //    }

        //    // Generate a unified lookup string for this table path (e.g., "fruit.apple")
        //    std::string strFullNormalizedPath = "";
        //    for (size_t i = 0; i < vecSections.size(); ++i)
        //    {
        //        strFullNormalizedPath += vecSections[i] + (i == vecSections.size() - 1 ? "" : ".");
        //    }

        //    // Safety Constraint: Check if this specific exact table header path was already explicitly declared
        //    if (!bIsTableArray)
        //    {
        //        if (std::find(m_vecExplicitlyDefinedTables.begin(), m_vecExplicitlyDefinedTables.end(), strFullNormalizedPath)
        //            != m_vecExplicitlyDefinedTables.end())
        //        {
        //            throw std::runtime_error("Duplicate table declaration detected: " + strFullNormalizedPath);
        //        }
        //        m_vecExplicitlyDefinedTables.push_back(strFullNormalizedPath);
        //    }

        //    std::reference_wrapper<SNode> refCurrent = m_sRoot;
        //    for (size_t i = 0; i < vecSections.size() - 1; ++i)
        //    {
        //        const auto& ssSection = vecSections[i];

        //        auto it = refCurrent.get().mapTable.find(ssSection);
        //        if (it != refCurrent.get().mapTable.end())
        //        {
        //            if (it->second.IsValue())
        //            {
        //                throw std::runtime_error("Cannot re-define a scalar key value as an intermediate table structure.");
        //            }
        //            // If it exists but is a literal array (not explicitly headered), collision!
        //            if (it->second.IsArray() && !it->second.bIsExplicitlyHeadered)
        //            {
        //                throw std::runtime_error("Type collision: Cannot append table elements to a literal array.");
        //            }
        //        }
        //        else
        //        {
        //            SNode sNewNode;
        //            sNewNode.eType = ENodeType::node_table;
        //            sNewNode.ssKey = ssSection;
        //            refCurrent.get().mapTable[ssSection] = sNewNode;
        //        }
        //        refCurrent = std::ref(refCurrent.get().mapTable[ssSection]);
        //        if (refCurrent.get().IsArray() && !refCurrent.get().vecArray.empty())
        //        {
        //            refCurrent = std::ref(refCurrent.get().vecArray.back());
        //        }
        //    }

        //    std::string ssFinalSection = vecSections.back();
        //    auto itFinal = refCurrent.get().mapTable.find(ssFinalSection);

        //    if (itFinal != refCurrent.get().mapTable.end())
        //    {
        //        if (itFinal->second.IsValue())
        //        {
        //            throw std::runtime_error("Conflict: Table header overrides an existing scalar value.");
        //        }
        //        if (bIsTableArray && itFinal->second.IsTable())
        //        {
        //            throw std::runtime_error("Type collision: Static table cannot be turned into a table array.");
        //        }
        //        if (!bIsTableArray && itFinal->second.IsArray())
        //        {
        //            if (itFinal->second.bIsExplicitlyHeadered)
        //            {
        //                throw std::runtime_error("Type collision: Array of tables cannot be redefined as a standard table.");
        //            }
        //            else
        //            {
        //                throw std::runtime_error("Type collision: Literal array cannot be redefined as a table.");
        //            }
        //        }

        //        // CRITICAL FIX: The invalid exception here was thrown when itFinal->second.IsTable() was true
        //        // but it was implicitly created by a sub-key earlier (e.g., fruit.apple).
        //        // In TOML, a header can safely claim an implicit table, UNLESS it attempts to redefine a value
        //        // that is an explicit scalar or another explicit table type.
        //    }

        //    if (bIsTableArray)
        //    {
        //        if (refCurrent.get().mapTable.find(ssFinalSection) == refCurrent.get().mapTable.end())
        //        {
        //            SNode sNewArrayNode;
        //            sNewArrayNode.eType = ENodeType::node_array;
        //            sNewArrayNode.ssKey = ssFinalSection;
        //            sNewArrayNode.bIsExplicitlyHeadered = true; // Mark as Table Array type
        //            refCurrent.get().mapTable[ssFinalSection] = sNewArrayNode;
        //        }
        //        SNode sNewTableInstance;
        //        sNewTableInstance.eType = ENodeType::node_table;
        //        sNewTableInstance.ssKey = ssFinalSection;
        //        sNewTableInstance.bIsExplicitlyHeadered = true;
        //        refCurrent.get().mapTable[ssFinalSection].vecArray.push_back(sNewTableInstance);
        //    }
        //    else
        //    {
        //        if (refCurrent.get().mapTable.find(ssFinalSection) == refCurrent.get().mapTable.end())
        //        {
        //            SNode sNewNode;
        //            sNewNode.eType = ENodeType::node_table;
        //            sNewNode.ssKey = ssFinalSection;
        //            sNewNode.bIsExplicitlyHeadered = true;
        //            refCurrent.get().mapTable[ssFinalSection] = sNewNode;
        //        }
        //        else
        //        {
        //            refCurrent.get().mapTable[ssFinalSection].bIsExplicitlyHeadered = true;
        //        }
        //    }

        //    return vecSections;
        //}
        // Evaluates header configurations structural tables or table arrays ([[table.path]])
        std::vector<std::string> ParseTableHeader()
        {
            Consume(); // Consume standard structural '['

            bool bIsTableArray = false;
            if (Peek() == '[')
            {
                Consume();
                bIsTableArray = true;
            }

            std::vector<std::string> vecSections = ParseDottedKey();
            SkipInlineWhitespace();

            if (bIsTableArray)
            {
                if (Consume() != ']' || Consume() != ']')
                    throw std::runtime_error("Malformed table array closure.");
            }
            else
            {
                if (Consume() != ']')
                    throw std::runtime_error("Malformed table closure.");
            }

            std::reference_wrapper<SNode> refCurrent = m_sRoot;

            // 1. Traverse down through intermediate namespaces until the second-to-last token component
            for (size_t i = 0; i < vecSections.size() - 1; ++i)
            {
                const auto& ssSection = vecSections[i];

                if (refCurrent.get().bIsInline)
                {
                    throw std::runtime_error("Cannot add subtables to an immutable inline table.");
                }

                auto it = refCurrent.get().mapTable.find(ssSection);
                if (it != refCurrent.get().mapTable.end())
                {
                    if (it->second.IsValue())
                    {
                        throw std::runtime_error("Cannot re-define a scalar key value as an intermediate table structure.");
                    }
                    if (it->second.IsArray() && !it->second.bIsExplicitlyHeadered)
                    {
                        throw std::runtime_error("Type collision: Cannot append table elements to a literal array.");
                    }
                }
                else
                {
                    SNode sNewNode;
                    sNewNode.eType = ENodeType::node_table;
                    sNewNode.ssKey = ssSection;
                    refCurrent.get().mapTable[ssSection] = sNewNode;
                }

                // Advance cursor reference
                refCurrent = std::ref(refCurrent.get().mapTable[ssSection]);

                // Context Redirect: Always dive into the newest array element block if tracking a structural Table Array
                if (refCurrent.get().IsArray() && !refCurrent.get().vecArray.empty())
                {
                    refCurrent = std::ref(refCurrent.get().vecArray.back());
                }
            }

            // 2. Handle the final explicit destination node leaf signature
            std::string ssFinalSection = vecSections.back();
            auto itFinal = refCurrent.get().mapTable.find(ssFinalSection);

            if (itFinal != refCurrent.get().mapTable.end())
            {
                if (itFinal->second.IsValue())
                {
                    throw std::runtime_error("Conflict: Table header overrides an existing scalar value.");
                }
                if (bIsTableArray && itFinal->second.IsTable())
                {
                    throw std::runtime_error("Type collision: Static table cannot be turned into a table array.");
                }
                if (!bIsTableArray && itFinal->second.IsArray())
                {
                    if (itFinal->second.bIsExplicitlyHeadered)
                    {
                        throw std::runtime_error("Type collision: Array of tables cannot be redefined as a standard table.");
                    }
                    else
                    {
                        throw std::runtime_error("Type collision: Literal array cannot be redefined as a table.");
                    }
                }

                // SAFETY CONSTRAINT REMEDIED:
                // Only throw duplicate declaration errors if a standard table header is redefined
                // *explicitly* inside the exact same local scope block wrapper.
                if (!bIsTableArray && itFinal->second.IsTable() && itFinal->second.bIsExplicitlyHeadered)
                {
                    throw std::runtime_error("Duplicate table declaration detected in this scope: " + ssFinalSection);
                }
            }

            // 3. Construct or instantiate the final target node leaf elements safely
            if (bIsTableArray)
            {
                if (refCurrent.get().mapTable.find(ssFinalSection) == refCurrent.get().mapTable.end())
                {
                    SNode sNewArrayNode;
                    sNewArrayNode.eType = ENodeType::node_array;
                    sNewArrayNode.ssKey = ssFinalSection;
                    sNewArrayNode.bIsExplicitlyHeadered = true;
                    refCurrent.get().mapTable[ssFinalSection] = sNewArrayNode;
                }
                SNode sNewTableInstance;
                sNewTableInstance.eType = ENodeType::node_table;
                sNewTableInstance.ssKey = ssFinalSection;
                sNewTableInstance.bIsExplicitlyHeadered = true;
                refCurrent.get().mapTable[ssFinalSection].vecArray.push_back(sNewTableInstance);
            }
            else
            {
                if (refCurrent.get().mapTable.find(ssFinalSection) == refCurrent.get().mapTable.end())
                {
                    SNode sNewNode;
                    sNewNode.eType = ENodeType::node_table;
                    sNewNode.ssKey = ssFinalSection;
                    sNewNode.bIsExplicitlyHeadered = true;
                    refCurrent.get().mapTable[ssFinalSection] = sNewNode;
                }
                else
                {
                    refCurrent.get().mapTable[ssFinalSection].bIsExplicitlyHeadered = true;
                }
            }

            return vecSections;
        }

        /**
         * @brief Extracts key assignment links and creates required target structural tables. Works relative to the passed
         * rsParentTable node, ensuring proper nesting.
         * @param[in] rsParentTable Reference to the parent table.
         */
        void ParseKeyValue(SNode& rsParentTable)
        {
            std::vector<std::string> vecKeys = ParseDottedKey();
            SkipInlineWhitespace();
            if (Consume() != '=')
                throw std::runtime_error("Missing valid variable declaration assignment mapping.");
            SkipInlineWhitespace();

            std::reference_wrapper<SNode> refTarget = std::ref(rsParentTable);
            for (size_t i = 0; i < vecKeys.size() - 1; ++i)
            {
                // ONLY block if we are attempting to add elements down through a table that was completed
                // and locked as an immutable inline structure from an external scope.
                if (refTarget.get().bIsInline)
                {
                    throw std::runtime_error("Cannot traverse or append keys through an immutable inline table.");
                }

                auto it = refTarget.get().mapTable.find(vecKeys[i]);
                if (it != refTarget.get().mapTable.end())
                {
                    if (it->second.IsValue())
                    {
                        throw std::runtime_error("Collision: Implied intermediate segment conflicts with an existing scalar.");
                    }
                    if (it->second.bIsInline)
                    {
                        throw std::runtime_error("Collision: Implied path segment penetrates an immutable inline table.");
                    }
                }
                else
                {
                    SNode sNewNode;
                    sNewNode.eType = ENodeType::node_table;
                    sNewNode.ssKey = vecKeys[i];
                    refTarget.get().mapTable[vecKeys[i]] = sNewNode;
                }
                refTarget = std::ref(refTarget.get().mapTable[vecKeys[i]]);
                if (refTarget.get().IsArray() && !refTarget.get().vecArray.empty())
                {
                    refTarget = std::ref(refTarget.get().vecArray.back());
                }
            }

            std::string ssFinalKey = vecKeys.back();

            // Safety Constraint: Check if the final target key value has already been populated in this specific map context
            if (refTarget.get().mapTable.find(ssFinalKey) != refTarget.get().mapTable.end())
            {
                throw std::runtime_error("Duplicate key configuration definition detected: " + ssFinalKey);
            }

            SNode sParsedValue = ParseValueNode();

            // Safety Constraint: Catch empty data assignment attempts like `key = # comment`
            if (sParsedValue.IsValue() && sParsedValue.ssValueRaw.empty())
            {
                throw std::runtime_error("Assignments cannot be empty or contain only comment elements.");
            }

            sParsedValue.ssKey = ssFinalKey;
            refTarget.get().mapTable[ssFinalKey] = sParsedValue;
        }

        /**
         * @brief Resolves scalar elements, multi-line inline dictionaries, and arrays
         * @return The parsed node.
         */
        SNode ParseValueNode()
        {
            SkipWhitespaceAndComments();
            char c = Peek();
            SNode sValue;

            if (c == '"' || c == '\'')
            {
                sValue.eType = ENodeType::node_value;
                sValue.ssValueRaw = ParseStringNode();
            }
            else if (c == '{')
            {
                sValue.eType = ENodeType::node_table;
                Consume(); // Consume starting '{'
                while (!IsEof())
                {
                    SkipWhitespaceAndComments();
                    if (Peek() == '}')
                    {
                        Consume();
                        break;
                    }
                    ParseKeyValue(sValue); // Can append safely during the parsing lifecycle phase
                    SkipWhitespaceAndComments();
                    if (Peek() == ',')
                    {
                        Consume();
                    }
                    else if (Peek() == '}')
                    {
                        Consume();
                        break;
                    }
                }
                // CRITICAL FIX: Lock the inline structure to prevent outside modifications
                // ONLY after it has completely finished parsing and its braces are closed!
                sValue.bIsInline = true;
            }
            else if (c == '[')
            {
                sValue.eType = ENodeType::node_array;
                sValue.bIsExplicitlyHeadered = false; // Literal array token
                Consume();
                while (!IsEof())
                {
                    SkipWhitespaceAndComments();
                    if (Peek() == ']')
                    {
                        Consume();
                        break;
                    }
                    sValue.vecArray.push_back(ParseValueNode());
                    SkipWhitespaceAndComments();
                    if (Peek() == ',')
                    {
                        Consume();
                    }
                    else if (Peek() == ']')
                    {
                        Consume();
                        break;
                    }
                }
            }
            else
            {
                sValue.eType = ENodeType::node_value;
                std::string strRaw = "";
                while (!IsEof() && Peek() != '\n' && Peek() != '\r' && Peek() != ',' && Peek() != '}' && Peek() != ']'
                       && Peek() != '#')
                {
                    char rc = Consume();
                    if (rc != ' ' && rc != '\t')
                        strRaw += rc;
                }
                sValue.ssValueRaw = strRaw;
            }
            return sValue;
        }
    };
} // namespace sdv::toml::simple_parser

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif
