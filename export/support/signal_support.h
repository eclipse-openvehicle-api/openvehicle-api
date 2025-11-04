/**
 *
 * @file      signal_support.h
 * @brief     This file provides base-implementations and helpers for signals and signal handling.
 * @version   0.1
 * @date      2022.11.14
 * @author    Thomas.pfleiderer@zf.com
 * @copyright Copyright ZF Friedrichshaven AG (c) 2022
 *
 */
#ifndef SIGNAL_SUPPORT_H
#define SIGNAL_SUPPORT_H

#include <functional>
#include <mutex>
#include <vector>

#include "../interfaces/dispatch.h"
#include "local_service_access.h"

/**
 * @brief Software Defined Vehicle framework.
 */
namespace sdv
{
    /**
     * @brief Core features.
     */
    namespace core
    {
        // Forward declaration
        class CSignal;
        class CTrigger;
        class CTransaction;

        /**
         * @brief Dispatch service convenience class.
         */
        class CDispatchService
        {
        public:
            /**
             * @brief Constructor. The constructor will automatically try to connect to the Data Dispatch Service.
             */
            CDispatchService();

            /**
             * @brief Destructor
             */
            ~CDispatchService();

            /**
             * @brief Register a signal for sending over the network; reading from the dispatch service. Data is provided by the
             * signal publisher and dependable on the requested behavior stored until it is sent.
             * @param[in] rssSignalName Reference to the name of the signal. To guarantee uniqueness, it is preferred to add the group
             * hierarchy to the signal name separated by a dot. E.g. with CAN: MAB.BcmChas1Fr03.SteerReCtrlReqAgReq
             * @param[in] tDefVal The default value of the signal.
             * @return Returns the initialized signal class or an empty signal when the signal already existed or the dispatch
             * service could not be reached.
             */
            template <typename TType>
            CSignal RegisterTxSignal(const u8string& rssSignalName, TType tDefVal);

            /**
             * @brief Register a signal for reception over the network; providing to the dispatch service.
             * @param[in] rssSignalName Name of the signal. To guarantee uniqueness, it is preferred to add the group hierarchy to
             * the signal name separated by a dot. E.g. with CAN: MAB.BcmChas1Fr03.SteerReCtrlReqAgReq
             * @return Returns the initialized signal class or an empty signal when the signal already existed or the dispatch
             * service could not be reached.
             */
            CSignal RegisterRxSignal(const u8string& rssSignalName);

            /**
             * @brief Add a publisher of signal data.
             * @param[in] rssSignalName Reference to the name of the signal to publish data for.
             * @return Returns the initialized signal class or an empty signal when the signal was nor registered before or the
             * dispatch service could not be reached.
             */
            CSignal AddPublisher(const u8string& rssSignalName);

            /**
             * @brief Subscribe to a signal event.
             * @param[in] rssSignalName Reference to the name of the signal to publish data for.
             * @param[in] func Function to call when data is received.
             * @return Returns the initialized signal class or an empty signal when the signal was nor registered before or the
             * dispatch service could not be reached.
             */
            CSignal Subscribe(const u8string& rssSignalName, std::function<void(any_t)> func);

            /**
             * @brief Subscribe to a signal event and allow updating the signal value automatically.
             * @tparam TType Type of the signal data.
             * @param[in] rssSignalName Reference to the name of the signal to publish data for.
             * @param[in] rtVal Reference to the value to be filled automatically.
             * @return Returns the initialized signal class or an empty signal when the signal was nor registered before or the
             * dispatch service could not be reached.
             */
            template <typename TType>
            CSignal Subscribe(const u8string& rssSignalName, std::atomic<TType>& rtVal);

            /**
             * @brief Get a list of registered signals.
             * @return List of registration functions.
             */
            sequence<SSignalRegistration> GetRegisteredSignals() const;

            /**
             * @brief Create a transaction.
             * @return Returns the transaction object. Returns an empty transaction object when the limit of transactions has been
             * exhausted.
             */
            CTransaction CreateTransaction();

            /**
             * @brief Finish a transaction.
             * @param[in] rTransaction Reference to the transaction to finish.
            */
            void FinishTransaction(CTransaction& rTransaction);

