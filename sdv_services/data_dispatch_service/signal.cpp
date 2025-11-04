#include "dispatchservice.h"
#include "signal.h"
#include "trigger.h"
#include "transaction.h"

CProvider::CProvider(CSignal& rSignal) : m_rSignal(rSignal)
{}

void CProvider::DestroyObject()
{
    m_rSignal.RemoveProvider(this);
}

void CProvider::Write(/*in*/ sdv::any_t anyVal, /*in*/ sdv::IInterfaceAccess* pTransaction)
{
    // Let the transaction handle the write if there is a transaction.
    CTransaction* pTransactionObj = static_cast<CTransaction*>(pTransaction);
    if (pTransactionObj)
    {
        pTransactionObj->DeferWrite(m_rSignal, anyVal);
        return;
    }

    // Write the value
    // Note, explicitly convert to uint64_t to resolve ambiguous function selection under Linux.
    std::set<CTrigger*> setTriggers;
    m_rSignal.WriteFromProvider(anyVal, static_cast<uint64_t>(0), setTriggers);

    // Execute the triggers
    for (CTrigger* pTrigger : setTriggers)
        pTrigger->Execute();

}

CConsumer::CConsumer(CSignal& rSignal, sdv::IInterfaceAccess* pEvent /*= nullptr*/) :
    m_rSignal(rSignal)
{
    if (pEvent) m_pEvent = pEvent->GetInterface<sdv::core::ISignalReceiveEvent>();
}

void CConsumer::DestroyObject()
{
    m_rSignal.RemoveConsumer(this);
}

sdv::any_t CConsumer::Read(/*in*/ sdv::IInterfaceAccess* pTransaction) const
{
    const CTransaction* pTransactionObj = static_cast<const CTransaction*>(pTransaction);

    // Determine the transaction ID to use
    uint64_t uiTransactionID = 0;
    if (pTransactionObj) uiTransactionID = pTransactionObj->GetReadTransactionID();

    // Request a read from the signal
    return m_rSignal.ReadFromConsumer(uiTransactionID);
}

void CConsumer::Distribute(const sdv::any_t& ranyVal)
{
    if (m_pEvent) m_pEvent->Receive(ranyVal);
}

CSignal::CSignal(CDispatchService& rDispatchSvc, const sdv::u8string& rssName, sdv::core::ESignalDirection eDirection,
    sdv::any_t anyDefVal /*= sdv::any_t()*/) :
    m_rDispatchSvc(rDispatchSvc), m_ssName(rssName), m_eDirection(eDirection), m_anyDefVal(anyDefVal)
{
    for (auto& rprVal : m_rgprVal)
        rprVal = {0, anyDefVal};
}

CSignal::~CSignal()
{}

sdv::u8string CSignal::GetName() const
{
    return m_ssName;
}

sdv::core::ESignalDirection CSignal::GetDirection() const
{
    return m_eDirection;
}

sdv::any_t CSignal::GetDefVal() const
{
    return m_anyDefVal;
}

CProvider* CSignal::CreateProvider()
{
    std::unique_lock<std::mutex> lock(m_mtxSignalObjects);
    auto ptrProvider = std::make_unique<CProvider>(*this);
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an exception was
    // triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrProvider)
        return nullptr;
    CProvider* pObject = ptrProvider.get();
    if (pObject)
        m_mapProviders.try_emplace(pObject, std::move(ptrProvider));
    else
        RemoveProvider(nullptr); // Cleanup
    return pObject;
}

void CSignal::RemoveProvider(CProvider* pProvider)
{
    std::unique_lock<std::mutex> lock(m_mtxSignalObjects);
    m_mapProviders.erase(pProvider);
    bool bUnregister = m_mapProviders.empty() && m_mapConsumers.empty();
    lock.unlock();
    if (bUnregister)
        m_rDispatchSvc.UnregisterSignal(m_ssName, m_eDirection);
}

CConsumer* CSignal::CreateConsumer(sdv::IInterfaceAccess* pEvent /*= nullptr*/)
{
    std::unique_lock<std::mutex> lock(m_mtxSignalObjects);
    auto ptrConsumer = std::make_unique<CConsumer>(*this, pEvent);
    // Ignore cppcheck warning; normally the returned pointer should always have a value at this stage (otherwise an exception was
    // triggered).
    // cppcheck-suppress knownConditionTrueFalse
    if (!ptrConsumer)
        return nullptr;
    CConsumer* pObject = ptrConsumer.get();
    if (pObject)
        m_mapConsumers.try_emplace(pObject, std::move(ptrConsumer));
    else
        RemoveConsumer(nullptr); // Cleanup
    return pObject;
}

