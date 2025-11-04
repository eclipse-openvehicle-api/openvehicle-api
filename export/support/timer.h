#ifndef VAPI_TASK_TIMER_H
#define VAPI_TASK_TIMER_H

#include "../interfaces/timer.h"
#include "interface_ptr.h"
#include "local_service_access.h"
#include <functional>

namespace sdv
{
    namespace core
    {
        /**
         * @brief Task timer class.
         */
        class CTaskTimer
        {
        public:
            /**
             * @brief Dfault constructor
             */
            CTaskTimer() = default;

            /**
             * @brief Constructor
             * @param[in] uiPeriod The period to create a timer for.
             * @param[in] fnCallback Callback function to be called on task execution.
             */
            CTaskTimer(uint32_t uiPeriod, std::function<void()> fnCallback) :
                m_ptrCallback(std::make_unique<STimerCallback>(fnCallback))
            {
                if (!m_ptrCallback) return;
                if (!uiPeriod) return;

                // Get the task timer service.
                sdv::core::ITaskTimer* pTaskTimer = sdv::core::GetObject<sdv::core::ITaskTimer>("TaskTimerService");
                if (!pTaskTimer)
                {
                    pTaskTimer = sdv::core::GetObject<sdv::core::ITaskTimer>("SimulationTaskTimerService");
                    if (!pTaskTimer) return;
                }

                // Create the timer
                m_pTimer = pTaskTimer->CreateTimer(uiPeriod, m_ptrCallback.get());
            }

            /**
            * @brief Constructor
            * @param[in] uiPeriod The period to create a timer for.
            * @param[in] pTask Pointer to the interface of the task object to be called. The object must expose
            * sdv::core::ITastExecute.
            */
            CTaskTimer(uint32_t uiPeriod, sdv::IInterfaceAccess* pTask)
            {
                if (!uiPeriod) return;

                // Get the task timer service.
                sdv::core::ITaskTimer* pTaskTimer = sdv::core::GetObject<sdv::core::ITaskTimer>("TaskTimerService");
                if (!pTaskTimer)
                {
                    pTaskTimer = sdv::core::GetObject<sdv::core::ITaskTimer>("SimulationTaskTimerService");
                    if (!pTaskTimer) return;
                }

                // Create the timer
                m_pTimer = pTaskTimer->CreateTimer(uiPeriod, pTask);
            }

            /**
             * @brief Copy constructor is deleted.
             */
            CTaskTimer(const CTaskTimer&) = delete;

            /**
             * @brief Move constructor
             * @param[in] rtimer Reference to the timer to move from.
             */
            CTaskTimer(CTaskTimer&& rtimer) noexcept :
                m_pTimer(rtimer.m_pTimer), m_uiPeriod(rtimer.m_uiPeriod), m_ptrCallback(std::move(rtimer.m_ptrCallback))
            {
                rtimer.m_pTimer = nullptr;
                rtimer.m_uiPeriod = 0;
            }

            /**
             * @brief Destructor
             */
            ~CTaskTimer()
            {
                Reset();
            }

            /**
             * @brief Assignment operator is deleted.
             */
            CTaskTimer& operator=(const CTaskTimer&) = delete;

            /**
             * @brief Move operator
             * @param[in] rtimer Reference to the timer to move from.
             * @return Reference to this timer object.
            */
            CTaskTimer& operator=(CTaskTimer&& rtimer) noexcept
            {
                Reset();
                m_pTimer = rtimer.m_pTimer;
                m_ptrCallback = std::move(rtimer.m_ptrCallback);
                m_uiPeriod = rtimer.m_uiPeriod;
                rtimer.m_pTimer = nullptr;
                rtimer.m_uiPeriod = 0;
                return *this;
            }

            /**
             * @brief Returns whether the timer is valid.
             */
            operator bool() const
            {
                return m_pTimer;
            }

            /**
             * @brief Reset the timer. Releases the timer object.
             */
            void Reset()
            {
                if (m_pTimer)
                {
                    IObjectDestroy* pDestroy = m_pTimer->GetInterface<IObjectDestroy>();
                    if (pDestroy)
                        pDestroy->DestroyObject();
                    m_pTimer = nullptr;
                }
                m_ptrCallback.reset();
                m_uiPeriod = 0ul;
            }

            /**
             * @brief Get the task timer period.
             * @return The period of the timer in ms.
             */
            uint32_t GetPeriod() const
            {
                return m_uiPeriod;
            }

        private:
            /**
            * @brief Timer callback wrapper object.
            */
            struct STimerCallback : public IInterfaceAccess, public core::ITaskExecute
            {
                STimerCallback(std::function<void()> fnCallback) : m_fnCallback(fnCallback)
                {}

            protected:
                // Interface map
                BEGIN_SDV_INTERFACE_MAP()
                    SDV_INTERFACE_ENTRY(core::ITaskExecute)
                END_SDV_INTERFACE_MAP()

                /**
                * @brief Execute the trigger. Overload of ITxTriggerCallback::Execute.
                */
                virtual void Execute() override
                {
                    if (m_fnCallback) m_fnCallback();
                }

            private:
                std::function<void()>       m_fnCallback;           ///< Callback function
            };

            sdv::IInterfaceAccess*          m_pTimer = nullptr;     ///< Timer object
            uint32_t                        m_uiPeriod = 0ul;       ///< Task timer period
            std::unique_ptr<STimerCallback> m_ptrCallback;          ///< Callback object
        };
    }
}

#endif // !defined VAPI_TASK_TIMER_H