            /**
             * @brief Create a trigger object for the TX interface.
             * @param[in] fnExecute Callback function that is triggered.
             * @param[in] bSpontaneous When set, will be triggered on signal changes.
             * @param[in] uiDelayTime The minimal time between two triggers.
             * @param[in] uiPeriod When not 0, triggers periodically (time in ms).
             * @param[in] bOnlyWhenActive When set, periodic trigger will only occur once if the signal value equals the default
             * value.
             * @return If successful, returns the initialized trigger object or an empty trigger object when not successful.
             */
            CTrigger CreateTxTrigger(std::function<void()> fnExecute, bool bSpontaneous = true, uint32_t uiDelayTime = 0,
                uint32_t uiPeriod = 0ul, bool bOnlyWhenActive = false);
        };

        /**
         * @brief Transaction wrapping class
         */
        class CTransaction
        {
            /// Dispatch service friend class.
            friend CDispatchService;

        public:
            /**
            * @brief Default constructor
            */
            CTransaction() = default;

        protected:
            /**
            * @brief Constructor for transaction.
            * @param[in] rDispatch Reference to the dispatch class.
            * @param[in] pTransaction Pointer to the interface of the transaction object.
            */
            CTransaction(CDispatchService& rDispatch, IInterfaceAccess* pTransaction) :
                m_pDispatch(&rDispatch), m_pTransaction(pTransaction)
            {}

        public:
            /**
            * @brief Copy constructor (deleted)
            * @param[in] rTransaction Reference to the transaction to copy from.
            */
            CTransaction(const CTransaction& rTransaction) = delete;

            /**
            * @brief Move constructor
            * @param[in] rTransaction Reference to the transaction to move from.
            */
            CTransaction(CTransaction&& rTransaction) :
                m_pDispatch(rTransaction.m_pDispatch), m_pTransaction(rTransaction.m_pTransaction)
            {
                rTransaction.m_pDispatch = nullptr;
                rTransaction.m_pTransaction = nullptr;
            }

            /**
            * @brief Destructor
            */
            ~CTransaction()
            {
                Finish();
            }

            /**
            * @brief Assignment operator (deleted)
            * @param[in] rTransaction Reference to the transaction to copy from.
            * @return Reference to this class.
            */
            CTransaction& operator=(const CTransaction& rTransaction) = delete;

            /**
            * @brief Move operator
            * @param[in] rTransaction Reference to the transaction to move from.
            * @return Reference to this class.
            */
            CTransaction& operator=(CTransaction&& rTransaction)
            {
                Finish();
                m_pDispatch = rTransaction.m_pDispatch;
                m_pTransaction = rTransaction.m_pTransaction;
                rTransaction.m_pDispatch = nullptr;
                rTransaction.m_pTransaction = nullptr;
                return *this;
            }

            /**
            * @brief Valid transaction?
            */
            operator bool() const { return m_pTransaction ? true : false; }

            /**
            * @brief Finish the transaction.
            */
            void Finish()
            {
                if (m_pDispatch && m_pTransaction)
                {
                    IObjectDestroy* pDestroy = m_pTransaction->GetInterface<IObjectDestroy>();
                    pDestroy->DestroyObject();
                }

                m_pDispatch = nullptr;
                m_pTransaction = nullptr;
            }

            /**
             * @brief Get the transaction interface.
             * @return The transaction interface that was used for this transaction or nullptr when the transaction was started yet
             * or had finished before.
             */
            IInterfaceAccess* GetTransaction() const
            {
                return m_pTransaction;
            }

        private:
            CDispatchService*       m_pDispatch = nullptr;          ///< Pointer to the dispatch class.
            IInterfaceAccess*       m_pTransaction = nullptr;       ///< Transaction object
        };

        /**
         * @brief Signal class wrapping the signal access functions.
        */
        class CSignal
        {
            friend CDispatchService;

        public:
            /**
            * @brief Default constructor
            */
            CSignal() = default;

