#include <interfaces/core.h>
#include <support/signal_support.h>

namespace vss
{
	namespace Device
	{
		/**
		 * @brief IReceptionSignalSpeed abstract device interface, example of receiving a value
		 */
		interface IReceptionSignalSpeed_Event
		{
			/** Interface ID. */
			static constexpr ::sdv::interface_id _id = 0xA012345678900400;

			/**
			 * @brief Set Speed value (for example of type int32_t)
			 * @param[in] value of current speed
			 */
			virtual void SetSpeedValue(/*in*/ int32_t value) = 0;
		};

		/**
		 * @brief IReceptionSignalSpeed abstract device interface, example of receiving a value
		 */
		interface IReceptionSignalSpeed
		{
			/** Interface ID. */
			static constexpr ::sdv::interface_id _id = 0xA012345678900500;

			/**
			 * @brief Register IReceptionSignalSpeed_Event on signal change
			 * Register all events and call them on signal change
			 * @param[in] event function
			 */
			virtual void RegisterSpeedEvent(/*in*/ IReceptionSignalSpeed_Event* event) = 0;

			/**
			 * @brief UnRegister IReceptionSignalSpeed_Event on signal change
			 * Register all events and call them on signal change
			 * @param[in] event function
			 */
			virtual void UnRegisterSpeedEvent(/*in*/ IReceptionSignalSpeed_Event* event) = 0;
		};
	}
}

namespace vss
{
	namespace Service
	{
		/**
		* @brief Vehicle speed service
		*/
		interface IReceptionSignalSpeed
		{
			/** Interface ID. */
			static constexpr ::sdv::interface_id _id = 0xA012345678900600;

			/**
			 * @brief Get Speed value (for example of type int32_t)
	         * @return Returns current speed
			 */
			virtual int32_t GetSpeedValue() = 0;

			/**
			* @brief Register Callback on signal change
			* @param[in] callback function
			*/
			virtual void RegisterCallBack(/*in*/ vss::Device::IReceptionSignalSpeed_Event* callback) = 0;

			/**
			* @brief Unregister Callback
			* @param[in] callback function
			*/
			virtual void UnregisterCallBack(/*in*/ vss::Device::IReceptionSignalSpeed_Event* callback) = 0;
		};
	}
}
