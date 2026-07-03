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

#ifndef LOCAL_SERVICE_ACCESS_H
#define LOCAL_SERVICE_ACCESS_H

#include "../interfaces/core.h"
#include "../interfaces/log.h"
#include "../interfaces/repository.h"
#include "../interfaces/com.h"
#include "../interfaces/app.h"
#include "../interfaces/param.h"
#include "../interfaces/permission.h"
#include "interface_ptr.h"
#include <thread>
#include <utility>
#include <functional>
#include <sstream>
#include <utility>
#include <string>
#include <stdexcept>
#ifdef __GNUC__
// Needed for getpid()
#include <unistd.h>
#endif

/**
 * @brief Software Defined Vehicle framework.
 */
namespace sdv
{
    // Forward declaration
    interface IInterfaceAccess;

    /**
     * @brief Core features.
     */
    namespace core
    {
#ifndef SDV_CUSTOM_GETOBJECT
        /**
         * @brief Get an object from the repository service.
         * @attention This function only work when called from modules loaded via the repository service.
         * @param[in] rssObjectName Reference to the name of the requested object (as defined via DECLARE_OBJECT_CLASS_NAME).
         * @return Returns the smart interface pointer of the object instance if found or nullptr otherwise.
         */
        inline TInterfaceAccessPtr GetObject(const std::string& rssObjectName)
        {
            if (!GetCore())
                return nullptr;
            IObjectAccess* pRepository = GetCore<IObjectAccess>();
            if (!pRepository)
                return nullptr;
            return pRepository->GetObject(rssObjectName);
        }

        /**
         * @brief Get an object from the repository service.
         * @attention This function can only be used on controlled objects. Local object are not returned using this function.
         * @param[in] tObjectID Object ID of the object instantiation.
         * @return Returns the smart interface pointer of the object instance if found or nullptr otherwise.
         */
        inline TInterfaceAccessPtr GetObject(TObjectID tObjectID)
        {
            if (!GetCore())
                return nullptr;
            IObjectAccess* pRepository = GetCore<IObjectAccess>();
            if (!pRepository)
                return nullptr;
            return pRepository->GetObjectByID(tObjectID);
        }
#else
        /**
         * @brief Get an object from the repository service.
         * @attention This function only work when called from modules loaded via the repository service.
         * @param[in] rssObjectName Reference to the name of the requested object (as defined via DECLARE_OBJECT_CLASS_NAME).
         * @return Returns the smart interface pointer of the object instance if found or nullptr otherwise.
         */
        TInterfaceAccessPtr GetObject(const std::string& rssObjectName);

        /**
         * @brief Get an object from the repository service.
         * @attention This function can only be used on controlled objects. Local object are not returned using this function.
         * @param[in] tObjectID Object ID of the object instantiation.
         * @return Returns the smart interface pointer of the object instance if found or nullptr otherwise.
         */
        TInterfaceAccessPtr GetObject(TObjectID tObjectID);
#endif

        /**
         * @brief Get an object from the repository service.
         * @tparam TInterface The type of interface to return.
         * @attention This function can only be used on controlled objects. Local object are not returned using this function.
         * @param[in] rssObjectName Reference to the name of the requested object (as defined via DECLARE_OBJECT_CLASS_NAME).
         * @return Returns the interface pointer of the object instance if found or nullptr otherwise.
         */
        template <typename TInterface>
        inline TInterface* GetObject(const std::string& rssObjectName)
        {
            return GetObject(rssObjectName).GetInterface<TInterface>();
        }

        /**
         * @brief Get an object from the repository service.
         * @tparam TInterface The type of interface to return.
         * @attention This function can only be used on controlled objects. Local object are not returned using this function.
         * @param[in] tObjectID Object ID of the object instantiation.
         * @return Returns the interface pointer of the object instance if found or nullptr otherwise.
         */
        template <typename TInterface>
        inline TInterface* GetObject(TObjectID tObjectID)
        {
            return GetObject(tObjectID).GetInterface<TInterface>();
        }

