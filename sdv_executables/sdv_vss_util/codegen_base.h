#ifndef CODEGEN_BASE_H
#define CODEGEN_BASE_H

#include <map>
#include <string>
#include <sstream>
#include <cmath>

/**
 * @brief Code generator base class.
*/
class CCodeGeneratorBase
{
protected:
    /**
    * @brief Keyword map for keyword replacement in a string.
    */
    typedef std::map<std::string, std::string> CKeywordMap;

    /**
    * @brief Replace keywords in a string.
    * @param[in] rssStr Reference to the string containing the keywords.
    * @param[in] rmapKeywords Map with keywords to replace.
    * @param[in] cMarker Character to identify the keyword with (placed before and after the keyword; e.g. %keyword%).
    * @return Returns the string with replacements.
    */
    static std::string ReplaceKeywords(const std::string& rssStr, const CKeywordMap& rmapKeywords, char cMarker = '%');

    /**
     * @brief Generator specific string generator. The std::to_string doesn't handle floating points very well.
     * @tparam T Type of the variable to stringetize.
     * @param[in] rtVar Reference to the variable.
     * @return The generated string object.
     */
    template <typename T>
    static std::string to_string(const T& rtVar)
    {
        std::ostringstream sstream;
        sstream << rtVar;
        return sstream.str();
    }

    /**
     * @brief Overloading for double.
     * @param[in] rdVar Reference to the double variable.
     * @return The generated string object.
     */
    static std::string to_string(const double& rdVar)
    {
        std::ostringstream sstream;
        sstream << rdVar;
        if (std::round(rdVar) == rdVar) sstream << ".0";
        return sstream.str();
    }

    /**
     * @brief Overloading for float.
     * @param[in] rfVar Reference to the float variable.
     * @return The generated string object.
     */
    static std::string to_string(const float& rfVar)
    {
        std::ostringstream sstream;
        sstream << rfVar;
        if (std::round(rfVar) == rfVar) sstream << ".0";
        sstream << "f";
        return sstream.str();
    }
};

#endif // !defined CODEGEN_BASE_H