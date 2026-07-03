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

#include <optional>
#include "../../include/gtest_custom.h"
#include "../../../sdv_services/core/permission_control.h"

TEST(PermissionControl, DefaultRestriction)
{
    CPermissionControl control;
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);
}

TEST(PermissionControl, SetRestrictionInternal)
{
    CPermissionControl control;
    std::vector<CAccessPermission> vecPermissions;
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Full access
    vecPermissions.emplace_back(control.CreatePermissionObject(sdv::core::EAccessPermission::full_access));
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::full_access);

    // Reduce access to local access
    vecPermissions.emplace_back(control.CreatePermissionObject(sdv::core::EAccessPermission::local_access));
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::local_access);

    // Reduce access to remote access
    vecPermissions.emplace_back(control.CreatePermissionObject(sdv::core::EAccessPermission::remote_access));
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::remote_access);

    // Reduce access to restricted access
    vecPermissions.emplace_back(control.CreatePermissionObject(sdv::core::EAccessPermission::restricted_access));
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Try increase access to full access - this should not have any change
    vecPermissions.emplace_back(control.CreatePermissionObject(sdv::core::EAccessPermission::full_access));
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Remove the local access index #1 - this should not have any change
    EXPECT_EQ(vecPermissions[1].Permission(), sdv::core::EAccessPermission::local_access);
    vecPermissions.erase(vecPermissions.begin() + 1);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Remove first full access index #0 - this should not have any change
    EXPECT_EQ(vecPermissions[0].Permission(), sdv::core::EAccessPermission::full_access);
    vecPermissions.erase(vecPermissions.begin());
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Remove remote access new index #0 -  this should not have any change
    EXPECT_EQ(vecPermissions[0].Permission(), sdv::core::EAccessPermission::remote_access);
    vecPermissions.erase(vecPermissions.begin());
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Remove restricted access new index #0 -  access will change to full access
    EXPECT_EQ(vecPermissions[0].Permission(), sdv::core::EAccessPermission::restricted_access);
    vecPermissions.erase(vecPermissions.begin());
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::full_access);

    // Remove the remaining access - this would change to (default) restricted access again
    vecPermissions.clear();
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);
}

TEST(PermissionControl, SetRestrictionExternal)
{
    CPermissionControl control;
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Try to set external permission to full access; this will not work
    EXPECT_EQ(control.RestrictAccessPermission(sdv::core::EAccessPermission::full_access), 0u);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Try to set external permission to local access; this will not work
    EXPECT_EQ(control.RestrictAccessPermission(sdv::core::EAccessPermission::local_access), 0u);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Try to set external permission to remote access; this will not work
    EXPECT_EQ(control.RestrictAccessPermission(sdv::core::EAccessPermission::remote_access), 0u);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Try to set external permission to restricted access; this will not work
    EXPECT_EQ(control.RestrictAccessPermission(sdv::core::EAccessPermission::restricted_access), 0u);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);
}

TEST(PermissionControl, InitializeRestrictionInternal_SetRestrictionExternal)
{
    CPermissionControl control;
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Set restriction to full access
    std::optional<CAccessPermission> optPermission = control.CreatePermissionObject(sdv::core::EAccessPermission::full_access);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::full_access);

    // Set external permission to local access
    sdv::core::TPermissionID permidLocalAccess = control.RestrictAccessPermission(sdv::core::EAccessPermission::local_access);
    EXPECT_NE(permidLocalAccess, 0u);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::local_access);

    // Set external permission to remote access
    sdv::core::TPermissionID permidRemoteAccess = control.RestrictAccessPermission(sdv::core::EAccessPermission::remote_access);
    EXPECT_NE(permidRemoteAccess, 0u);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::remote_access);

    // Set external permission to restricted access
    sdv::core::TPermissionID permidRestrictedAccess =
        control.RestrictAccessPermission(sdv::core::EAccessPermission::restricted_access);
    EXPECT_NE(permidRestrictedAccess, 0u);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Set external permission to full access - this will not change the access
    sdv::core::TPermissionID permidFullAccess = control.RestrictAccessPermission(sdv::core::EAccessPermission::full_access);
    EXPECT_NE(permidFullAccess, 0u);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Release not existing access ID - should not work
    EXPECT_FALSE(control.ReleaseAccessPermission(0));
    EXPECT_FALSE(control.ReleaseAccessPermission(1));

    // Release local, restricted and full access - this will change access to remote access
    EXPECT_TRUE(control.ReleaseAccessPermission(permidLocalAccess));
    EXPECT_TRUE(control.ReleaseAccessPermission(permidRestrictedAccess));
    EXPECT_TRUE(control.ReleaseAccessPermission(permidFullAccess));
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::remote_access);

    // Release local again - should not work
    EXPECT_FALSE(control.ReleaseAccessPermission(permidLocalAccess));

    // Terminate internal full access - access should stay with remote access
    optPermission.reset();
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::remote_access);

    // Release remote access - the access permission will change to restricted access again
    EXPECT_TRUE(control.ReleaseAccessPermission(permidRemoteAccess));
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);
}