        /**
         * @brief Log function enables logging for SDV.
         * @param[in] eSeverity Severity level of the log message which will be logged, e.g. Info, Warning, Error etc.
         * @param[in] rssSrcFile Name of the file from which the message is logged.
         * @param[in] uiSrcLine Line of the file from which the message is logged.
         * @param[in] rssMessage Reference to the log message to be logged.
         */
        inline void Log(ELogSeverity eSeverity, const u8string& rssSrcFile, uint32_t uiSrcLine, const u8string& rssMessage)
        {
            ILogger* pLogger = GetCore() ? GetCore<ILogger>() : nullptr;
#ifdef _WIN32
            if (pLogger)
                pLogger->Log(eSeverity, rssSrcFile, uiSrcLine, _getpid(), "", rssMessage);
#elif defined __unix__
            if (pLogger)
                pLogger->Log(eSeverity, rssSrcFile, uiSrcLine, getpid(), "", rssMessage);
#else
    #error The OS is currently not supported!
#endif
        }

        /**
         * @brief Log a message with line and position.
         * @param severity The severity of the message. One of the entries from ELogServerity.
         */
#define SDV_LOG(severity, ...) sdv::core::internal::CSDVLogImpl(severity, __FILE__, __LINE__, __VA_ARGS__)

        /**
         * @brief Log a trace message with line and position.
         */
#define SDV_LOG_TRACE(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::trace, __FILE__, __LINE__, __VA_ARGS__)

        /**
         * @brief Log a debug message with line and position.
         */
#define SDV_LOG_DEBUG(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::debug, __FILE__, __LINE__, __VA_ARGS__)

        /**
         * @brief Log an information message with line and position.
         */
#define SDV_LOG_INFO(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::info, __FILE__, __LINE__, __VA_ARGS__)

        /**
         * @brief Log a warning message with line and position.
         */
#define SDV_LOG_WARNING(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::warning, __FILE__, __LINE__, __VA_ARGS__)

        /**
         * @brief Log an error message with line and position.
         */
#define SDV_LOG_ERROR(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::error, __FILE__, __LINE__, __VA_ARGS__)

        /**
         * @brief Log a fatal message with line and position.
         */
#define SDV_LOG_FATAL(...) sdv::core::internal::CSDVLogImpl(sdv::core::ELogSeverity::fatal, __FILE__, __LINE__, __VA_ARGS__)

        namespace internal
        {
            /**
             * @brief Template to log messages with line and position.
             * @param[in] eSeverity Severity level of the message.
             * @param[in] szSrcFile Name of the file from which the message is logged. Specified by C++ standard.
             * @param[in] uiSrcLine Line of the file from which the message is logged. Specified by C++ standard.
             * @param[in] ...args identifier specified by C++ standard that uses the ellipsis notation in the parameters.
             */
            template <typename... Args>
            inline void CSDVLogImpl(ELogSeverity eSeverity, const char* szSrcFile, uint32_t uiSrcLine, Args&&... args)
            {
                std::ostringstream stream;
                (stream << ... << std::forward<Args>(args));

                Log(eSeverity, szSrcFile ? szSrcFile : "", uiSrcLine, stream.str().c_str());
            }
        } // namespace internal

        /**
         * @brief Create a utility
         * @param[in] rssUtilityName Reference to the utility name.
         * @param[in] rssUtilityConfig Optional reference to the utility configuration.
         * @return Smart pointer to the utility or NULL when the utility could not be found.
         */
        inline TObjectPtr CreateUtility(const std::string& rssUtilityName, const std::string& rssUtilityConfig = std::string())
        {
            TInterfaceAccessPtr ptrRepository = GetObject("RepositoryService");
            IRepositoryUtilityCreate* pUtilityCreate = ptrRepository.GetInterface<IRepositoryUtilityCreate>();
            if (!pUtilityCreate) return nullptr;
            return pUtilityCreate->CreateUtility(rssUtilityName, rssUtilityConfig);
        }

