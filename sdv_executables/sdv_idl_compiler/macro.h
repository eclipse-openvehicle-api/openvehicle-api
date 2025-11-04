#ifndef MACRO_H
#define MACRO_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include "token.h"

// Forward declarations
class CIdlCompilerEnvironment;

/**
 * @brief A set of macro names used previously in macro expansion.
 */
 using CUsedMacroSet = std::set<std::string>;

/**
 * @brief Macro processing class.
 */
class CMacro
{
public:
    /**
     * @brief Constructor
     * @param[in] szName The name of the macro.
     * @param[in] pvecParams Pointer to the list of zero or more param definitions or NULL when the macro doesn't have param
     * definitions.
     * @param[in] szValue Zero terminated string to the value or NULL when the macro doesn't have a value.
     */
    CMacro(const char* szName, const std::vector<std::string>* pvecParams, const char* szValue);

    /**
     * @brief Copy constructor
     * @param[in] rMacro Reference to the macro to copy from.
     */
    CMacro(const CMacro& rMacro);

    /**
     * @brief Move constructor
     * @param[in] rMacro Reference to the macro to copy from.
     */
    CMacro(CMacro&& rMacro) noexcept;

    /**
     * @brief Assignment operator
     * @param[in] rMacro Reference to the macro to copy from.
     * @return Reference to this macro.
     */
    CMacro& operator=(const CMacro& rMacro);

    /**
     * @brief Move operator
     * @param[in] rMacro Reference to the macro to copy from.
     * @return Reference to this macro.
     */
    CMacro& operator=(CMacro&& rMacro) noexcept;

    /**
     * @brief Equality operator
     * @param[in] rMacro Reference to the macro to copy from.
     * @return Returns true if the macros are identical; otherwise returns false.
     */
    bool operator==(const CMacro& rMacro);

    /**
     * @brief Inequality operator
     * @param[in] rMacro Reference to the macro to copy from.
     * @return Returns true if the macros are not identical; otherwise returns false.
     */
    bool operator!=(const CMacro& rMacro);

    /**
     * @brief Get the name of the macro.
     * @return Reference to the string containing the macro name.
     */
    const std::string& GetName() const;

    /**
     * @brief Returns whether the macro expects zero or more parameters (uses parenthesis for its parameters).
     * @attention A macro could return 'true' even when it expects zero parameters. In that case, it uses at least parenthesis.
     * If no parenthesis is defined for this macro, it would returns false.
     * @return Returns 'true' when the macro expects parameters; false when not.
     */
    bool ExpectParameters() const;

    /**
     * @brief Expand the macro with the provided parameters. Returns the value string of the macro with the filled in parameters.
     * @param[in] renv Reference to the current environment needed to expand parameters.
     * @param[in] rtoken Token of the parameters in the code. Used for throwing errors.
     * @param[in] rvecParams Vector with the parameters.
     * @param[in, out] rsetUsedMacros Reference to the set of previously expanded macros, preventing circular expansion. This set
     * will be extended if macros
     * @return Returns the macro string with the resolved parameters.
     */
    std::string Expand(const CIdlCompilerEnvironment& renv, const CToken& rtoken, const std::vector<std::string>& rvecParams,
        CUsedMacroSet& rsetUsedMacros) const;

private:
    std::string                 m_ssName;           ///< Name of the macro (provided through the define-direction).
    bool                        m_bExpectParams;    ///< When set, the macro expects parenthesis and zero or more parameters.
    std::vector<std::string>    m_vecParamDefs;     ///< List of parameters.
    std::string                 m_ssValue;          ///< The macro value to replace the name with.
};

/**
 * @brief Map containing the associations of macro name and value.
 */
using CMacroMap = std::map<std::string, CMacro>;

#endif // !defined MACRO_H