        protected:
            /**
             * @brief Constructor for signal interface received by a call to RegisterTxSignal, RegisterRxSignal or
             * RequestSignalPublisher.
             * @param[in] rDispatch Reference to the dispatch class.
             * @param[in] rssName Reference to the string holding the name of the signal.
             * @param[in] pSignal The signal interface access allowing access to the ISignalWrite interface.
             * @param[in] bRegistering Boolean indicating that the signal was created using a registration function rather than an
             * access function.
             */
            CSignal(CDispatchService& rDispatch, const u8string& rssName, IInterfaceAccess* pSignal, bool bRegistering) :
                m_pDispatch(&rDispatch), m_ssName(rssName), m_pSignal(pSignal), m_bRegistering(bRegistering)
            {
                if (pSignal)
                {
                    m_pSignalWrite = pSignal->GetInterface<ISignalWrite>();
                    m_pSignalRead = pSignal->GetInterface<ISignalRead>();
                }
            }

            /**
             * @brief Constructor for signal receiving callback.
             * @param[in] rDispatch Reference to the dispatch class.
             * @param[in] rssName Reference to the string holding the name of the signal.
             * @param[in] func Callback function being called on receiving data.
             */
            CSignal(CDispatchService& rDispatch, const u8string& rssName, std::function<void(any_t)> func) :
                m_pDispatch(&rDispatch), m_ssName(rssName),
                m_ptrSubscriptionHandler(std::make_unique<CReceiveEventHandler>(rDispatch, func))
            {}

            /**
             * @brief Constructor for signal receiving callback.
             * @tparam TType Type of the variable to update automatically on an event call.
             * @param[in] rDispatch Reference to the dispatch class.
             * @param[in] rssName Reference to the string holding the name of the signal.
             * @param[in] rtVal Reference to the variable to update automatically on an event call.
             */
            template <typename TType>
            CSignal(CDispatchService& rDispatch, const u8string& rssName, std::atomic<TType>& rtVal) :
                m_pDispatch(&rDispatch), m_ssName(rssName),
                m_ptrSubscriptionHandler(std::make_unique<CReceiveEventHandler>(rDispatch, rtVal))
            {}

            /**
             * @brief Get the subscription event handler, if existing.
             * @return Interface to the subscription handler or NULL when no handler is available.
             */
            IInterfaceAccess* GetSubscriptionEventHandler()
            {
                return m_ptrSubscriptionHandler.get();
            }

            /**
             * @brief Assign subscription object returned by the dispatch service.
             * @param[in] pSubscription The interface to the subscription object.
             */
            void Assign(IInterfaceAccess* pSubscription)
            {
                // The subscription object is managed through the signal int
                if (m_ptrSubscriptionHandler && !m_pSignal) m_pSignal = pSubscription;
            }

        public:
            /**
             * @brief Copy constructor (not available).
             * @param[in] rSignal Reference to the signal class to copy from.
             */
            CSignal(const CSignal& rSignal) = delete;

            /**
             * @brief Move constructor.
             * @param[in] rSignal Reference to the signal class to move from.
            */
            CSignal(CSignal&& rSignal) : m_pDispatch(rSignal.m_pDispatch), m_ssName(std::move(rSignal.m_ssName)),
                m_pSignal(rSignal.m_pSignal), m_pSignalWrite(rSignal.m_pSignalWrite), m_pSignalRead(rSignal.m_pSignalRead),
                m_ptrSubscriptionHandler(std::move(rSignal.m_ptrSubscriptionHandler)), m_bRegistering(rSignal.m_bRegistering)
            {
                rSignal.m_pSignal = nullptr;
                rSignal.m_pSignalWrite = nullptr;
                rSignal.m_pSignalRead = nullptr;
                rSignal.m_bRegistering = false;
            }

            /**
             * @brief Assignment operator (not available).
             * @param[in] rSignal Reference to the signal class to copy from.
             * @return Reference to this class.
            */
            CSignal& operator=(const CSignal& rSignal) = delete;