        /**
         * @brief Get a parameter from the object smart pointer.
         * @param[in] pParameters Pointer to the parameter map interface of an object.
         * @param[in] rssParamName Name of the parameter (incl. groups preceding the name and separated with a dot).
         * @param[in] bNoExcept When set, do not trigger an std::runtime exception when the object or the parameter could not be
         * found.
         * @return Returns the value of the parameter or an empty any_t when the object or the parameter could not be found and
         * bNoExcept was set.
         */
        inline any_t GetParameter(const IParameters* pParameters, const std::string& rssParamName, bool bNoExcept = true)
        {
            if (!pParameters)
            {
                if (bNoExcept) return {};
                throw std::runtime_error("The object doesn't expose IParameters interface.");
            }
            any_t any = pParameters->GetParam(rssParamName);
            if (any.empty())
            {
                if (bNoExcept) return {};
                throw std::runtime_error("The parameter could not be found.");
            }
            return any;
        }

        /**
         * @brief Get a parameter from the parameter interface.
         * @param[in] rptrObject Reference to object smart pointer to the the parameter of.
         * @param[in] rssParamName Name of the parameter (incl. groups preceding the name and separated with a dot).
         * @param[in] bNoExcept When set, do not trigger an std::runtime exception when the object or the parameter could not be
         * found.
         * @return Returns the value of the parameter or an empty any_t when the object or the parameter could not be found and
         * bNoExcept was set.
         */
        inline any_t GetParameter(const TInterfaceAccessPtr& rptrObject, const std::string& rssParamName, bool bNoExcept = true)
        {
            const IParameters* pParameters = rptrObject.GetInterface<IParameters>();
            if (!pParameters)
            {
                if (bNoExcept) return {};
                throw std::runtime_error("The object doesn't expose IParameters interface.");
            }
            return GetParameter(pParameters, rssParamName, bNoExcept);
        }

        /**
         * @brief Get a parameter from the object with the supplied name.
         * @param[in] rssObjectName Reference to the string containing the name of the object or service.
         * @param[in] rssParamName Name of the parameter (incl. groups preceding the name and separated with a dot).
         * @param[in] bNoExcept When set, do not trigger an std::runtime exception when the object or the parameter could not be
         * found.
         * @return Returns the value of the parameter or an empty any_t when the object or the parameter could not be found and
         * bNoExcept was set.
         */
        inline any_t GetParameter(const std::string& rssObjectName, const std::string& rssParamName, bool bNoExcept = true)
        {
            TInterfaceAccessPtr ptrObject = GetObject(rssObjectName);
            if (!ptrObject)
            {
                if (bNoExcept) return {};
                throw std::runtime_error("An object with the name '" + rssObjectName + "' could not be found.");
            }
            return GetParameter(ptrObject, rssParamName, bNoExcept);
        }

