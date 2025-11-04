#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include <array>
#include <iostream>

/**
 * @brief Print a table of a vector of rows.
 * @tparam nSize The array size.
 * @details the first line is the header.
 * @param[in] rvecRows Reference to the vector of rows.
 * @param[in] rstream The output stream to use for printing.
 * @param[in] bNoHeader When set, skip the first row (the header row).
 */
template <size_t nSize>
void PrintTable(const std::vector<std::array<std::string, nSize>>& rvecRows, std::ostream& rstream, bool bNoHeader = false)
{
    // Calculate the maximum length of the column by going through each cell.
    bool bSkipFirst = bNoHeader;
    size_t rgnLength[nSize] = {};
    for (const auto& rrgssRow : rvecRows)
    {
        if (bSkipFirst)
        {
            bSkipFirst = false;
            continue;
        }

        for (size_t nIndex = 0; nIndex < nSize; nIndex++)
        {
            if (rrgssRow[nIndex].size() > rgnLength[nIndex])
                rgnLength[nIndex] = rrgssRow[nIndex].size();
        }
    }

    // Print a cell
    auto fnPrintCell = [&](size_t nIndex, const std::string& rssText)
    {
        // Print the text
        rstream << rssText;

        // Check index
        if (nIndex == nSize - 1)    // Last column; nl needed
            rstream << std::endl;
        else // Separator needed
            rstream << std::string(rgnLength[nIndex] - rssText.size() + 2, ' ');
    };

    // Print each line
    bSkipFirst = bNoHeader;
    bool bInitialLine = !bNoHeader;
    for (const auto& rrgssRow : rvecRows)
    {
        if (bSkipFirst)
        {
            bSkipFirst = false;
            continue;
        }

        // Start with a space if a header is present
        if (!bNoHeader)
            rstream << " ";

        // Print the text
        for (size_t nIndex = 0; nIndex < nSize; nIndex++)
            fnPrintCell(nIndex, rrgssRow[nIndex]);

        // Need a vertical separator?
        if (bInitialLine)
        {
            size_t nSeparatorLength = 1;
            for (size_t nIndex = 0; nIndex < nSize; nIndex++)
                nSeparatorLength += rgnLength[nIndex] + 2;
            nSeparatorLength--;
            rstream << std::string(nSeparatorLength, '-') << std::endl;
            bInitialLine = false;
        }
    }
}

#endif // !defined TABLE_H