            /**
             * @brief Move operator
             * @param[in] rSignal Reference to the signal class to move from.
             * @return Reference to this class.
             */
            CSignal& operator=(CSignal&& rSignal)
            {
                Reset();
                m_pDispatch = rSignal.m_pDispatch;
                m_ssName = std::move(rSignal.m_ssName);
                m_pSignal = rSignal.m_pSignal;
                m_pSignalWrite = rSignal.m_pSignalWrite;
                m_pSignalRead = rSignal.m_pSignalRead;
                m_ptrSubscriptionHandler = std::move(rSignal.m_ptrSubscriptionHandler);
                m_bRegistering = rSignal.m_bRegistering;
                rSignal.m_pSignal = nullptr;
                rSignal.m_pSignalWrite = nullptr;
                rSignal.m_pSignalRead = nullptr;
                rSignal.m_bRegistering = false;
                return *this;
            }

            /**
             * Destructor
             */
            ~CSignal()
            {
                Reset();
            }

            /**
             * @brief Reset the signal
            */
            void Reset()
            {
                // First destroy the signal object. This will also prevent events to arrive.
                if (m_pSignal)
                {
                    IObjectDestroy* pDestroy = m_pSignal->GetInterface<IObjectDestroy>();
                    if (pDestroy) pDestroy->DestroyObject();
                }
                m_ptrSubscriptionHandler.reset();
                m_bRegistering = false;
                m_ssName.clear();
                m_pSignal = nullptr;
                m_pSignalWrite = nullptr;
                m_pSignalRead = nullptr;
                m_pDispatch = nullptr;
            }

            /**
             * @brief Operator testing for validity of this class.
             */
            operator bool() const
            {
                return m_pSignal ? true : false;
            }

            /**
             * @brief Update the signal value. This function is available for Rx signals (receiving data from the network) and for
             * services of publishing signals.
             * @param[in] tVal The value to update the signal with.
             * @param[in] rTransaction Reference to the transaction to use for reading.
             */
            template <typename TType>
            void Write(TType tVal, const CTransaction& rTransaction = CTransaction())
            {
                if (m_pSignalWrite) m_pSignalWrite->Write(any_t(tVal), rTransaction.GetTransaction());
            }

            /**
             * @brief Read the signal value. This function is available for Tx signals (sending data over the network).
             * @param[in] rTransaction Reference to the transaction to use for reading.
             * @return The signal value or empty when no value or interface is available.
             */
            any_t Read(const CTransaction& rTransaction = CTransaction()) const
            {
                any_t anyVal;
                if (m_pSignalRead) anyVal = m_pSignalRead->Read(rTransaction.GetTransaction());
                return anyVal;
            }

            /**
             * @brief Was this signal class used for registration of the signal.
             * @return Set when this signal is used for registration.
             */
            bool UsedForRegistration() const { return m_bRegistering; }

            /**
             * @brief Get the name of the signal.
             * @return String containing the name of the string.
            */
            u8string GetName() const { return m_ssName; }

        private:
            /**
             * @brief Receive event handler
             */
            class CReceiveEventHandler : public IInterfaceAccess, public ISignalReceiveEvent
            {
            public:
                /**
                * @brief Constructor for attaching a function.
                * @param[in] rDispatch Reference to the dispatch class.
                * @param[in] funcSignalReceive Callback function being called on receiving data.
                */
                CReceiveEventHandler(CDispatchService& rDispatch, std::function<void(any_t)> funcSignalReceive) :
                    m_rDispatch(rDispatch), m_funcSignalReceive(funcSignalReceive)
                {}

                /**
                * @brief Constructor for attaching a variable
                * @tparam TType Type of the variable to update automatically on an event call.
                * @param[in] rDispatch Reference to the dispatch class.
                * @param[in] rtVal Reference to the variable to update automatically on an event call.
                */
                template <typename TType>
                CReceiveEventHandler(CDispatchService& rDispatch, std::atomic<TType>& rtVal) :
                    m_rDispatch(rDispatch), m_ptrValue(std::make_unique<CValueAssignmentHelperT<TType>>(rtVal))
                {}

                /**
                 * @brief Destructor
                 */
                ~CReceiveEventHandler()
                {
                    if (m_pSubscription)
                    {
                        IObjectDestroy* pDestroy = m_pSubscription->GetInterface<IObjectDestroy>();
                        if (pDestroy) pDestroy->DestroyObject();
                    }
                }

                /**
                 * @brief Helper class for the value assignment.
                */
                class CValueAssignmentHelper : public ISignalReceiveEvent
                {
                public:
                    virtual ~CValueAssignmentHelper() = default;
                };

