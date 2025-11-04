#ifndef DISPATCH_SERVICE_H
#define DISPATCH_SERVICE_H

#include <interfaces/dispatch.h>
#include <support/component_impl.h>
#include <memory>
#include <map>
#include <list>
#include <set>

// Data dispatch service for CAN:
//
// - CAN Link object for Lotus Eletre registers all the CAN signals as follows:
// - Rx Signals are being registered to receive data from the vehicle and supply data to the vehicle devices.
//      - Signals are all grouped per message and per node/ECU
//      - One event is sent to update the data in the vehicle devices
//      - Data is updated first and then the event is broadcasted (toggle buffer)
// - Tx Signals are being registered to receive data from the vehicle devices and supply data to the vehicle
//      - Signals are grouped per message and per node/ECU
//      - (Default values are available for each signal; either one time reset or reset after each send) -> service task?
//      - Sending per event or per timer

#include "transaction.h"
#include "signal.h"
#include "trigger.h"

/**
* @brief data dispatch service to read/write and react on signal changes
*/
class CDispatchService : public sdv::CSdvObject, public sdv::core::ISignalTransmission, public sdv::core::ISignalAccess,
    public sdv::core::IDispatchTransaction, public sdv::IObjectControl
{
public:
    /**
     * @brief Constructor
     */
    CDispatchService();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::core::ISignalTransmission)
        SDV_INTERFACE_ENTRY(sdv::core::ISignalAccess)
        SDV_INTERFACE_ENTRY(sdv::core::IDispatchTransaction)
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("DataDispatchService")
    DECLARE_OBJECT_SINGLETON()
    DECLARE_OBJECT_DEPENDENCIES("TaskTimerService")

    /**
    * @brief Create a TX trigger object that defines how to trigger the signal transmission. Overload of
    * sdv::core::ISignalTransmission::CreateTxTrigger.
    * @param[in] uiCycleTime When set to any value other than 0, provides a cyclic trigger (ms). Could be 0 if cyclic
    * triggering is not required.
    * @param[in] uiDelayTime When set to any value other than 0, ensures a minimum time between two triggers. Could be 0
    * if minimum time should not be enforced.
    * @param[in] uiBehaviorFlags Zero or more flags from sdv::core::ETxTriggerBehavior.
    * @param[in] pTriggerCallback Pointer to the trigger callback object. This object needs to expose the ITxTriggerCallback
    * interface. This interface must stay valid during the lifetime of the generated trigger object.
    * @returns On success, returns an interface to the trigger object. Use the ITxTrigger interface to assign signals to
    * the trigger. Returns null when a trigger was requested without cycletime and without trigger behavior (which would
    * mean it would never trigger). Use IObjectDestroy to destroy the trigger object.
    */
    IInterfaceAccess* CreateTxTrigger(uint32_t uiCycleTime, uint32_t uiDelayTime, uint32_t uiBehaviorFlags,
        sdv::IInterfaceAccess* pTriggerCallback);

    /**
     * @brief Register a signal for sending over the network; reading from the dispatch service. Data is provided by the
     * signal publisher and dependable on the requested behavior stored until it is sent. Overload of
     * sdv::core::ISignalTransmission::RegisterTxSignal.
     * @param[in] ssSignalName Name of the signal. To guarantee uniqueness, it is preferred to add the group hierarchy to the
     * signal name separated by a dot. E.g. with CAN: MAB.BcmChas1Fr03.SteerReCtrlReqAgReq
     * @param[in] anyDefVal The default value of the signal.
     * @return Returns the IInterfaceAccess interface that allows access to the ISignalRead interface for reading the
     * signal value.
     */
    virtual sdv::IInterfaceAccess* RegisterTxSignal(/*in*/ const sdv::u8string& ssSignalName, /*in*/ sdv::any_t anyDefVal) override;

    /**
     * @brief Register a signal for reception over the network; providing to the dispatch service. Overload of
     * sdv::core::ISignalTransmission::RegisterRxSignal.
     * @param[in] ssSignalName Name of the signal. To guarantee uniqueness, it is preferred to add the group hierarchy to the
     * signal name separated by a dot. E.g. with CAN: MAB.BcmChas1Fr03.SteerReCtrlReqAgReq
     * @return Returns the IInterfaceAccess interface that allows access to the ISignalWrite interface for writing the
     * signal value.
     */
    virtual sdv::IInterfaceAccess* RegisterRxSignal(/*in*/ const sdv::u8string& ssSignalName) override;

    /**
     * @brief Requested a registered signal for publication (send signal). Overload of
     * sdv::core::ISignalAccess::RequestSignalPublisher.
     * @param[in] ssSignalName Name of the signal. To guarantee uniqueness, it is preferred to add the group hierarchy to the
     * signal name separated by a dot. E.g. with CAN: MAB.BcmChas1Fr03.SteerReCtrlReqAgReq
     * @return Returns the IInterfaceAccess interface that allows access to the ISignalWrite interface for writing the
     * signal value.
     */
    virtual sdv::IInterfaceAccess* RequestSignalPublisher(/*in*/ const sdv::u8string& ssSignalName) override;

    /**
     * @brief Add a registered signal for subscription (receive signal). Overload of
     * sdv::core::ISignalAccess::AddSignalSubscription.
     * @param[in] ssSignalName Name of the signal. To guarantee uniqueness, it is preferred to add the group hierarchy to the
     * signal name separated by a dot. E.g. with CAN: MAB.BcmChas1Fr03.SteerReCtrlReqAgReq
     * @param[in] pSubscriber Pointer to the IInterfaceAccess of the subscriber. The subscriber should implement the
     * ISignalReceiveEvent interface.
     * @return Returns an interface that can be used to manage the subscription.  Use IObjectDestroy to destroy the signal object.
     */
    virtual sdv::IInterfaceAccess* AddSignalSubscription(/*in*/ const sdv::u8string& ssSignalName, /*in*/ sdv::IInterfaceAccess* pSubscriber) override;

    /**
     * @brief Get a list of registered signals.
     * @return List of registration functions.
     */
    virtual sdv::sequence<sdv::core::SSignalRegistration> GetRegisteredSignals() const override;

    /**
    * @brief CreateTransaction a transaction. Overload of sdv::core::IDispatchTransaction::CreateTransaction.
    * @details When starting a group transaction, any writing to a signal will not be reflected yet until the transaction
    * is finalized. For the data link layer, this also allows freezing the reading values until all values have been read.
    * @return Returns the transaction interface or NULL when the transaction could not be started. Use IObjectDestroy to
    * destroy the transaction object.
    */
    virtual sdv::IInterfaceAccess* CreateTransaction() override;

    /**
     * @brief Get the next transaction ID.
     * @return Returns the next transaction ID.
     */
    uint64_t GetNextTransactionID();

    /**
     * @brief Create a new direct transaction ID.
     * @details The read transaction ID is used fir direct transmission after a read transaction was started. This prevents direct
     * transmission overwriting the protected read transaction.
     */
    void CreateDirectTransactionID();

    /**
     * @brief Get the current direct transaction ID.
     * @return The transaction ID.
     */
    uint64_t GetDirectTransactionID() const;

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    void Initialize(const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    void Shutdown() override;

    /**
    * @brief Unregister a previously registered signal. This will render all subscriptions and provider connections invalid.
    * @param[in] ssSignalName Name of the signal to unregister.
    * @param[in] eDirection The signal direction determines from which map the signal should be unregistered.
    */
    void UnregisterSignal(const sdv::u8string& ssSignalName, sdv::core::ESignalDirection eDirection);

    /**
     * @brief Find the signal with the supplied name.
     * @param[in] rssSignalName Name of the signal to find.
     * @param[in] eDirection The signal direction determines at which map the signal should be searched for.
     * @return Pointer to the signal or NULL when the signal could not be found.
     */
    CSignal* FindSignal(const sdv::u8string& rssSignalName, sdv::core::ESignalDirection eDirection);

    /**
    * @brief Finalize a transaction transaction. Any update made on this interface between the start and the finalize will be
    * in effect at once.
    * @param[in] pTransaction The transaction to finalize.
    */
    void FinishTransaction(const CTransaction* pTransaction);

    /**
     * @brief Get the trigger execution scheduler.
     * @return Returns a reference to the contained trigger execution scheduler.
    */
    CScheduler& GetScheduler();

    /**
     * @brief Remove a trigger object from the trigger map.
     * @param[in] pTrigger Pointer to the trigger object to remove.
     */
    void RemoveTxTrigger(CTrigger* pTrigger);

private:
    mutable std::mutex                              m_mtxSignals;                           ///< Signal object map protection.
    std::map<sdv::u8string, CSignal>                m_mapRxSignals;                         ///< Signal object map.
    std::map<sdv::u8string, CSignal>                m_mapTxSignals;                         ///< Signal object map.
    sdv::EObjectStatus      m_eObjectStatus = sdv::EObjectStatus::initialization_pending;    ///< Object status.
    std::atomic_uint64_t                            m_uiTransactionCnt = 1ull;              ///< Transaction counter.
    std::mutex                                      m_mtxTransactions;                      ///< List with transactions access.
    std::list<CTransaction>                         m_lstTransactions;                      ///< List with transactions.
    uint64_t                                        m_uiDirectTransactionID;                ///< Current direct transaction ID.
    CScheduler                                      m_scheduler;                            ///< Scheduler for trigger execution.
    mutable std::mutex                              m_mtxTriggers;                          ///< Trigger object map protection.
    std::map<CTrigger*, std::unique_ptr<CTrigger>>  m_mapTriggers;                          ///< Trigger object map.
};

DEFINE_SDV_OBJECT(CDispatchService)

#endif // !defined DISPATCH_SERVICE_H