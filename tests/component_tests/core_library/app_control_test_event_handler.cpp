#include <gtest/gtest.h>
#include <support/sdv_core.h>
#include <interfaces/app.h>
#include <support/mem_access.h>
#include <support/app_control.h>
#include <list>
#include <mutex>
#include "../../../sdv_services/core/local_shutdown_request.h"
#include "../../../global/base64.h"

/**
 * @brief Helper class for receiving application events.
 */
class CAppEventHandler : public sdv::IInterfaceAccess, public sdv::app::IAppEvent
{
public:
    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::app::IAppEvent)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Process the event. Overload of IAppEvent::ProcessEvent.
     * @param[inout] sEvent Event structure containing the information to process.
     */
    virtual void ProcessEvent(/*inout*/ sdv::app::SAppEvent& sEvent) override
    {
        std::unique_lock<std::mutex> lock(m_mtxEventList);
        switch (sEvent.uiEventID)
        {
        case sdv::app::EVENT_OPERATION_STATE_CHANGED:
            m_lstEventList.push_back(sEvent);
            break;
        case sdv::app::EVENT_RUNNING_LOOP:
            m_nLoopCount++;
            break;
        default:
            break;
        }
    }

    /**
    * @brief Get the event list.
    * @return A copy of the event list.
    */
    std::list<sdv::app::SAppEvent> GetEventList() const
    {
        std::unique_lock<std::mutex> lock(m_mtxEventList);
        return m_lstEventList;
    }

    /**
    * @brief Reset the event list.
    */
    void ResetEventList()
    {
        std::unique_lock<std::mutex> lock(m_mtxEventList);
        m_lstEventList.clear();
    }

    /**
     * @brief Get the loop counter value.
     * @return The loop count.
     */
    size_t GetLoopCount() { return m_nLoopCount; }

private:
    mutable std::mutex              m_mtxEventList;     ///< Event list protection
    std::list<sdv::app::SAppEvent>  m_lstEventList;     ///< Event list
    size_t                          m_nLoopCount = 0;   ///< Loop counter
};

TEST(CoreLibrary_AppControl_EventHandler, Startup_Invalid_ApplicationMode)
{
    // Prevent error reporting on std::cerr - they will influence test outcome.
    auto* pCErr = std::cerr.rdbuf();
    std::ostringstream sstreamCErr;
    std::cerr.rdbuf(sstreamCErr.rdbuf());

    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Invalid\"", &handler);
    EXPECT_FALSE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    std::cerr.rdbuf(pCErr);
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Default_NoConfig)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Standalone_NoConfig)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Standalone\"", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_External_NoConfig)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"External\"", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::external);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Isolated_NoConfig)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup(R"code(
[Application]
Mode = "Isolated"

[Console]
Report = "Silent"
)code", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::isolated);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Main_NoConfig)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Main\"", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::main);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Essential_NoConfig)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Essential\"", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::essential);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Maintenance_NoConfig)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Maintenance\"", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::maintenance);
    EXPECT_EQ(pContext->GetInstanceID(), 1000u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Default_DefineInstance)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nInstance=2005", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Standalone_DefineInstance)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Standalone\"\nInstance=2005", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_External_DefineInstance)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"External\"\nInstance=2005", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::external);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Isolated_DefineInstance)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    std::string ssConfig = R"code([Application]
Mode = "Isolated"
Instance = 2005
Connection = ")code";
    ssConfig += Base64EncodePlainText("test") + "\"";
    bool bResult = pControl->Startup(ssConfig, &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::isolated);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Main_DefineInstance)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Main\"\nInstance=2005", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::main);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Essential_DefineInstance)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Essential\"\nInstance=2005", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::essential);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, Startup_Maintenance_DefineInstance)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Maintenance\"\nInstance=2005", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::maintenance);
    EXPECT_EQ(pContext->GetInstanceID(), 2005u);

    sdv::app::EAppOperationState rgeStartupStates[] = {
        sdv::app::EAppOperationState::initializing,
        sdv::app::EAppOperationState::initialized,
        sdv::app::EAppOperationState::configuring,
        sdv::app::EAppOperationState::running
    };
    auto lstEventList = handler.GetEventList();
    size_t nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeStartupStates)>)
                EXPECT_EQ(rgeStartupStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeStartupStates)>);
        }
    }
    handler.ResetEventList();

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    sdv::app::EAppOperationState rgeShutdownStates[] = {
        sdv::app::EAppOperationState::shutting_down,
        sdv::app::EAppOperationState::not_started
    };
    lstEventList = handler.GetEventList();
    nIndex = 0;
    for (const auto& rsEvent : lstEventList)
    {
        if (rsEvent.uiEventID == sdv::app::EVENT_OPERATION_STATE_CHANGED)
        {
            if (nIndex < std::extent_v<decltype(rgeShutdownStates)>)
                EXPECT_EQ(rgeShutdownStates[nIndex++], static_cast<sdv::app::EAppOperationState>(rsEvent.uiInfo));
            else
                EXPECT_LT(nIndex, std::extent_v<decltype(rgeShutdownStates)>);
        }
    }
}