                /**
                 * @brief Type specific helper for the value assignment.
                 * @tparam TType The type of the value.
                */
                template <typename TType>
                class CValueAssignmentHelperT : public CValueAssignmentHelper
                {
                public:
                    /**
                     * @brief Constructor
                     * @param[in] rtVal Reference to the value to update at every receive event.
                    */
                    CValueAssignmentHelperT(std::atomic<TType>& rtVal) : m_rtVal(rtVal)
                    {}

                    /**
                     * Destructor
                     */
                    virtual ~CValueAssignmentHelperT() = default;

                    /**
                    * @brief A signal value was received. Overload function of ISignalReceiveEvent::Receive.
                    * @param[in] anyVal The signal value.
                    */
                    virtual void Receive(any_t anyVal) override
                    {
                        m_rtVal = static_cast<TType>(anyVal);
                    }

                private:
                    std::atomic<TType>&     m_rtVal;
                };

                /**
                 * @brief Set the subscriber cookie. This allows automatically subscription removal.
                 * @param[in] pSubscription Subscription interface.
                 */
                void Assign(IInterfaceAccess* pSubscription)
                {
                    m_pSubscription = pSubscription;
                }

            private:
                // Interface map
                BEGIN_SDV_INTERFACE_MAP()
                    SDV_INTERFACE_ENTRY(IInterfaceAccess)
                    SDV_INTERFACE_ENTRY(ISignalReceiveEvent)
                END_SDV_INTERFACE_MAP()

                /**
                 * @brief A signal value was received. Overload function of ISignalReceiveEvent::Receive.
                 * @param[in] anyVal The signal value.
                 */
                virtual void Receive(any_t anyVal) override
                {
                    if (m_funcSignalReceive) m_funcSignalReceive(anyVal);
                    if (m_ptrValue) m_ptrValue->Receive(anyVal);
                }

                CDispatchService&               m_rDispatch;                ///< Reference to the dispatch service.
                std::function<void(any_t)> m_funcSignalReceive;        ///< Receive signal data - callback function.
                IInterfaceAccess*               m_pSubscription = nullptr;  ///< Cookie received by adding an receive subscription.
                std::unique_ptr<CValueAssignmentHelper>     m_ptrValue;     ///< Value to update instead of a callback function.
            };

            /**
             * @brief Receive event handler smart pointer.
             */
            using CReceiveEventHandlerPtr = std::unique_ptr<CReceiveEventHandler>;

            CDispatchService*                   m_pDispatch = nullptr;          ///< Pointer to the dispatch service.
            u8string                       m_ssName;                       ///< Signal name.
            IInterfaceAccess*              m_pSignal = nullptr;            ///< The signal interface.
            ISignalWrite*            m_pSignalWrite = nullptr;       ///< Write signal data interface.
            ISignalRead*             m_pSignalRead = nullptr;        ///< Write signal data interface.
            CReceiveEventHandlerPtr             m_ptrSubscriptionHandler;       ///< Receive event handler smart pointer.
            bool                                m_bRegistering = false;         ///< When set, the signal was created using the
                                                                                ///< registration function.
        };

        /**
         * @brief Trigger class wrapping the trigger functionality.
         */
        class CTrigger
        {
            friend CDispatchService;
        public:
            /**
             * @brief Default constructor.
             */
            CTrigger() = default;

        protected:
            /**
            * @brief Constructor for trigger.
            * @param[in] rDispatch Reference to the dispatch class.
            * @param[in] fnExecute Function to the callback.
            */
            CTrigger(CDispatchService& rDispatch, std::function<void()> fnExecute) :
                m_pDispatch(&rDispatch), m_ptrCallback(std::make_unique<STriggerCallback>(fnExecute))
            {}

        public:
            /**
             * @brief Copy constructor (deleted)
             * @param[in] rTrigger Reference to the trigger to copy from.
             */
            CTrigger(const CTrigger& rTrigger) = delete;