void CSignal::RemoveConsumer(CConsumer* pConsumer)
{
    std::unique_lock<std::mutex> lock(m_mtxSignalObjects);
    m_mapConsumers.erase(pConsumer);
    if (m_mapConsumers.empty())
    {
        // Remove any triggers
        std::unique_lock<std::mutex> lockTrigger(m_mtxTriggers);
        while (!m_setTriggers.empty())
        {
            CTrigger* pTrigger = *m_setTriggers.begin();
            m_setTriggers.erase(m_setTriggers.begin());
            lockTrigger.unlock();

            // Remove this signal from the trigger.
            pTrigger->RemoveSignal(m_ssName);

            lockTrigger.lock();
        }
    }

    bool bUnregister = m_mapProviders.empty() && m_mapConsumers.empty();
    lock.unlock();
    if (bUnregister)
        m_rDispatchSvc.UnregisterSignal(m_ssName, m_eDirection);
}

void CSignal::WriteFromProvider(const sdv::any_t& ranyVal, uint64_t uiTransactionID, std::set<CTrigger*>& rsetTriggers)
{
    if (m_rDispatchSvc.GetStatus() != sdv::EObjectStatus::running) return;

    uint64_t uiTransactionIDTemp = uiTransactionID;
    if (!uiTransactionIDTemp) uiTransactionIDTemp = m_rDispatchSvc.GetDirectTransactionID();

    // Store the value
    std::unique_lock<std::mutex> lock(m_mtxVal);
    if (m_nValIndex >= std::extent_v<decltype(m_rgprVal)>) m_nValIndex = 0;     // TODO: This is a serious error.
    size_t nTargetIndex = m_nValIndex;

    // Create an entry with the current transaction ID
    if (m_rgprVal[nTargetIndex].first < uiTransactionIDTemp)
    {
        nTargetIndex = (nTargetIndex + 1) % std::extent_v<decltype(m_rgprVal)>;
        m_rgprVal[nTargetIndex].first = uiTransactionIDTemp;
        m_nValIndex = nTargetIndex;
    }

    // Update the value
    m_rgprVal[nTargetIndex].second = ranyVal;
    lock.unlock();

    // Add all triggers to the set of triggers
    std::unique_lock<std::mutex> lockTriggers(m_mtxTriggers);
    for (CTrigger* pTrigger : m_setTriggers)
        rsetTriggers.insert(pTrigger);
    lockTriggers.unlock();

    // Trigger the update event.
    DistributeToConsumers(ranyVal);
}

sdv::any_t CSignal::ReadFromConsumer(uint64_t uiTransactionID) const
{
    std::unique_lock<std::mutex> lock(m_mtxVal);

    if (m_nValIndex >= std::extent_v<decltype(m_rgprVal)>) return {};   // TODO: This is a serious error.

    // Determine the transaction ID to use
    uint64_t uiTransactionIDCopy = uiTransactionID;
    size_t nTargetIndex = m_nValIndex;
    if (!uiTransactionIDCopy) uiTransactionIDCopy = m_rgprVal[nTargetIndex].first;

    // Find the signal with the same or lower transaction ID
    while (m_rgprVal[m_nValIndex].first > uiTransactionIDCopy)  // Value too new
    {
        // Search for the index with the transaction ID.
        // Ignore cppcheck comparison operator (template is interpreted as > operator)
        // cppcheck-suppress compareBoolExpressionWithInt
        // cppcheck-suppress knownConditionTrueFalse
        nTargetIndex = (nTargetIndex + std::extent_v<decltype(m_rgprVal)> - 1) % std::extent_v<decltype(m_rgprVal)>;
        if (nTargetIndex == m_nValIndex) return m_anyDefVal; // Transaction too old... cannot update
    }

    // Fitting transaction found. Get the value.
    sdv::any_t anyVal = m_rgprVal[nTargetIndex].second;

    return anyVal;
}

void CSignal::DistributeToConsumers(const sdv::any_t& ranyVal)
{
    if (m_rDispatchSvc.GetStatus() != sdv::EObjectStatus::running) return;

    std::unique_lock<std::mutex> lock(m_mtxSignalObjects);
    for (auto& rvtConsumer : m_mapConsumers)
        rvtConsumer.second->Distribute(ranyVal);
}

void CSignal::AddTrigger(CTrigger* pTrigger)
{
    std::unique_lock<std::mutex> lock(m_mtxTriggers);
    m_setTriggers.emplace(pTrigger);
}

void CSignal::RemoveTrigger(CTrigger* pTrigger)
{
    std::unique_lock<std::mutex> lock(m_mtxTriggers);
    m_setTriggers.erase(pTrigger);
}

bool CSignal::EqualsDefaultValue() const
{
    return ReadFromConsumer(0) == m_anyDefVal;
}