        /**
         * @brief Get a parameter from the object with the supplied name. Resolve enums and bitmask objects in the returned
         * parameter string.
         * @param[in] rssObjectName Reference to the string containing the name of the object or service.
         * @param[in] rssParamName Name of the parameter (incl. groups preceding the name and separated with a dot).
         * @param[in] bNoExcept When set, do not trigger an std::runtime exception when the object or the parameter could not be
         * found.
         * @return Returns the value of the parameter as a text string or an empty string when the object or the parameter could
         * not be found and bNoExcept was set.
         */
        inline std::string GetParameterExpand(const std::string& rssObjectName, const std::string& rssParamName, bool bNoExcept = true)
        {
            TInterfaceAccessPtr ptrObject = GetObject(rssObjectName);
            if (!ptrObject)
            {
                if (bNoExcept) return {};
                throw std::runtime_error("An object with the name '" + rssObjectName + "' could not be found.");
            }

            const IParameters* pParameters = ptrObject.GetInterface<IParameters>();
            if (!pParameters)
            {
                if (bNoExcept) return {};
                throw std::runtime_error("The object doesn't expose IParameters interface.");
            }
            any_t any = pParameters->GetParam(rssParamName);
            if (any.empty())
            {
                if (bNoExcept) return {};
                throw std::runtime_error("The parameter could not be found.");
            }

            SParamInfo sParamInfo = pParameters->GetParamInfo(rssParamName);
            if (sParamInfo.get_switch() == EParamType::enum_param)
            {
                for (const SLabelInfo::SLabel& rsLabel : sParamInfo.uExtInfo.sEnumInfo.seqLabels)
                {
                    if (rsLabel.anyValue == any)
                        return rsLabel.ssLabel;
                }
            }
            if (sParamInfo.get_switch() == EParamType::bitmask_param)
            {
                uint64_t uiValue = static_cast<uint64_t>(any);
                std::stringstream sstreamValue;
                for (const SLabelInfo::SLabel& rsLabel : sParamInfo.uExtInfo.sBitmaskInfo.seqLabels)
                {
                    uint64_t uiValueLabel = static_cast<uint64_t>(rsLabel.anyValue);
                    if (uiValue & uiValueLabel)
                    {
                        if (sstreamValue.rdbuf()->in_avail()) // Has characters
                            sstreamValue << "|";
                        sstreamValue << rsLabel.ssLabel;
                        uiValue = uiValue & ~uiValueLabel;
                    }
                }
                if (uiValue)    // Not all bits have labels
                {
                    if (sstreamValue.rdbuf()->in_avail()) // Has characters
                        sstreamValue << "|";
                    sstreamValue << uiValue;
                }
                return sstreamValue.str();
            }
            return any;
        }

        /// Internal namespace
        namespace internal
        {
            /**
             * @brief Trim whitespace at the begin and end of the text.
             * @param[in] rssText Reference to the text to trim whitespace for.
             * @return The trimmed text.
             */
            inline std::string TrimWhitespace(const std::string& rssText)
            {
                size_t nFirst = rssText.find_first_not_of(" \t\r\n");
                if (nFirst == std::string::npos) return "";
                size_t nLast = rssText.find_last_not_of(" \t\r\n");
                return rssText.substr(nFirst, (nLast - nFirst + 1));
            }

            /**
             * @brief Helper function to safely check if a string contains a valid, unescaped $(...) sequence.
             * @param[in] rssText The text to check.
             * @return Returns whether the text represents a valid variable.
             */
            inline bool ContainsValidVariable(const std::string& rssText)
            {
                size_t nLength = rssText.length();
                size_t nIndex  = 0;

                while (nIndex < nLength)
                {
                    if (rssText[nIndex] == '\\')
                    {
                        // Skip the escape and the escaped character
                        nIndex += 2;
                    }
                    else if (rssText[nIndex] == '$')
                    {
                        // Check if it forms the start of a token sequence '$( '
                        if (nIndex + 1 < nLength && rssText[nIndex + 1] == '(')
                        {
                            size_t nClose = rssText.find(')', nIndex + 2);
                            if (nClose != std::string::npos)
                            {
                                // Found a valid structural candidate
                                return true;
                            }
                        }
                        nIndex++;
                    }
                    else
                    {
                        nIndex++;
                    }
                }
                return false;
            }
        } // namespace internal
        