            /**
             * @brief Move constructor
             * @param[in] rTrigger Reference to the trigger to move from.
             */
            CTrigger(CTrigger&& rTrigger) :
                m_pDispatch(rTrigger.m_pDispatch), m_pTrigger(rTrigger.m_pTrigger),
                m_ptrCallback(std::move(rTrigger.m_ptrCallback))
            {
                rTrigger.m_pDispatch = nullptr;
                rTrigger.m_pTrigger = nullptr;
            }

            /**
             * @brief Destructor
             */
            ~CTrigger()
            {
                Reset();
            }

            /**
             * @brief Assignment operator (deleted)
             * @param[in] rTrigger Reference to the trigger to copy from.
             * @return Reference to this class.
             */
            CTrigger& operator=(const CTrigger& rTrigger) = delete;

            /**
             * @brief Move operator
             * @param[in] rTrigger Reference to the trigger to move from.
             * @return Reference to this class.
             */
            CTrigger& operator=(CTrigger&& rTrigger)
            {
                Reset();
                m_pDispatch = rTrigger.m_pDispatch;
                m_pTrigger = rTrigger.m_pTrigger;
                m_ptrCallback = std::move(rTrigger.m_ptrCallback);
                rTrigger.m_pDispatch = nullptr;
                rTrigger.m_pTrigger = nullptr;
                return *this;
            }

            /**
             * @brief Valid trigger?
             */
            operator bool() const { return m_pTrigger ? true : false; }

            /**
             * @brief Reset the trigger.
             */
            void Reset()
            {
                if (m_pDispatch && m_pTrigger)
                {
                    IObjectDestroy* pDestroy = m_pTrigger->GetInterface<IObjectDestroy>();
                    pDestroy->DestroyObject();
                }

                m_pDispatch = nullptr;
                m_pTrigger = nullptr;
                m_ptrCallback.reset();
            }

            /**
             * @brief Add a signal that should trigger.
             * @param[in] rSignal Reference to the signal.
             */
            void AddSignal(const CSignal& rSignal)
            {
                if (!m_pTrigger) return;
                ITxTrigger* pTrigger = m_pTrigger->GetInterface<ITxTrigger>();
                if (!pTrigger) return;

                pTrigger->AddSignal(rSignal.GetName());
            }

            /**
             * @brief Remove a signal from the trigger.
             * @param[in] rSignal Reference to the signal.
             */
            void RemoveSignal(const CSignal& rSignal)
            {
                if (!m_pTrigger) return;
                ITxTrigger* pTrigger = m_pTrigger->GetInterface<ITxTrigger>();
                if (!pTrigger) return;

                pTrigger->RemoveSignal(rSignal.GetName());
            }

        protected:
            /**
             * @brief Get the pointer of the callback object.
             * @return Interface pointer to the callback.
             */
            IInterfaceAccess* GetCallback()
            {
                return m_ptrCallback.get();
            }

            /**
             * @brief Assign the trigger object.
             * @param[in] pTrigger Pointer to the interface to the trigger object.
             */
            void Assign(IInterfaceAccess* pTrigger)
            {
                m_pTrigger = pTrigger;
            }

        private:
            /**
             * @brief Trigger callback wrapper object.
             */
            struct STriggerCallback : public IInterfaceAccess, public ITxTriggerCallback
            {
                STriggerCallback(std::function<void()> fnExecute) : m_fnExecute(fnExecute)
                {}

            protected:
                // Interface map
                BEGIN_SDV_INTERFACE_MAP()
                    SDV_INTERFACE_ENTRY(ITxTriggerCallback)
                END_SDV_INTERFACE_MAP()

                /**
                 * @brief Execute the trigger. Overload of ITxTriggerCallback::Execute.
                 */
                virtual void Execute() override
                {
                    if (m_fnExecute) m_fnExecute();
                }

            private:
                std::function<void()>   m_fnExecute;                    ///< Execution callback function.
            };

            CDispatchService*                   m_pDispatch = nullptr;          ///< Pointer to the dispatch class.
            IInterfaceAccess*                   m_pTrigger = nullptr;           ///< Trigger object
            std::unique_ptr<STriggerCallback>   m_ptrCallback;                  ///< Callback object
        };

        inline CDispatchService::CDispatchService()
        {}

        inline CDispatchService::~CDispatchService()
        {}

