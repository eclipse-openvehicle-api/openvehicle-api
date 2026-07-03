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

#ifndef PERMISSION_CONTROL_H
#define PERMISSION_CONTROL_H

#include <interfaces/permission.h>
#include <list>
#include <support/component_impl.h>

#include "../../global/unique_id.h"

// Forward declaration
class CPermissionControl;

/**
 * @brief Use a list to create a stack, which allows elements to be deleted.
 */
using TPermissionStack = std::list<sdv::core::EAccessPermission>;

/**
 * @brief Permission class for the requested access. This class manages the lifetime of the permissions.
 */
class CAccessPermission
{
private:
    /// Permission control is allowed to create an access permission object.
    friend CPermissionControl;

    /**
     * @brief Constructor
     * @param[in] rPermissionControl Reference to permission control.
     * @param[in] ePermission The requested access permission.
     */
    CAccessPermission(CPermissionControl& rPermissionControl, sdv::core::EAccessPermission ePermission);

public:
    /**
     * @brief Destructor
     */
    ~CAccessPermission();

    /**
     * @brief Copy constructor
     * @param[in] rPermission Reference to the permission object to copy from.
     */
    CAccessPermission(const CAccessPermission& rPermission);

    /**
     * @brief Move constructor.
     * @param[in] rPermission Reference to the permission object to move from.
     */
    CAccessPermission(CAccessPermission&& rPermission);

    /**
     * @brief Copy assignment operator.
     * @param[in] rPermission Reference to the permission object to move from.
     * @return Reference to this access permission object.
     */
    CAccessPermission& operator=(const CAccessPermission& rPermission);

    /**
     * @brief Move assignment operator.
     * @param[in] rPermission Reference to the permission object to move from.
     * @return Reference to this access permission object.
     */
    CAccessPermission& operator=(CAccessPermission&& rPermission);

    /**
     * @brief Get the permission level assigned to this object.
     * @remarks This might not correspond to the current permission level available for this thread.
     * @return The permission level.
     */
    sdv::core::EAccessPermission Permission() const;

private:
    CPermissionControl& m_rPermissionControl;   ///< Reference to the permission control.
    sdv::core::EAccessPermission m_ePermission; ///< Requested permission.
};

/**
 * @brief Permission control provides a simple access mechanism to allow the management of access to system functions.
 */
class CPermissionControl : public sdv::IInterfaceAccess, public sdv::core::IPermissionControl
{
public:
    /// Access permission class can access the permission control.
    friend CAccessPermission;

    /**
     * @brief Default constructor
     */
    CPermissionControl() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::core::IPermissionControl)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Restrict the access permission for the current thread. Overload of
     * sdv::core.:IPermissionControl::RestrictAccessPermission.
     * @remarks The access restriction will be assigned to the current thread and combined with previous and future
     * permissions. The lowest assigned permission will determine the actual access permission for the current thread.
     * @remarks The access restriction will stay in effect until it is released by the function ReleaseAccessPermission.
     * @remarks A newly created thread has fully restricted access. This cannot be changed using this function. Use an access
     * restriction transfer from one thread to this thread to set a higher level of access permission.
     * @param[in] ePermission The permission to restrict to.
     * @return The permission ID for this restriction or 0 when the access permission could not be set. Use the
     * ReleaseAccessPermission to release the restriction again.
     */
    virtual sdv::core::TPermissionID RestrictAccessPermission(/*in*/ sdv::core::EAccessPermission ePermission) override;

    /**
     * @brief Release a previously set access restriction for the current thread. Overload of
     * sdv::core::IPermissionControl::ReleaseAccessPermission.
     * @param[in] tPermissionID The ID of the access restriction previously set for the current thread.
     * @return Returns whether the restriction could be released successfully.
     */
    virtual bool ReleaseAccessPermission(/*in*/ sdv::core::TPermissionID tPermissionID) override;

    /**
     * @brief Prepare to transfer the access restriction from the current thread.Overload of
     * sdv::core.:IPermissionControl::TransferCurrentPermission.
     * @return The ID of the transfer object containing the current access permissions or 0 when the transfer preparation has
     * failed.
     */
    virtual sdv::core::TPermissionTransferID TransferCurrentPermission() override;

    /**
     * @brief Set the access permission using a transfer object from one thread to another. Overload of
     * sdv::core.:IPermissionControl::SetAccessPermission.
     * @remarks An new thread has fully restricted access per default. Use this function to set the required access level. If a
     * thread has already initialized with the proper access level, this function will set identical or lower access permissions
     * for the current thread.
     * @param[in] tTransferID The IS of the prepared access permission transfer.
     * @return The permission ID for this restriction or 0 when the access permission could not be transferred. Use the
     * ReleaseAccessPermission to release the restriction again.
     */
    virtual sdv::core::TPermissionID SetAccessPermission(/*in*/ sdv::core::TPermissionTransferID tTransferID) override;

    /**
     * @brief Get the access permission level for the current thread. This will be the lowest restriction set for the current
     * thread. Overload of sdv::core.:IPermissionControl::GetCurrentPermission.
     * @return The current access permission level.
     */
    virtual sdv::core::EAccessPermission GetCurrentPermission() const override;

    /**
     * @brief Create a permission object that assigns the access permissions to the call thread. The permissions stay in place
     * until a last copy of the object is destroyed.
     * @param[in] ePermission The new permission to set for the thread. If the permission has a value smaller than
     * sdv::core::EAccessPermission::restricted_access, the object will be created with restricted access permission.
     * @return The permission object. The permission will stay in place during the lifetime of the object.
     */
    CAccessPermission CreatePermissionObject(sdv::core::EAccessPermission ePermission);