TEST(PermissionControl, DefaultThreadPermission)
{
    CPermissionControl control;
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Set restriction to full access
    CAccessPermission permission = control.CreatePermissionObject(sdv::core::EAccessPermission::full_access);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::full_access);

    // Start new thread - access should be restricted
    std::thread thread([&]()
        {
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);
        });
    thread.join();
}

TEST(PermissionControl, InternalSetThreadPermission)
{
    CPermissionControl control;
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Set restriction to local access
    CAccessPermission permission = control.CreatePermissionObject(sdv::core::EAccessPermission::local_access);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::local_access);

    // Start new thread - access should be restricted
    std::thread thread([&]()
        {
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

            // Set full access permission
            CAccessPermission permissionFullAccess = control.CreatePermissionObject(sdv::core::EAccessPermission::full_access);
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::full_access);

            // Set local access permission
            CAccessPermission permissionLocalAccess = control.CreatePermissionObject(sdv::core::EAccessPermission::local_access);
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::local_access);

            // Set remote access permission
            CAccessPermission permissionRemoteAccess = control.CreatePermissionObject(sdv::core::EAccessPermission::remote_access);
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::remote_access);

            // Set restricted access permission
            CAccessPermission permissionRestrictedAccess =
                control.CreatePermissionObject(sdv::core::EAccessPermission::restricted_access);
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);
        });
    thread.join();

    // Access permissions on main thread should still be local
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::local_access);
}

TEST(PermissionControl, TransferThreadPermission)
{
    CPermissionControl control;
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

    // Set restriction to full access
    CAccessPermission permissionFullAccess = control.CreatePermissionObject(sdv::core::EAccessPermission::full_access);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::full_access);
    sdv::core::TPermissionID transferFullAccess = control.TransferCurrentPermission();
    EXPECT_NE(transferFullAccess, 0u);

    // Set restriction to local access
    CAccessPermission permissionLocalAccess = control.CreatePermissionObject(sdv::core::EAccessPermission::local_access);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::local_access);
    sdv::core::TPermissionID transferLocalAccess = control.TransferCurrentPermission();
    EXPECT_NE(transferLocalAccess, 0u);

    // Set restriction to remote access
    CAccessPermission permissionRemoteAccess = control.CreatePermissionObject(sdv::core::EAccessPermission::remote_access);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::remote_access);
    sdv::core::TPermissionID transferRemoteAccess = control.TransferCurrentPermission();
    EXPECT_NE(transferRemoteAccess, 0u);

    // Set restriction to restricted access
    CAccessPermission permissionRestrictedAccess = control.CreatePermissionObject(sdv::core::EAccessPermission::restricted_access);
    EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);
    sdv::core::TPermissionID transferRestrictedAccess = control.TransferCurrentPermission();
    EXPECT_NE(transferRestrictedAccess, 0u);

    // Start new thread - access should be restricted
    std::thread thread([&]()
        {
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

            // Tets bogus transfer IDs
            EXPECT_FALSE(control.SetAccessPermission(0));
            EXPECT_FALSE(control.SetAccessPermission(1));

            // Set full access permission from transfer ID
            sdv::core::TPermissionID permidFullAccess = control.SetAccessPermission(transferFullAccess);
            EXPECT_NE(permidFullAccess, 0u);
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::full_access);

            // Set local access permission from transfer ID
            sdv::core::TPermissionID permidLocalAccess = control.SetAccessPermission(transferLocalAccess);
            EXPECT_NE(permidLocalAccess, 0u);
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::local_access);

            // Set remote access permission from transfer ID
            sdv::core::TPermissionID permidRemoteAccess = control.SetAccessPermission(transferRemoteAccess);
            EXPECT_NE(permidRemoteAccess, 0u);
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::remote_access);

            // Set restricted access permission from transfer ID
            sdv::core::TPermissionID permidRestrictedAccess = control.SetAccessPermission(transferRestrictedAccess);
            EXPECT_NE(permidRestrictedAccess, 0u);
            EXPECT_EQ(control.GetCurrentPermission(), sdv::core::EAccessPermission::restricted_access);

            // At the end of the thread, the IDs will be cleaned up automatically.
        });
    thread.join();
}