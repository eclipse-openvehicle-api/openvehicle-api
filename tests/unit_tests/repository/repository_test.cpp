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

#include <gtest/gtest.h>
#include <fstream>
#include "mock.h"
#include "generated/IComponent.h"
#include <interfaces/repository.h>
#include "../../../global/exec_dir_helper.h"

const uint32_t LoopCount = 100; //< amount of loops used for concurrency tests. more loops means more thorough deadlock check at the cost of increased runtime

// used to supply external objects for testing pruposes. Implements IObjectControl to make sure this is NOT called for externally supplied objects
class TestExternalObject : public sdv::IInterfaceAccess, public sdv::IObjectControl
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    virtual void Initialize([[maybe_unused]] const sdv::u8string& ssObjectConfig) override
    {
        FAIL() << "Error: Initialize should not be called by Repo Service!";
        //m_eObjectState = sdv::EObjectState::initialization_failure;
    }

    virtual sdv::EObjectState GetObjectState() const override
    {
        return m_eObjectState;
    }

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    virtual void SetOperationMode(sdv::EOperationMode eMode) override
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_eObjectState == sdv::EObjectState::running || m_eObjectState == sdv::EObjectState::initialized)
                m_eObjectState = sdv::EObjectState::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_eObjectState == sdv::EObjectState::configuring || m_eObjectState == sdv::EObjectState::initialized)
                m_eObjectState = sdv::EObjectState::running;
            break;
        default:
            break;
        }
    }

    virtual sdv::u8string GetObjectConfig() const override
    {
        return {};
    }

    virtual void Shutdown() override
    {
        m_eObjectState = sdv::EObjectState::shutdown_in_progress;
        FAIL() << "Error: Shutdown should not be called by Repo Service!";
        //m_eObjectState = sdv::EObjectState::destruction_pending;

    }

    sdv::EObjectState m_eObjectState = sdv::EObjectState::initialization_pending;
};

TEST(RepositoryTest, LoadNonexistentModule)
{
    CRepository repository;     // Must be created first
    CModuleControl modulectrl;
    CHelper helper(modulectrl, repository);
    EXPECT_FALSE(modulectrl.Load((GetExecDirectory() / "TestFooBar.sdv").generic_u8string()));
}

TEST(RepositoryTest, CreateNonexistentClass)
{
    CRepository repository;     // Must be created first
    CModuleControl modulectrl;
    CHelper helper(modulectrl, repository);
    ASSERT_TRUE(modulectrl.Load((GetExecDirectory() / "UnitTest_Repository_test_module.sdv").generic_u8string()));
    EXPECT_FALSE(repository.CreateObject2("TestFooBar", nullptr, nullptr));
    repository.DestroyObject2("TestFooBar");
}

TEST(RepositoryTest, GetNonexistentObject)
{
    CRepository repository;     // Must be created first
    CModuleControl modulectrl;
    CHelper helper(modulectrl, repository);
    ASSERT_TRUE(modulectrl.Load((GetExecDirectory() / "UnitTest_Repository_test_module.sdv").generic_u8string()));
    bool bRes = repository.CreateObject2("Example_Object", nullptr, nullptr);
    EXPECT_TRUE(bRes);
    EXPECT_EQ(nullptr, repository.GetObject("TestFooBar"));
    repository.DestroyObject2("Example_Object");
}

TEST(RepositoryTest, InstantiateAndGet)
{
    CRepository repository;     // Must be created first
    CModuleControl modulectrl;
    CHelper helper(modulectrl, repository);
    ASSERT_TRUE(modulectrl.Load((GetExecDirectory() / "UnitTest_Repository_test_module.sdv").generic_u8string()));
    bool bRes = repository.CreateObject2("Example_Object", nullptr, nullptr);
    EXPECT_TRUE(bRes);
    EXPECT_NE(nullptr, repository.GetObject("Example_Object"));
    repository.DestroyObject2("Example_Object");
    EXPECT_EQ(nullptr, repository.GetObject("Example_Object"));
}

TEST(RepositoryTest, InstantiateInitFail)
{
    CRepository repository;     // Must be created first
    CModuleControl modulectrl;
    CHelper helper(modulectrl, repository);
    ASSERT_TRUE(modulectrl.Load((GetExecDirectory() / "UnitTest_Repository_test_module.sdv").generic_u8string()));
    bool bRes = repository.CreateObject2("TestObject_IObjectControlFail", nullptr, nullptr);
    EXPECT_FALSE(bRes);
    EXPECT_EQ(nullptr, repository.GetObject("TestObject_IObjectControlFail"));
    repository.DestroyObject2("TestObject_IObjectControlFail");
}
