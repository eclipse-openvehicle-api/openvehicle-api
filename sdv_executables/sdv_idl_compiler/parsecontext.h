#ifndef PARSE_CONTEXT_H
#define PARSE_CONTEXT_H

#include "includes.h"
#include "source.h"
#include "codepos.h"
#include <filesystem>
#include <string>

/**
 * @brief Source code context
 */
class CContext
{
public:

    /**
     * @brief Constructor using a path
     * @param[in] rpath Reference to the path of the file.
     * @param[in] eLocation The location of the file to parse.
     */
    CContext(const std::filesystem::path& rpath, sdv::idl::IEntityContext::ELocation eLocation = sdv::idl::IEntityContext::ELocation::source);

    /**
     * @brief Constructor using code
     * @param[in] szCode Zero terminated string containing the code to set.
     */
    CContext(const char* szCode);

    /**
     * @brief Copy constructor
     */
    CContext(const CContext&) = delete;

    /**
     * @brief Move constructor
     * @param[in, out] rContext Reference to the source code context.
     */
    CContext(CContext&& rContext) noexcept;

    /**
     * @brief Destructor
     */
    ~CContext();

    /**
     * @brief Assignment operator
     * @return Returns a reference to this object.
     */
    CContext& operator=(const CContext&) = delete;

    /**
     * @brief Move operator
     * @param[in, out] rContext Reference to the source code structure.
     * @return Returns a reference to this object.
     */
    CContext& operator=(CContext&& rContext) noexcept;

    /**
     * @brief Compare for equal context.
     * @param[in] rContext Reference to the source code structure.
     * @return Returns the comparison result.
     */
    bool operator==(const CContext& rContext) const;

    /**
     * @brief Compare for inequal context.
     * @param[in] rContext Reference to the source code structure.
     * @return Returns the comparison result.
     */
    bool operator!=(const CContext& rContext) const;

    /**
     * @{
     * @brief Get source access.
     * @return Reference to the source code.
     */
    CSource& Source() { return m_source; }
    const CSource& Source() const { return m_source; }
    /**
     * @}
     */

    /**
     * @{
     * @brief Get code access.
     * @return Reference to the code position.
     */
    CCodePos& Code() { return m_code; }
    const CCodePos& Code() const { return m_code; }
    /**
     * @}
     */

    /**
     * @brief Get the code location.
     * @return Returns the code location.
    */
    sdv::idl::IEntityContext::ELocation GetLocation() const { return m_eLocation; }

private:
    CSource                             m_source;                                                   ///< Source file holding code
    CCodePos                            m_code;                                                     ///< Code navigation
    sdv::idl::IEntityContext::ELocation m_eLocation = sdv::idl::IEntityContext::ELocation::source;  ///< Location of the code.
};

/**
 * @brief Context shared pointer.
 */
using CContextPtr = std::shared_ptr<CContext>;

#endif // !defined(PARSE_CONTEXT_H)