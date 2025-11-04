#include "dispatchservice.h"


CDispatchService::CDispatchService()
{
    CreateDirectTransactionID();
}

sdv::IInterfaceAccess* CDispatchService::CreateTxTrigger(uint32_t uiCycleTime, uint32_t uiDelayTime, uint32_t uiBehaviorFlags,
    sdv::IInterfaceAccess* pTriggerCallback)
{
    if (m_eObjectStatus != sdv::EObjectStatus::configuring) return nullptr;

    // Check for parameter validity
    if (!uiCycleTime && !(uiBehaviorFlags & static_cast<uint32_t>(sdv::core::ISignalTransmission::ETxTriggerBehavior::spontaneous)))
        return nullptr;
    if (!pTriggerCallback)
        return nullptr;
    sdv::core::ITxTriggerCallback* pCallback = pTriggerCallback->GetInterface<sdv::core::ITxTriggerCallback>();
    if (!pCallback) return nullptr;

    std::unique_ptr<CTrigger> ptrTrigger = std::make_unique<CTrigger>(*this, uiCycleTime, uiDelayTime, uiBehaviorFlags, pCallback);
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrTrigger)
        return nullptr;
    CTrigger* pObject = ptrTrigger.get();
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an
    // exception was triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!pObject)
        return nullptr;
    if (!ptrTrigger->IsValid()) return nullptr;

    std::unique_lock<std::mutex> lock(m_mtxTriggers);
    return m_mapTriggers.emplace(pObject, std::move(ptrTrigger)).second ? pObject : nullptr;
}

sdv::IInterfaceAccess* CDispatchService::RegisterTxSignal(/*in*/ const sdv::u8string& ssSignalName, /*in*/ sdv::any_t anyDefVal)
{
    if (m_eObjectStatus != sdv::EObjectStatus::configuring) return nullptr;

    std::unique_lock<std::mutex> lock(m_mtxSignals);

    auto prSignal = m_mapTxSignals.try_emplace(ssSignalName, *this, ssSignalName, sdv::core::ESignalDirection::sigdir_tx, anyDefVal);
    return prSignal.first->second.CreateConsumer();
}

sdv::IInterfaceAccess* CDispatchService::RegisterRxSignal(/*in*/ const sdv::u8string& ssSignalName)
{
    if (m_eObjectStatus != sdv::EObjectStatus::configuring) return nullptr;

    std::unique_lock<std::mutex> lock(m_mtxSignals);
    auto prSignal = m_mapRxSignals.try_emplace(ssSignalName, *this, ssSignalName, sdv::core::ESignalDirection::sigdir_rx);
    return prSignal.first->second.CreateProvider();
}

sdv::IInterfaceAccess* CDispatchService::RequestSignalPublisher(/*in*/ const sdv::u8string& ssSignalName)
{
    if (m_eObjectStatus != sdv::EObjectStatus::configuring) return nullptr;

    std::unique_lock<std::mutex> lock(m_mtxSignals);
    auto itSignal = m_mapTxSignals.find(ssSignalName);
    if (itSignal == m_mapTxSignals.end()) return nullptr;
    if (itSignal->second.GetDirection() != sdv::core::ESignalDirection::sigdir_tx)
        return nullptr;
    return itSignal->second.CreateProvider();
}

sdv::IInterfaceAccess* CDispatchService::AddSignalSubscription(/*in*/ const sdv::u8string& ssSignalName, /*in*/ IInterfaceAccess* pSubscriber)
{
    if (m_eObjectStatus != sdv::EObjectStatus::configuring) return nullptr;

    std::unique_lock<std::mutex> lock(m_mtxSignals);
    auto itSignal = m_mapRxSignals.find(ssSignalName);
    if (itSignal == m_mapRxSignals.end()) return 0;
    if (itSignal->second.GetDirection() != sdv::core::ESignalDirection::sigdir_rx)
        return 0ull;
    return itSignal->second.CreateConsumer(pSubscriber);
}