        /**
         * @brief Resolves embedded object parameter variables formatted as $(object:param) within a text string.
         * @param[in] rssText The source text containing potential variables and backslash escapes.
         * @param[in] bRecursive If true, continues resolving until no more unescaped variables remain.
         * @param[in] bNoExcept If true, returns an empty string for errors; if false, throws std::runtime_error.
         * @return The fully resolved text string.
         */
        inline std::string ResolveText(const std::string& rssText, bool bRecursive = true, bool bNoExcept = true)
        {
            std::string ssCurrentText = rssText;
            bool bMayHaveMoreVariables = true;

            while (bMayHaveMoreVariables)
            {
                bMayHaveMoreVariables = false;
                std::string ssResult = "";
                ssResult.reserve(ssCurrentText.length()); // Pre-allocate memory for performance

                size_t nLength = ssCurrentText.length();
                size_t nIndex  = 0;

                while (nIndex < nLength)
                {
                    char cCurrent = ssCurrentText[nIndex];

                    // Handle backslash escape sequences
                    if (cCurrent == '\\')
                    {
                        // Check if the next character is a '$'
                        if (nIndex + 1 < nLength && ssCurrentText[nIndex + 1] == '$')
                        {
                            ssResult.push_back('$'); // Strip the escape character, keep the literal '$'
                            nIndex += 2;
                        }
                        else
                        {
                            ssResult.push_back('\\'); // Preserve other backslashes for paths
                            nIndex++;
                        }
                    }
                    // Identify potential variables
                    else if (cCurrent == '$')
                    {
                        // extension 2: Only throw an error or treat as a variable if it is
                        // explicitly part of a dynamic sequence tracking towards a '$(...)' match.
                        if (nIndex + 1 >= nLength || ssCurrentText[nIndex + 1] != '(')
                        {
                            // It is a loose literal '$' (like currency). Keep it and do not throw.
                            ssResult.push_back('$');
                            nIndex++;
                            continue;
                        }

                        size_t nVarStart = nIndex + 2; // Position right after "$("
                        size_t nVarEnd   = ssCurrentText.find(')', nVarStart);

                        // Validate that a matching closing parenthesis exists
                        if (nVarEnd == std::string::npos)
                        {
                            if (!bNoExcept)
                                throw std::runtime_error(
                                    "Format error: Missing closing parenthesis ')' for variable starting at index "
                                    + std::to_string(nIndex));
                            return "";
                        }

                        // Extract the inner payload block
                        std::string sVariableContent = ssCurrentText.substr(nVarStart, nVarEnd - nVarStart);
                        size_t nColonPos = sVariableContent.find(':');

                        // Validate that the delimiter ':' separates object and parameter names
                        if (nColonPos == std::string::npos)
                        {
                            if (!bNoExcept)
                                throw std::runtime_error("Format error: Variable content '" + sVariableContent
                                                         + "' lacks an 'object:param' colon separator");
                            return "";
                        }

                        // Extract and clean whitespace before/after tokens, preserving inner spaces
                        std::string sObjName   = internal::TrimWhitespace(sVariableContent.substr(0, nColonPos));
                        std::string sParamName = internal::TrimWhitespace(sVariableContent.substr(nColonPos + 1));

                        // Request the parameter.
                        std::string ssValue = GetParameterExpand(sObjName, sParamName, bNoExcept);

                        // Add the parameter to the string
                        ssResult.append(ssValue);

                        nIndex = nVarEnd + 1; // Move parsing index past the ')'
                    }
                    else
                    {
                        ssResult.push_back(cCurrent);
                        nIndex++;
                    }
                }

                ssCurrentText = ssResult;

                // Exit immediately if recursive resolution was disabled
                if (!bRecursive) break;

                // extension 2: Intelligently analyze if a recursive processing loop is required
                if (internal::ContainsValidVariable(ssCurrentText))
                    bMayHaveMoreVariables = true;
            }

            return ssCurrentText;
        }

        /**
         * @brief Get the current access permission.
         * @return The current access permission.
         */
        inline EAccessPermission GetCurrentAccessPermission()
        {
            IPermissionControl* pPermissionControl = GetCore<IPermissionControl>();
            if (!pPermissionControl)
                return EAccessPermission::not_set;
            return pPermissionControl->GetCurrentPermission();
        }

