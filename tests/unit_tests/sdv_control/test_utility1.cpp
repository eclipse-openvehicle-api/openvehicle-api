#include <gtest/gtest.h>
#include <support/component_impl.h>
#include "generated/test_component.h"

/**
 * @brief Utility test component #1
 */
class CTestUtility1 : public sdv::CSdvObject, public sdv::IObjectControl, public IHello

{
public:
    /**
     * @brief Constructor
     */
    ~CTestUtility1()
    {
        EXPECT_EQ(m_eObjectStatus, sdv::EObjectStatus::destruction_pending);
    }

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(IHello)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Utility)
    DECLARE_OBJECT_CLASS_NAME("SDVControl_Test_Utility1")

    /**
     * @brief Initialize method. On success, a subsequent call to GetStatus returns EObjectStatus::running
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
        EXPECT_TRUE(m_eObjectStatus == sdv::EObjectStatus::running || m_eObjectStatus == sdv::EObjectStatus::initialized ||
            m_eObjectStatus == sdv::EObjectStatus::configuring);
        if (m_eObjectStatus != sdv::EObjectStatus::running && m_eObjectStatus != sdv::EObjectStatus::initialized
            && m_eObjectStatus != sdv::EObjectStatus::configuring)
            std::cout << "Object status = " << static_cast<uint32_t>(m_eObjectStatus) << " (expected initialized=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::initialized) << " or configuring=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::configuring) << " or running=" <<
            static_cast<uint32_t>(sdv::EObjectStatus::running) << ")." << std::endl;

        m_eObjectStatus = sdv::EObjectStatus::destruction_pending;
    }

    /**
     * @brief Say hello. Overload of IHello::SayHello.
     * @return The greetings string.
     */
    virtual sdv::u8string SayHello() const override
    {
        return "Hello from utility #1";
    }


private:
    sdv::EObjectStatus m_eObjectStatus = sdv::EObjectStatus::initialization_pending;        ///< Object status
};

DEFINE_SDV_OBJECT(CTestUtility1)

