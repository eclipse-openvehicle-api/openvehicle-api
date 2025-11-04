#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <support/interface_ptr.h>

// Forward declaration
class CDispatchService;
class CSignal;

/**
 * @brief Transaction administration
 * @details During creation of the transaction object, the transaction is of undefined type. The transaction takes the read or
 * write type only after a call to the read or write function. After this, the transaction doesn't change type any more and will
 * block calls to the functions provided for the other type (e.g. a read transaction doesn't allow a write and a write transaction
 * doesn't allow a read).
 * A read transaction is using the transaction ID to identify the current time of the transaction. Any data available before can
 * be read. Any data coming after will not be transmitted. In case there is no data any more (the transaction is too old) the
 * default value will be used.
 * A write transaction will collect the signal values. The distribution is deferred until the transaction is finalized. During
 * finalization, a new transaction ID is requested and the values are distributed using this ID (this is necessary so a read
 * transaction can decide whether the values are included in a read operation).
 * The written values are stored in a ringbuffer in the signal class. The latest position in the ring buffer contains the last
 * distributed transaction.
 */
class CTransaction : public sdv::IInterfaceAccess, public sdv::IObjectDestroy
{
public:
    /**
     * @brief Constructor
     * @param[in] rDispatchSvc Reference to the dispatch service.
     */
    explicit CTransaction(CDispatchService& rDispatchSvc);

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IInterfaceAccess)
        SDV_INTERFACE_ENTRY(sdv::IObjectDestroy)
    END_SDV_INTERFACE_MAP()

    /**
    * @brief Destroy the object. Overload of sdv::IObjectDestroy::DestroyObject.
    */
    virtual void DestroyObject() override;

    /**
     * @brief When called, enables the transaction as read-transaction. This would only happen when the transaction is still in
     * undefined state.
     * @return Returns the read-transaction ID or 0 when the transaction is not registered as read-transaction.
     */
    uint64_t GetReadTransactionID() const;

    /**
     * @brief When called, enables the transaction as write-transaction. This would only happen when the transaction is still in
     * undefined state. In that case, the value will be added to the deferred signal map. Any previous value will be overwritten.
     * @param[in] rSignal Reference to the signal class.
     * @param[in] ranyVal Reference to the value.
    */
    void DeferWrite(CSignal& rSignal, sdv::any_t& ranyVal);

    /**
     * @brief Finalizes the transaction. For read transactions, nothing happens. For write transactions, a transaction ID is stored
     * with the written signal value.
     */
    void FinalizeWrite();

    /**
     * @brief Set the iterator to this transaction (prevents having to search for the transaction in the transaction list).
     * @param[in] itTransaction The iterator to store.
     */
    void SetIterator(std::list<CTransaction>::iterator itTransaction);

    /**
     * @brief Get the stored iterator to the transaction.
     * @return The stored iterator.
     */
    std::list<CTransaction>::iterator GetIterator() const;

private:

    /**
     * @brief Transaction type
     */
    enum class ETransactionType
    {
        undefined = 0,          ///< Transaction type was not defined yet.
        read_transaction = 1,   ///< Read transaction (for TX signals).
        write_transaction = 2,  ///< Write transaction (for RX signals and publishers).
    };

    sdv::CLifetimeCookie                m_cookie = sdv::CreateLifetimeCookie();             ///< Lifetime cookie to manage the module lifetime.
    CDispatchService&                   m_rDispatchSvc;                                     ///< Reference to dispatch service.
    mutable ETransactionType            m_eTransactionType = ETransactionType::undefined;   ///< Transaction type.
    std::mutex                          m_mtxDeferredWriteSignalMap;                        ///< Deferred write signal map access.
    std::map<CSignal*, sdv::any_t>      m_mapDeferredWriteSignalMap;                        ///< Deferred write signal map.
    uint64_t                            m_uiReadTransactionID = 0ull;                       ///< Read transaction ID.
    std::list<CTransaction>::iterator m_itTransaction{};                                    ///< Iterator of the transaction in the transaction list.
};

#endif // !defined TRANSACTION_H