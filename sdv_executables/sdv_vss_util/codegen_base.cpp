#include "codegen_base.h"
#include <sstream>

std::string CCodeGeneratorBase::ReplaceKeywords(const std::string& rssStr, const CKeywordMap& rmapKeywords, char cMarker /*= '%'*/)
{
    std::stringstream sstream;
    size_t nPos = 0;
    while (nPos < rssStr.size())
    {
        // Find the initial separator
        size_t nSeparator = rssStr.find(cMarker, nPos);
        sstream << rssStr.substr(nPos, nSeparator == std::string::npos ? nSeparator : nSeparator - nPos);
        nPos = nSeparator;
        if (nSeparator == std::string::npos) continue;
        nPos++;

        // Find the next separator.
        nSeparator = rssStr.find(cMarker, nPos);
        if (nSeparator == std::string::npos)
        {
            // Internal error: missing second separator during code building.
            continue;
        }

        // Find the keyword in the keyword map (between the separator and the position).
        CKeywordMap::const_iterator itKeyword = rmapKeywords.find(rssStr.substr(nPos, nSeparator - nPos));
        if (itKeyword == rmapKeywords.end())
        {
            // Internal error: invalid keyword during building code.
            nPos = nSeparator + 1;
            continue;
        }
        sstream << itKeyword->second;
        nPos = nSeparator + 1;
    }
    return sstream.str();
}