        /**
         * @brief Get the current transmission from this thread and prepare a transfer to another thread.
         * @return Transfer ID identifying the permission of this thread.
         */
        inline TPermissionTransferID TransferCurrentPermission()
        {
            IPermissionControl* pPermissionControl = GetCore<IPermissionControl>();
            if (!pPermissionControl) return 0u;
            return pPermissionControl->TransferCurrentPermission();
        }

        /**
         * @brief Class managing the lifetime of an access permission.
         */
        class CAccessPermission
        {
            // Friend functions
            friend CAccessPermission RestrictAccessPermission(EAccessPermission);
            friend CAccessPermission SetAccessPermission(TPermissionTransferID);

        public:
            /**
             * @brief Default constructor
             */
            CAccessPermission() = default;

        private:
            /**
             * @brief Constructor
             * @param[in] tPermissionID The access permission to manage.
             */
            CAccessPermission(TPermissionID tPermissionID) : m_tPermissionID(tPermissionID)
            {}

        public:
            /**
             * @brief Copy constructor is deleted.
             * @param[in] rPermission Reference to the permission to copy from.
             */
            CAccessPermission(const CAccessPermission& rPermission) = delete;

            /**
             * @brief Move constructor.
             * @param[in] rPermission Reference to the permission to move from.
             */
            CAccessPermission(CAccessPermission&& rPermission) : m_tPermissionID(rPermission.m_tPermissionID)
            {
                rPermission.m_tPermissionID = 0u;
            }

            /**
             * @brief Destructor
             */
            ~CAccessPermission()
            {
                Release();
            }

            /**
             * @brief Copy assignment operator is deleted.
             * @param[in] rPermission Reference to the permission to copy from.
             * @return Reference to this class.
             */
            CAccessPermission& operator=(const CAccessPermission& rPermission) = delete;

            /**
             * @brief Move assignment operator.
             * @param[in] rPermission Reference to the permission to move from.
             * @return Reference to this class.
             */
            CAccessPermission& operator=(CAccessPermission&& rPermission)
            {
                m_tPermissionID = rPermission.m_tPermissionID;
                rPermission.m_tPermissionID = 0u;
                return *this;
            }

            /**
             * @brief Boolean operator.
             * @return Returns whether a valid permission ID is assigned to this class.
             */
            operator bool() const
            {
                return m_tPermissionID ? true : false;
            }

            /**
             * @brief Does the access permission class contain a valid permission ID.
             * @return Returns whether a valid permission ID is assigned to this class.
             */
            bool IsValid() const
            {
                return m_tPermissionID ? true : false;
            }

            /**
             * @brief Release the permission.
             */
            void Release()
            {
                if (!m_tPermissionID) return;
                IPermissionControl* pPermissionControl = GetCore<IPermissionControl>();
                if (!pPermissionControl) return;
                pPermissionControl->ReleaseAccessPermission(m_tPermissionID);
                m_tPermissionID = 0u;
            }

        private:
            TPermissionID   m_tPermissionID = 0u;       ///< Permission ID to be managed by this class.
        };

        /**
         * @brief Restrict the current access permission.
         * @param[in] ePermission The permission the access should be restricted to.
         * @return The access permission connected to this restriction. The lifetime of the access permission is managed by the
         * returned class.
         */
        inline CAccessPermission RestrictAccessPermission(EAccessPermission ePermission)
        {
            IPermissionControl* pPermissionControl = GetCore<IPermissionControl>();
            if (!pPermissionControl) return {};
            return pPermissionControl->RestrictAccessPermission(ePermission);
        }

        /**
         * @brief Set the access permission from a transfer ID that defined the access permission of a different thread.
         * @param[in] tTransferID The ID of the transferred access permission.
         * @return The access permission for the current thread. The lifetime of the access permission is managed by the returned
         * class.
         */
        inline CAccessPermission SetAccessPermission(TPermissionTransferID tTransferID)
        {
            IPermissionControl* pPermissionControl = GetCore<IPermissionControl>();
            if (!pPermissionControl) return {};
            return pPermissionControl->SetAccessPermission(tTransferID);
        }

