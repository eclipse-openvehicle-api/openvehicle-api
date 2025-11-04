#include <gtest/gtest.h>
#include <iostream>
#include <mutex>
#include <functional>
#include <thread>
#include <support/component_impl.h>
#include "generated/IComponent.h"
#include "../../../global/tracefifo/trace_fifo.cpp"

class CTestLockService
    : public sdv::CSdvObject, public ITestLock
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(ITestLock)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
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
    class CTestObjectControl
    : public sdv::CSdvObject
    , public sdv::IObjectControl
{
public:

    ~CTestObjectControl()
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::destruction_pending);
    }

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("Example_Object")

    /**
     * @brief Initialization method. On success, a subsequent call to GetStatus returns EObjectStatus::running
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize([[maybe_unused]] const sdv::u8string& ssObjectConfig)
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::initialization_pending);

        m_eObjectStatus = sdv::EObjectStatus::initialized;
    }

    /**
     * @brief Gets the current status of the object
     * @return EObjectStatus The current status of the object
     */
    virtual sdv::EObjectStatus GetStatus() const
    {
        return m_eObjectStatus;
    }

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
     * @brief Shutdown method called before the object is destroyed.
     * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the object!
     * After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
     * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still call it during the shutdown sequence!
     * Any subsequent call to GetStatus should return EObjectStatus::destruction_pending
     */
    virtual void Shutdown()
    {
        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    }

private:
    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
};

DEFINE_SDV_OBJECT(CTestObjectControl)

/**
 * @brief Example component testing IObjectControl
 */
class CTestObjectControlFail : public sdv::CSdvObject, public sdv::IObjectControl
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("TestObject_IObjectControlFail")

    /**
     * @brief Initialization method. On success, a subsequent call to GetStatus returns EObjectStatus::running
     * @param[in] ssObjectConfig Optional configuration string.
     */
    virtual void Initialize([[maybe_unused]] const sdv::u8string& ssObjectConfig)
    {
        m_eObjectStatus = sdv::EObjectStatus::initialization_failure;
    }

    /**
     * @brief Gets the current status of the object
     * @return EObjectStatus The current status of the object
     */
    virtual sdv::EObjectStatus GetStatus() const
    {
        return m_eObjectStatus;
    }

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode)
    {
        switch (eMode)
        {
        case sdv::EOperationMode::configuring:
            if (m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::configuring;
            break;
        case sdv::EOperationMode::running:
            if (m_eObjectStatus == sdv::EObjectStatus::configuring || m_eObjectStatus == sdv::EObjectStatus::initialized)
                m_eObjectStatus = sdv::EObjectStatus::running;
            break;
        default:
            break;
        }
    }

    /**
     * @brief Shutdown method called before the object is destroyed.
     * @attention Implement calls to other SDV objects here as this is no longer considered safe during the destructor of the object!
     * After a call to shutdown any threads/callbacks/etc that could call other SDV objects need to have been stopped.
     * The SDV object itself is to remain in a state where it can respond to calls to its interfaces as other objects may still call it during the shutdown sequence!
     * Any subsequent call to GetStatus should return EObjectStatus::destruction_pending
     */
    virtual void Shutdown()
    {
        m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;

        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    }

private:
    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
};

DEFINE_SDV_OBJECT(CTestObjectControlFail)

