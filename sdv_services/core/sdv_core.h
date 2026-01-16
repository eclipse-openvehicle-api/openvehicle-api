#ifndef CORE_H
#define CORE_H

#include <support/component_impl.h>
#include "app_control.h"
#include "module_control.h"
#include "memory.h"
#include "repository.h"
#include "logger_control.h"
#include "logger.h"
#include "app_config.h"

/**
* @brief SDV core instance class containing containing the instances for the core services.
*/
class CSDVCore : public sdv::IInterfaceAccess
{
public:
    /**
    * @brief Constructor
    */
    CSDVCore();

    /**
    * @brief Destructor
    */
    ~CSDVCore();

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_CHAIN_MEMBER(m_appctrl)
        SDV_INTERFACE_CHAIN_MEMBER(m_modulectrl)
        SDV_INTERFACE_CHAIN_MEMBER(m_memmgr)
        SDV_INTERFACE_CHAIN_MEMBER(m_repository)
        SDV_INTERFACE_CHAIN_MEMBER(m_loggerctrl)
    END_SDV_INTERFACE_MAP()

    /**
    * @brief The one and only instance.
    * @return Reference to this class.
    */
    static CSDVCore& GetInstance();

    /**
     * @brief Return the application control.
     * @return Reference to the application control.
     */
    CAppControl& GetAppControl();

    /**
     * @brief Return the module control.
     * @return Reference to the module control.
    */
    CModuleControl& GetModuleControl();

    /**
     * @brief Return the memory manager.
     * @return Reference to the memory manager.
     */
    CMemoryManager& GetMemoryManager();

    /**
     * @brief Return the repository.
     * @return Reference to the repository.
    */
    CRepository& GetRepository();

    /**
     * @brief Return the logger control.
     * @return Reference to the logger control.
    */
    CLoggerControl& GetLoggerControl();

    /**
     * @brief Return the default logger.
     * @return Reference to the default logger.
     */
    CLogger& GetDefaultLogger();

    /**
     * @brief Return the application config class.
     * @return Reference to the application config class.
     */
    CAppConfig& GetAppConfig();

private:
    CMemoryManager	    m_memmgr;           ///< Memory manager - note: needs to be first in the list of members!
    CAppControl         m_appctrl;          ///< Application control
    CRepository         m_repository;       ///< Repository - note: repository should be present before module control!
    CModuleControl      m_modulectrl;       ///< Module control
    CLogger             m_defaultlogger;    ///< Default logger - note: the logger must be present before the logger control!
    CLoggerControl      m_loggerctrl;       ///< Logger control
    CAppConfig          m_appconfig;        ///< Application configuration class
};

/**
 * @brief Exported function for core access.
 * @return Pointer to the interface of the core library.
 */
extern "C" SDV_SYMBOL_PUBLIC sdv::IInterfaceAccess* SDVCore();

/**
 * @brief Return the application control.
 * @return Reference to the application control.
 */
CAppControl& GetAppControl();

/**
 * @brief Return the module control.
 * @return Reference to the module control.
 */
CModuleControl& GetModuleControl();

/**
 * @brief Return the memory manager.
 * @return Reference to the memory manager.
 */
CMemoryManager& GetMemoryManager();

/**
 * @brief Return the repository.
 * @return Reference to the repository.
 */
CRepository& GetRepository();

/**
 * @brief Return the logger control.
 * @return Reference to the logger control.
 */
CLoggerControl& GetLoggerControl();

/**
 * @brief Return the default logger.
 * @attention Use the logger control to access the logger.
 * @return Reference to the default logger.
 */
CLogger& GetDefaultLogger();

/**
 * @brief Return the application config class.
 * @return Reference to the application config class.
 */
CAppConfig& GetAppConfig();

/**
 * @brief Get the location of the core_services.sdv.
 * @return Path to the directory containing the loaded core directory.
 */
std::filesystem::path GetCoreDirectory();

#endif // !defined CORE_H