        /**
         * @brief Secure thread implementation based on std::thread transferring the permissions from the creation thread to the
         * execution thread.
         */
        class secure_thread : public std::thread
        {
        public:
            /**
             * @brief Default constructor
             */
            secure_thread() = default;

            /**
             * @brief Copy constructor is deleted.
             * @param[in] rthread Reference to the thread object.
             */
            secure_thread(const secure_thread& rthread) = delete;

            /**
             * @brief Move constructor.
             * @param[in] rthread Reference to the thread object.
             */
            secure_thread(secure_thread&& rthread) : std::thread(static_cast<std::thread&&>(rthread))
            {}

            /**
             * @brief Assignment constructor for thread execution.
             * @tparam F The function to execute.
             * @tparam Args The argument types of the function to execute.
             * @param[in] f Reference to the function.
             * @param[in] args Reference to zero or more arguments.
             */
            template <class F, class... Args>
            explicit secure_thread(F&& f, Args&&... args)
            {
                static_cast<std::thread&>(*this) = std::thread(
                    [](TPermissionTransferID tTransferID, auto&& function, auto&&... arguments)
                    {
                        CAccessPermission permission = SetAccessPermission(tTransferID);
                        std::invoke(std::forward<decltype(function)>(function), std::forward<decltype(arguments)>(arguments)...);
                    }, TransferCurrentPermission(), std::forward<F>(f), std::forward<Args>(args)...);
            }

            /**
             * @brief Move assignment operator.
             * @param[in] rthread Reference to the thread object.
             * @return Reference to this object.
            */
            secure_thread& operator=(secure_thread&& rthread)
            {
                static_cast<std::thread&>(*this) = static_cast<std::thread&&>(rthread);
                return *this;
            }
        };
    } // namespace core
}// namespace sdv

/**
 * @{
 * @brief Comparison operators
 * @param[in] e1 First permission
 * @param[in] e2 Second permission
 * @return Result of the comparison
 */
inline bool operator<(sdv::core::EAccessPermission e1, sdv::core::EAccessPermission e2) { return static_cast<int32_t>(e1) < static_cast<int32_t>(e2); }
inline bool operator<=(sdv::core::EAccessPermission e1, sdv::core::EAccessPermission e2) { return static_cast<int32_t>(e1) <= static_cast<int32_t>(e2); }
inline bool operator>(sdv::core::EAccessPermission e1, sdv::core::EAccessPermission e2) { return static_cast<int32_t>(e1) > static_cast<int32_t>(e2); }
inline bool operator>=(sdv::core::EAccessPermission e1, sdv::core::EAccessPermission e2) { return static_cast<int32_t>(e1) >= static_cast<int32_t>(e2); }
/**
 * @}
 */

namespace sdv
{
    namespace app
    {
        /**
         * @brief Get application attribute.
         * @param[in] rssAttribute Name of the attribute.
         * @return The attribute value or an empty any-value if the attribute wasn't found or didn't have a value.
         */
        inline any_t GetAppSettingsAttribute(const std::string& rssAttribute)
        {
            const IParameters* pParameters = core::GetObject<IParameters>("AppSettingsService");
            if (!pParameters) return {};
            return pParameters->GetParam(rssAttribute);
        }

        /**
         * @brief Returns true if the current application is configured for silent console reporting.
         * @return Returns whether the silent console reporting has been enabled.
         */
        inline bool ConsoleIsSilent()
        {
            return GetAppSettingsAttribute("Console.Reporting") == "Silent";
        }

