#ifndef SIGNAL_H
#define SIGNAL_H

#include <support/interface_ptr.h>
#include <interfaces/dispatch.h>
#include "trigger.h"

// Forward declaration
class CDispatchService;
class CSignal;

/**
* @brief Class implementing the signal provider. Needed for provider interface implementation.
*/
class CProvider : public sdv::IInterfaceAccess, public sdv::IObjectDestroy, public sdv::core::ISignalWrite
{
public:
    /**
    * @brief Constructor
    * @param[in] rSignal Reference to the signal instance.
    */
    explicit CProvider(CSignal& rSignal);

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
        SDV_INTERFACE_ENTRY(sdv::core::ISignalWrite)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Destroy the object. Overload of sdv::IObjectDestroy::DestroyObject.
     */
    virtual void DestroyObject() override;

    /**
     * @brief Update the signal value. Overload of sdv::core::ISignalWrite::Write.
     * @param[in] anyVal The value to update the signal with.
     * @param[in] pTransaction The transaction interface. Could be NULL in case the update should occur immediately.
     */
    virtual void Write(/*in*/ sdv::any_t anyVal, /*in*/ sdv::IInterfaceAccess* pTransaction) override;

    /**
     * @brief Update the signal value with the transaction ID supplied. A new entry will be created if the transaction is
     * larger.
     * @param[in] ranyVal Reference to the value to update the signal with.
     * @param[in] uiTransactionID The transaction ID or 0 for current transaction ID.
     */
    void Write(const sdv::any_t& ranyVal, uint64_t uiTransactionID);

private:
    sdv::CLifetimeCookie    m_cookie = sdv::CreateLifetimeCookie(); ///< Lifetime cookie to manage the module lifetime.
    CSignal&                m_rSignal;                              ///< Reference to the signal class.
};

/**
* @brief Class implementing the signal consumer. Needed for consumer interface implementation.
*/
class CConsumer : public sdv::IInterfaceAccess, public sdv::IObjectDestroy, public sdv::core::ISignalRead
{
public:
    /**
    * @brief Constructor
    * @param[in] rSignal Reference to the signal instance.
    * @param[in] pEvent The event to be triggered on a signal change. Optional.
    */
    CConsumer(CSignal& rSignal, sdv::IInterfaceAccess* pEvent = nullptr);

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
        SDV_INTERFACE_ENTRY(sdv::core::ISignalRead)
    END_SDV_INTERFACE_MAP()

    /**
     * @brief Destroy the object. Overload of sdv::IObjectDestroy::DestroyObject.
     */
    virtual void DestroyObject() override;

    /**
    * @brief Get the signal value. Overload of sdv::core::ISignalRead::Read.
    * @param[in] pTransaction The transaction interface. Could be NULL in case the most up-to-date value is requested.
    * @return Returns the value.
    */
    virtual sdv::any_t Read(/*in*/ sdv::IInterfaceAccess* pTransaction) const override;

    /**
    * @brief Update the signal value with the transaction ID supplied. A new entry will be created if the transaction is
    * larger.
    * @param[in] ranyVal Reference to the value to update the signal with.
    * @param[in] uiTransactionID The transaction ID or 0 for current transaction ID.
    */
    void Write(const sdv::any_t& ranyVal, uint64_t uiTransactionID);

    /**
    * @brief Distribute a value to all consumers.
    * @param[in] ranyVal Reference of the value to consumers.
    */
    void Distribute(const sdv::any_t& ranyVal);

private:
    sdv::CLifetimeCookie    m_cookie = sdv::CreateLifetimeCookie(); ///< Lifetime cookie to manage the module lifetime.
    CSignal&                            m_rSignal;                  ///< Reference to the signal class
    sdv::core::ISignalReceiveEvent*     m_pEvent = nullptr;         ///< Receive event interface if available.
};

/**
 * @brief Signal administration
 */
class CSignal
{
public:
    /**
     * @brief Helper object to manage object lifetime.
     */
    struct SSignalObjectHelper
    {
        /**
         * @brief Default destructor (needs to be virtual to allow deletion of derived class).
         */
        virtual ~SSignalObjectHelper() = default;
    };

    /**
     * @brief Signal class constructor.
     * @param[in] rDispatchSvc Reference to the dispatch service.
     * @param[in] rssName Reference to the name string.
     * @param[in] anyDefVal Any containing the default value (for send-signals).
     * @param[in] eDirection Definition whether the signal is a send or receive signal.
    */
    CSignal(CDispatchService& rDispatchSvc, const sdv::u8string& rssName, sdv::core::ESignalDirection eDirection,
        sdv::any_t anyDefVal = sdv::any_t());