sdv::sequence<sdv::core::SSignalRegistration> CDispatchService::GetRegisteredSignals() const
{
    sdv::sequence<sdv::core::SSignalRegistration> seqRegistrations;
    std::unique_lock<std::mutex> lock(m_mtxSignals);
    for (const auto& prSignal : m_mapRxSignals)
        seqRegistrations.push_back({prSignal.first, prSignal.second.GetDirection()});
    for (const auto& prSignal : m_mapTxSignals)
        seqRegistrations.push_back({prSignal.first, prSignal.second.GetDirection()});
    return seqRegistrations;
}

sdv::IInterfaceAccess* CDispatchService::CreateTransaction()
{
    // NOTE: Transactions can take place at any time (not only when running). DO not restrict transactions to any
    // operation mode.

    std::unique_lock<std::mutex> lock(m_mtxTransactions);
    auto itTransaction = m_lstTransactions.emplace(m_lstTransactions.end(), *this);
    if (itTransaction == m_lstTransactions.end()) return nullptr;
    itTransaction->SetIterator(itTransaction);
    return &(*itTransaction);
}

uint64_t CDispatchService::GetNextTransactionID()
{
    return m_uiTransactionCnt++;
}

void CDispatchService::CreateDirectTransactionID()
{
    m_uiDirectTransactionID = GetNextTransactionID();
}

uint64_t CDispatchService::GetDirectTransactionID() const
{
    return m_uiDirectTransactionID;
}

void CDispatchService::Initialize(const sdv::u8string& /*ssObjectConfig*/)
{
    m_eObjectStatus = sdv::EObjectStatus::initializing;
    m_scheduler.Start();
    m_eObjectStatus = sdv::EObjectStatus::initialized;
}

sdv::EObjectStatus CDispatchService::GetStatus() const
{
    return m_eObjectStatus;
}

void CDispatchService::SetOperationMode(sdv::EOperationMode eMode)
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

void CDispatchService::Shutdown()
{
    m_eObjectStatus = sdv::EObjectStatus::shutdown_in_progress;

    m_scheduler.Stop();

    m_eObjectStatus = sdv::EObjectStatus::initialization_pending;
}

void CDispatchService::UnregisterSignal(/*in*/ const sdv::u8string& ssSignalName, sdv::core::ESignalDirection eDirection)
{
    // NOTE: Normally the remove function should be called in the configuration mode. Since it doesn't give
    // feedback and the associated caller might delete any receiving function, allow the removal to take place even
    // when running.

    std::unique_lock<std::mutex> lock(m_mtxSignals);

    // Remove the signal
    if (eDirection == sdv::core::ESignalDirection::sigdir_rx)
        m_mapRxSignals.erase(ssSignalName);
    else
        m_mapTxSignals.erase(ssSignalName);
}

CSignal* CDispatchService::FindSignal(const sdv::u8string& rssSignalName, sdv::core::ESignalDirection eDirection)
{
    std::unique_lock<std::mutex> lock(m_mtxSignals);

    if (eDirection == sdv::core::ESignalDirection::sigdir_rx)
    {
        auto itSignal = m_mapRxSignals.find(rssSignalName);
        return itSignal == m_mapRxSignals.end() ? nullptr : &itSignal->second;
    } else
    {
        auto itSignal = m_mapTxSignals.find(rssSignalName);
        return itSignal == m_mapTxSignals.end() ? nullptr : &itSignal->second;
    }
}

void CDispatchService::FinishTransaction(const CTransaction* pTransaction)
{
    // NOTE: Transactions can take place at any time (not only when running). DO not restrict transactions to any
    // operation mode.

    std::unique_lock<std::mutex> lock(m_mtxTransactions);

    // Delete the transaction
    auto itTransaction = pTransaction->GetIterator();
    m_lstTransactions.erase(itTransaction);
}

CScheduler& CDispatchService::GetScheduler()
{
    return m_scheduler;
}

void CDispatchService::RemoveTxTrigger(CTrigger* pTrigger)
{
    // NOTE: Normally the remove function should be called in the configuration mode. Since it doesn't give
    // feedback and the associated caller might delete any receiving function, allow the removal to take place even
    // when running.

    std::unique_lock<std::mutex> lock(m_mtxTriggers);
    m_scheduler.RemoveFromSchedule(pTrigger);
    m_mapTriggers.erase(pTrigger);
}