        template <typename TType>
        inline CSignal CDispatchService::RegisterTxSignal(const u8string& rssName, TType tDefVal)
        {
            ISignalTransmission* pRegister = GetObject<ISignalTransmission>("DataDispatchService");
            CSignal signal;
            if (pRegister)
                signal = CSignal(*this, rssName,
                    pRegister->RegisterTxSignal(rssName, any_t(tDefVal)),
                    true);
            return signal;
        }

        inline CSignal CDispatchService::RegisterRxSignal(const u8string& rssName)
        {
            ISignalTransmission* pRegister = GetObject<ISignalTransmission>("DataDispatchService");
            CSignal signal;
            if (pRegister)
                signal = CSignal(*this, rssName, pRegister->RegisterRxSignal(rssName), true);
            return signal;
        }

        inline CSignal CDispatchService::AddPublisher(const u8string& rssSignalName)
        {
            ISignalAccess* pAccess = GetObject<ISignalAccess>("DataDispatchService");
            CSignal signal;
            if (pAccess)
                signal = CSignal(*this, rssSignalName, pAccess->RequestSignalPublisher(rssSignalName), false);
            return signal;
        }

        inline CSignal CDispatchService::Subscribe(const u8string& rssSignalName, std::function<void(any_t)> func)
        {
            ISignalAccess* pAccess = GetObject<ISignalAccess>("DataDispatchService");
            CSignal signal(*this, rssSignalName, func);
            if (pAccess)
                signal.Assign(pAccess->AddSignalSubscription(rssSignalName, signal.GetSubscriptionEventHandler()));
            return signal;
        }

        template <typename TType>
        inline CSignal CDispatchService::Subscribe(const u8string& rssSignalName, std::atomic<TType>& rtVal)
        {
            ISignalAccess* pAccess = GetObject<ISignalAccess>("DataDispatchService");
            CSignal signal(*this, rssSignalName, rtVal);
            if (pAccess)
                signal.Assign(pAccess->AddSignalSubscription(rssSignalName, signal.GetSubscriptionEventHandler()));
            return signal;
        }

        inline sequence<SSignalRegistration> CDispatchService::GetRegisteredSignals() const
        {
            ISignalAccess* pAccess = GetObject<ISignalAccess>("DataDispatchService");
            sequence<SSignalRegistration> seqSignalNames;
            if (pAccess) seqSignalNames = pAccess->GetRegisteredSignals();
            return seqSignalNames;
        }

        inline CTransaction CDispatchService::CreateTransaction()
        {
            IDispatchTransaction* pTransaction = GetObject<IDispatchTransaction>("DataDispatchService");
            CTransaction transaction;
            if (pTransaction) transaction = CTransaction(*this, pTransaction->CreateTransaction());
            return transaction;
        }

        inline void CDispatchService::FinishTransaction(CTransaction& rTransaction)
        {
            rTransaction.Finish();
        }

        inline CTrigger CDispatchService::CreateTxTrigger(std::function<void()> fnExecute, bool bSpontaneous /*= true*/,
            uint32_t uiDelayTime /*= 0*/, uint32_t uiPeriod /*= 0ul*/, bool bOnlyWhenActive /*= false*/)
        {
            if (!fnExecute) return CTrigger();
            if (!bSpontaneous && !uiPeriod) return CTrigger();
            ISignalTransmission* pSignalTransmission = GetObject<ISignalTransmission>("DataDispatchService");
            if (!pSignalTransmission) return CTrigger();
            uint32_t uiFlags = 0;
            if (bSpontaneous) uiFlags |= static_cast<uint32_t>(ISignalTransmission::ETxTriggerBehavior::spontaneous);
            if (uiPeriod && bOnlyWhenActive) uiFlags |= static_cast<uint32_t>(ISignalTransmission::ETxTriggerBehavior::periodic_if_active);
            CTrigger trigger(*this, fnExecute);
            trigger.Assign(pSignalTransmission->CreateTxTrigger(uiPeriod, uiDelayTime, uiFlags, trigger.GetCallback()));
            return trigger;
        }

    } // namespace core
} // namespace sdv


#endif // !defined SIGNAL_SUPPORT_H
