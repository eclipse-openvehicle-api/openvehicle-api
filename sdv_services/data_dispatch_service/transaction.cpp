#include "dispatchservice.h"
#include "transaction.h"
#include "trigger.h"
#include "signal.h"

CTransaction::CTransaction(CDispatchService& rDispatchSvc) :
    m_rDispatchSvc(rDispatchSvc), m_uiReadTransactionID(rDispatchSvc.GetNextTransactionID())
{}

void CTransaction::DestroyObject()
{
    FinalizeWrite();
    m_rDispatchSvc.FinishTransaction(this);
}

uint64_t CTransaction::GetReadTransactionID() const
{
    // Set transaction to write if not done so.
    if (m_eTransactionType != ETransactionType::read_transaction)
    {
        if (m_eTransactionType == ETransactionType::undefined)
        {
            m_eTransactionType = ETransactionType::read_transaction;

            // Create a new direct transaction ID to prevent the current values from overwriting
            m_rDispatchSvc.CreateDirectTransactionID();
        }
        else
            return 0ull;
    }

    return m_uiReadTransactionID;
}

void CTransaction::DeferWrite(CSignal& rSignal, sdv::any_t& ranyVal)
{
    // Set transaction to write if not done so.
    if (m_eTransactionType != ETransactionType::write_transaction)
    {
        if (m_eTransactionType == ETransactionType::undefined)
            m_eTransactionType = ETransactionType::write_transaction;
        else
            return;
    }

    // Add the value to the deferred signal map.
    std::unique_lock<std::mutex> lock(m_mtxDeferredWriteSignalMap);
    m_mapDeferredWriteSignalMap.insert_or_assign(&rSignal, ranyVal);
}

void CTransaction::FinalizeWrite()
{
    // Set transaction to write if not done so.
    if (m_eTransactionType != ETransactionType::write_transaction)
    {
        if (m_eTransactionType == ETransactionType::undefined)
            m_eTransactionType = ETransactionType::write_transaction;
        else
            return;
    }

    // Get the next transaction ID
    uint64_t uiWriteTransaction = m_rDispatchSvc.GetNextTransactionID();

    // Write the signals with the transaction ID.
    std::unique_lock<std::mutex> lock(m_mtxDeferredWriteSignalMap);
    std::set<CTrigger*> setTriggers;
    for (auto& rvtDeferredSignal : m_mapDeferredWriteSignalMap)
    {
        if (!rvtDeferredSignal.first) continue;
        rvtDeferredSignal.first->WriteFromProvider(rvtDeferredSignal.second, uiWriteTransaction, setTriggers);
    }
    m_mapDeferredWriteSignalMap.clear();
    lock.unlock();

    // Execute the triggers
    for (CTrigger* pTrigger : setTriggers)
        pTrigger->Execute();
}

void CTransaction::SetIterator(std::list<CTransaction>::iterator itTransaction)
{
    m_itTransaction = itTransaction;
}

std::list<CTransaction>::iterator CTransaction::GetIterator() const
{
    return m_itTransaction;
}