    /**
     * @brief Destructor
     */
    ~CSignal();

    /**
     * @brief Get the name of the signal.
     * @return String with the signal name.
     */
    sdv::u8string GetName() const;

    /**
     * @brief Get the signal direction.
     * @return The signal direction.
     */
    sdv::core::ESignalDirection GetDirection() const;

    /**
     * @brief Get the signal default value.
     * @return Any structure with default value.
     */
    sdv::any_t GetDefVal() const;

    /**
     * @brief Create a provider object.
     * @return Returns a pointer to the provider object or NULL when the signal is not configured to be a provider.
     */
    CProvider* CreateProvider();

    /**
     * @brief Remove a provider object.
     * @remarks If there are no other consumer/provider objects any more, the signal is unregistered from the dispatch service.
     * @param[in] pProvider Pointer to the provider to remove.
     */
    void RemoveProvider(CProvider* pProvider);

    /**
     * @brief Create a consumer object.
     * @param[in] pEvent The event to be triggered on a signal change. Optional.
     * @return Returns a pointer to the consumer object or NULL when the signal is not configured to be a consumer.
     */
    CConsumer* CreateConsumer(sdv::IInterfaceAccess* pEvent = nullptr);

    /**
     * @brief Remove a consumer object.
     * @remarks If there are no other consumer/provider objects any more, the signal is unregistered from the dispatch service.
     * @param[in] pConsumer Pointer to the consumer to remove.
     */
    void RemoveConsumer(CConsumer* pConsumer);

    /**
     * @brief Update the signal value with the transaction ID supplied. A new entry will be created if the transaction is larger.
     * @param[in] ranyVal Reference to the value to update the signal with.
     * @param[in] uiTransactionID The transaction ID or 0 for current transaction ID.
     * @param[in] rsetTriggers Set of triggers to execute on a spontaneous write.
     */
    void WriteFromProvider(const sdv::any_t& ranyVal, uint64_t uiTransactionID, std::set<CTrigger*>& rsetTriggers);

    /**
     * @brief Get the signal value.
     * @param[in] uiTransactionID The transaction ID or 0 for current transaction ID.
     * @return Returns the value.
     */
    sdv::any_t ReadFromConsumer(uint64_t uiTransactionID) const;

    /**
     * @brief Distribute a value to all consumers.
     * @param[in] ranyVal Reference of the value to distribute.
     */
    void DistributeToConsumers(const sdv::any_t& ranyVal);

    /**
     * @brief Add a trigger to the signal. This trigger will be returned when creating a trigger list.
     * @param[in] pTrigger Pointer to the trigger object.
     */
    void AddTrigger(CTrigger* pTrigger);

    /**
     * @brief Remove the trigger from the signal.
     * @param[in] pTrigger Pointer to the trigger object.
     */
    void RemoveTrigger(CTrigger* pTrigger);

    /**
     * @brief Returns whether the signal equals the default value.
     * @return Return the result of the comparison.
     */
    bool EqualsDefaultValue() const;

private:
    sdv::CLifetimeCookie            m_cookie = sdv::CreateLifetimeCookie();                 ///< Lifetime cookie to manage the module lifetime.
    CDispatchService&               m_rDispatchSvc;                                         ///< Reference to dispatch service.
    sdv::u8string                   m_ssName;                                               ///< Signal name
    sdv::core::ESignalDirection     m_eDirection = sdv::core::ESignalDirection::sigdir_tx;  ///< Signal direction
    sdv::any_t                      m_anyDefVal;                                            ///< Default value
    mutable std::mutex              m_mtxVal;                                               ///< Signal value protection
    mutable std::pair<uint64_t, sdv::any_t> m_rgprVal[16];                                  ///< The signal value
    mutable size_t                  m_nValIndex = 0;                                        ///< Most up-to-date-index
    mutable std::mutex              m_mtxSignalObjects;                                     ///< Signal object map protection.
    std::map<CProvider*, std::unique_ptr<CProvider>> m_mapProviders;                        ///< Map with signal objects.
    std::map<CConsumer*, std::unique_ptr<CConsumer>> m_mapConsumers;                        ///< Map with signal objects.
    std::mutex                      m_mtxTriggers;                                          ///< Protection for the trigger set.
    std::set<CTrigger*>             m_setTriggers;                                          ///< Trigger set for this signal.
};

#endif // !defined SIGNAL_H