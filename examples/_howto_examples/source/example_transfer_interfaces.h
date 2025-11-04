#include <interfaces/core.h>
#include <support/signal_support.h>

namespace vss
{
	namespace Device
	{
		/**
		* @brief ITransferSignalBrakeForce interface, example of transferring a value
		*/
		interface ITransferSignalBrakeForce
		{
			/** Interface ID. */
			static constexpr ::sdv::interface_id _id = 0xA012345678900800;

			/**
			* @brief Set brake force value
			* @param[in] value brake force
			* @return true on success otherwise false
			*/
			virtual bool SetBrakeForce(/*in*/ uint32_t value) = 0;
		};
	}
}

namespace vss
{
	namespace Service
	{
		/**
		* @brief ITransferSignalBrakeForce interface, example oftransferringg a value
		*/
		interface ITransferSignalBrakeForce
		{
			/** Interface ID. */
			static constexpr ::sdv::interface_id _id = 0xA012345678900900;

			/**
			* @brief Set brake force value
			* @param[in] value brake force
			* @return true on success otherwise false
			*/
			virtual bool SetBrakeForce(/*in*/ uint32_t value) = 0;
		};
	}
}
