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
#include <iostream>
#include <mutex>
#include <functional>
#include <thread>
#include <support/component_impl.h>
#include "generated/IComponent.h"
#include "../../../global/tracefifo/trace_fifo.cpp"

class CTestLockService : public sdv::CSdvObject, public ITestLock
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(ITestLock)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("TestLockService")

    void Lock() override
    {
        m_mutex.lock();
    }

    void Unlock() override
    {
        m_mutex.unlock();
    }

private:
    std::recursive_mutex m_mutex;
};

DEFINE_SDV_OBJECT(CTestLockService)

/**
 * @brief Example component testing IObjectControl
 */
class CTestObjectControl : public sdv::CSdvObject
{
public:
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("Example_Object")

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        return true;
    }

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {}
};

DEFINE_SDV_OBJECT(CTestObjectControl)

/**
 * @brief Example component testing IObjectControl
 */
class CTestObjectControlFail : public sdv::CSdvObject
{
public:

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_IObjectControlFail")

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override
    {
        return false;
    }

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override
    {}
};

DEFINE_SDV_OBJECT(CTestObjectControlFail)