TEST(CoreLibrary_AppControl_EventHandler, RunLoop_Default)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nInstance=2007", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    EXPECT_GT(handler.GetLoopCount(), 5u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_EventHandler, RunLoop_Standalone)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Standalone\"\nInstance=2007", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::standalone);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    EXPECT_GT(handler.GetLoopCount(), 5u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_EventHandler, RunLoop_External)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"External\"\nInstance=2007", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::external);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    EXPECT_GT(handler.GetLoopCount(), 5u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_EventHandler, RunLoop_Isolated)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    std::string ssConfig = R"code([Application]
Mode = "Isolated"
Instance = 2007
Connection = ")code";
    ssConfig += Base64EncodePlainText("test") + "\"";
    bool bResult = pControl->Startup(ssConfig, &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::isolated);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            sdv::app::IAppShutdownRequest* pShutdownRequest = sdv::core::GetObject<sdv::app::IAppShutdownRequest>("AppControlService");
            ASSERT_NE(pShutdownRequest, nullptr);
            pShutdownRequest->RequestShutdown();
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    EXPECT_GT(handler.GetLoopCount(), 5u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_EventHandler, RunLoop_Main)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Main\"\nInstance=2007", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::main);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            sdv::app::IAppShutdownRequest* pShutdownRequest = sdv::core::GetObject<sdv::app::IAppShutdownRequest>("AppControlService");
            ASSERT_NE(pShutdownRequest, nullptr);
            pShutdownRequest->RequestShutdown();
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    EXPECT_GT(handler.GetLoopCount(), 5u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_EventHandler, RunLoop_Essential)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Essential\"\nInstance=2007", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::essential);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    auto tpStart = std::chrono::high_resolution_clock::now();
    std::thread threadShutdownRequest([]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            RequestShutdown(2007u);
        });
    pControl->RunLoop();
    EXPECT_GT(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tpStart).count(), 0.100);
    threadShutdownRequest.join();

    EXPECT_GT(handler.GetLoopCount(), 5u);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

TEST(CoreLibrary_AppControl_EventHandler, RunLoop_Maintenance)
{
    CAppEventHandler handler;
    sdv::app::IAppControl* pControl = sdv::core::GetCore<sdv::app::IAppControl>();
    ASSERT_NE(pControl, nullptr);
    sdv::app::IAppContext* pContext = sdv::core::GetCore<sdv::app::IAppContext>();
    ASSERT_NE(pContext, nullptr);
    sdv::app::IAppOperation* pOperation = sdv::core::GetCore<sdv::app::IAppOperation>();
    ASSERT_NE(pOperation, nullptr);

    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);

    bool bResult = pControl->Startup("[Application]\nMode=\"Maintenance\"\nInstance=2007", &handler);
    EXPECT_TRUE(bResult);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::running);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::maintenance);
    EXPECT_EQ(pContext->GetInstanceID(), 2007u);

    // Loop not allowed...
    EXPECT_THROW(pControl->RunLoop(), sdv::XAccessDenied);

    pControl->Shutdown(true);
    EXPECT_EQ(pOperation->GetOperationState(), sdv::app::EAppOperationState::not_started);
    EXPECT_EQ(pContext->GetContextType(), sdv::app::EAppContext::no_context);
    EXPECT_EQ(pContext->GetInstanceID(), 0u);
}