private:
    /**
     * @brief Increment the counter for a specific permission.
     * @param[in] ePermission The permission the count should be incremented for.
     */
    void IncrementPermissionCount(sdv::core::EAccessPermission ePermission);

    /**
     * @brief Increment the counter for a specific permission.
     * @param[in] ePermission The permission the count should be incremented for.
     */
    void DecrementPermissionCount(sdv::core::EAccessPermission ePermission);

    CUniqueID<uint64_t> m_idgen; ///< Unique ID generator.

#if defined __GNUC__ && defined _WIN32
    // https://dev.azure.com/SW4ZF/AZP-431_DivDI_Vehicle_API/_workitems/edit/610009
    // In MINGW GCC implementation of thread_local, there is an issue with complex classes being used after the destruction. This
    // is caused by an architectural bug (https://github.com/msys2/MINGW-packages/issues/2519) causing the emutls (GCC Emulated TLS)
    // managing the memory allocation to be executed before the __cxa_thread_exit, the thread cleanup callback of Windows.
    // The solution is to use trivial data only (pointer, reference, integers, etc.). A workaround is implemented to use a reference
    // of the permission list and map instead of an instance to the list and map.

    /**
     * @brief Create a permission list object.
     * @return Reference to the created list object.
    */
    static std::list<std::pair<sdv::core::EAccessPermission, size_t>>& CreatePermissionList()
    {
        static std::mutex mtxPermissionLists;
        static std::list<std::list<std::pair<sdv::core::EAccessPermission, size_t>>> lstPermissionLists;
        std::unique_lock<std::mutex> lock(mtxPermissionLists);
        lstPermissionLists.resize(lstPermissionLists.size() + 1);
        return lstPermissionLists.back();
    }

    /// Thread specific permission tracker list (permission, lowest first and the count for each permission second).
    static thread_local std::list<std::pair<sdv::core::EAccessPermission, size_t>>& m_lstPermissionTracker;

    /**
     * @brief Create a permission map object.
     * @return Reference to the created map object.
     */
    static std::map<sdv::core::TPermissionID, CAccessPermission>& CreatePermissionMap()
    {
        static std::mutex mtxPermissionMaps;
        static std::list<std::map<sdv::core::TPermissionID, CAccessPermission>> lstPermissionMaps;
        std::unique_lock<std::mutex> lock(mtxPermissionMaps);
        lstPermissionMaps.resize(lstPermissionMaps.size() + 1);
        return lstPermissionMaps.back();
    }

    /// Thread specific permission access map.
    static thread_local std::map<sdv::core::TPermissionID, CAccessPermission>& m_mapPermissions;
#else
    /// Thread specific permission tracker list (permission, lowest first and the count for each permission second).
    static thread_local std::list<std::pair<sdv::core::EAccessPermission, size_t>> m_lstPermissionTracker;

    /// Thread specific permission access map.
    static thread_local std::map<sdv::core::TPermissionID, CAccessPermission> m_mapPermissions;
#endif

    /// Permission transfer object map (allow shifting permission from one thread to another).
    std::map<sdv::core::TPermissionTransferID, sdv::core::EAccessPermission> m_mapPermissionTransfer;

    std::mutex m_mtxPermissionTransfer; ///< Protecting the permission transfer map.
};

/**
 * @brief Return the permission control.
 * @return Reference to the permission control.
 */
CPermissionControl& GetPermissionControl();

/**
 * @brief Permission control service
 */
class CPermissionControlService : public sdv::CSdvObject
{
public:
    CPermissionControlService() = default;

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_CHAIN_MEMBER(GetPermissionControl())
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::system_object)
    DECLARE_OBJECT_CLASS_NAME("PermissionControlService")
    DECLARE_OBJECT_SINGLETON()
};

DEFINE_SDV_OBJECT(CPermissionControlService)

#endif // !defined PERMISSION_CONTROL_H