        /**
         * @brief Returns true if the current application is configured for verbose console reporting.
         * @return Returns whether the verbose console reporting has been enabled.
         */
        inline bool ConsoleIsVerbose()
        {
            return GetAppSettingsAttribute("Console.Reporting") == "Verbose";
        }

        /**
         * @brief Get the instance ID of the application.
         * @return The instance ID.
         */
        inline uint32_t GetAppInstanceID()
        {
            return GetAppSettingsAttribute("Application.Instance");
        }
    } // namespace app

    namespace com
    {
        /**
         * @brief Create a repository connection to a local server.
         * @param[in] nRetries Number of retries to connect (optional, default 30, minimum 3).
         * @return Returns a smart pointer to the repository proxy. Disconnection takes place when IObjectDestroy::DestroyObject is
         * called.
         */
        inline TObjectPtr ConnectToLocalServerRepository(size_t nRetries = 30)
        {
            // This function works with main, external and maintenance applications.
            const app::IAppContext* pAppContext = core::GetCore<app::IAppContext>();
            if (!pAppContext) return {};
            switch (pAppContext->GetContextType())
            {
            case app::EAppContext::main:
            case app::EAppContext::external:
            case app::EAppContext::maintenance:
                break;
            default:
                return {};
            }

            const sdv::app::IAppConnections* pConnections = core::GetObject<sdv::app::IAppConnections>("AppSettingsService");
            if (!pConnections) return {};
            std::string ssConnectionConfig = pConnections->GetConnectionConfig("Default");
            if (ssConnectionConfig.empty()) return {};

            sdv::core::IRepositoryControl* pRepository = core::GetObject<sdv::core::IRepositoryControl>("RepositoryService");
            if (!pRepository) return {};
            auto tConnectSvcID = pRepository->CreateObject("ClientConnectService", "ClientDefault", ssConnectionConfig);
            if (!tConnectSvcID) return {};

            sdv::TInterfaceAccessPtr ptrConnectSvc = core::GetObject(tConnectSvcID);
            sdv::com::IClientConnect* pClientConnect = ptrConnectSvc.GetInterface<sdv::com::IClientConnect>();
            if (!pClientConnect)
            {
                pRepository->DestroyObject("ClientDefault");
                return {};
            }

            try
            {
                // Try to connect (30 times with 1 second in between).
                size_t nCnt = 0;
                sdv::TObjectPtr ptrRemoteRepo;
                while (!ptrRemoteRepo && nCnt < std::max(nRetries, static_cast<size_t>(3u)))
                {
                    nCnt++;
                    if (pClientConnect->IsConnected() || pClientConnect->Connect())
                    {
                        ptrRemoteRepo = pClientConnect->GetRemoteRepository();
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }

                // Return the result
                return ptrRemoteRepo;
            }
            catch (const XAccessDenied& /*rExcept*/)
            {
                if (!app::ConsoleIsSilent())
                    std::cout << "Access denied trying to connect to a local repository with server instance ID#" <<
                        app::GetAppInstanceID() << "." << std::endl;
                return {};
            }
            catch (const XNotFound& /*rExcept*/)
            {
                if (!app::ConsoleIsSilent())
                    std::cout << "Local repository with server instance ID#" <<
                        app::GetAppInstanceID() << " not found." << std::endl;
                return {};
            }
            catch (const XInvalidState& rExcept)
            {
                if (!app::ConsoleIsSilent())
                    std::cout << "The local repository with server instance ID#" <<
                        app::GetAppInstanceID() << " is in an invalid state: " << rExcept.what() <<
                        std::endl;
                return {};
            }
            catch (const XTimeout& /*rExcept*/)
            {
                if (!app::ConsoleIsSilent())
                    std::cout << "Timeout occurred trying to connect to a local repository with server instance ID#" <<
                        app::GetAppInstanceID() << "." << std::endl;
                return {};
            }
        }
    } // namespace com
} // namespace sdv

#endif // !defined LOCAL_SERVICE_ACCESS_H