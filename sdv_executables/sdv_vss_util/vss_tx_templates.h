/**
 * @brief file template for the TX signals (device header)
 */
const char szTXVehicleDeviceHeaderTemplate[] = R"code(/**
 * @file %abstract_device_h%
 * @date %creation_date%
 * %version%
 */
#ifndef %vd_safeguard%
#define %vd_safeguard%

#include <iostream>
#include <support/component_impl.h>
#include <support/signal_support.h>
%tx_vd_includes_list%
#include "../signal_identifier.h"


/**
 * @brief Vehicle device %vss_shorten_no_dot%
 */
class CVehicleDevice%class_name%
	: public sdv::CSdvObject
	, public sdv::IObjectControl
%tx_vd_interface_list%{
public:

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(sdv::IObjectControl)
%tx_vd_interface_entry_list%	END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
	DECLARE_OBJECT_CLASS_NAME("%vss_original%_Device")

	/**
	 * @brief Constructor
	 */
	CVehicleDevice%class_name%();

	/**
	* @brief initialize device to get the object name or use the default.
	* @param[in] objectConfig Configuration containing the object name
	*/
	void Initialize(const sdv::u8string& objectConfig) override;

	/**
	* @brief Gets the current status of the object
	* @return EObjectStatus The current status of the object
	*/
	sdv::EObjectStatus GetStatus() const override;

	/**
	 * @brief Set the component operation mode. Overlovd of sdv::IObjectControl::SetOperationMode.
	 * @param[in] eMode The operation mode, the component should run in.
	 */
	void SetOperationMode(/*in*/ sdv::EOperationMode eMode) override;

	/**
	* @brief Shutdown function is to shutdown the execution of request thread
	*/
	void Shutdown() override;
	%tx_vd_function_list%
private:
	%tx_variable_list%
	std::atomic<sdv::EObjectStatus> m_status = { sdv::EObjectStatus::initialization_pending }; ///< To update the object status when it changes.
};

DEFINE_SDV_OBJECT(CVehicleDevice%class_name%)

#endif // !define %vd_safeguard%
)code";

/**
 * @brief file template for the TX signals (device cpp file)
 */
const char szTXVehicleDeviceClassTemplate[] = R"code(/**
 * @file %abstract_device_cpp%
 * @date %creation_date%
 * %version%
 */
#include <iostream>
#include "vd_%class_name_lowercase%.h"

/**
 * @brief Constructor
 */
CVehicleDevice%class_name%::CVehicleDevice%class_name%()
{
}

/**
* @brief initialize device to get the object name or use the default.
* @param[in] objectConfig Configuration containing the object name
*/
void CVehicleDevice%class_name%::Initialize(const sdv::u8string&)
{
    if (m_status != sdv::EObjectStatus::initialization_pending)
	{
	    return;
    }
	sdv::core::CDispatchService dispatch;
%tx_variable_init_list%
%tx_variable_check_list%
}


/**
* @brief Gets the current status of the object
* @return EObjectStatus The current status of the object
*/
sdv::EObjectStatus CVehicleDevice%class_name%::GetStatus() const
{
	return m_status;
}

/**
 * @brief Set the component operation mode. Overlovd of sdv::IObjectControl::SetOperationMode.
 * @param[in] eMode The operation mode, the component should run in.
 */
void CVehicleDevice%class_name%::SetOperationMode(/*in*/ sdv::EOperationMode eMode)
{
	switch (eMode)
	{
	case sdv::EOperationMode::configuring:
		if (m_status == sdv::EObjectStatus::running || m_status == sdv::EObjectStatus::initialized)
			m_status = sdv::EObjectStatus::configuring;
		break;
	case sdv::EOperationMode::running:
		if (m_status == sdv::EObjectStatus::configuring || m_status == sdv::EObjectStatus::initialized)
			m_status = sdv::EObjectStatus::running;
		break;
	default:
		break;
	}
}

/**
* @brief Shutdown function is to shutdown the execution of request thread
*/
void CVehicleDevice%class_name%::Shutdown()
{
%tx_reset_signals%
	m_status = sdv::EObjectStatus::destruction_pending;
}

%tx_function_implementations%
)code";

/**
 * @brief file template for the TX signals (service header)
 */
const char szTXBasicServiceHeaderTemplate[] = R"code(/**
 * @file %basic_service_h%
 * @date %creation_date%
 * %version%
 */
#ifndef %bs_safeguard%
#define %bs_safeguard%

#include <iostream>
#include <support/component_impl.h>
%tx_bs_includes_list%
/**
 * @brief basic service %class_name%
 */
class CBasicService%class_name%
	: public sdv::CSdvObject
%tx_bs_interface_list%{
public:

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(sdv::CSdvObject)
%tx_bs_interface_entry_list%	END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::BasicService)
	DECLARE_OBJECT_CLASS_NAME("%vss_original%_Service")

	/**
	* @brief Constructor
	*/
	CBasicService%class_name%();
	%tx_bs_function_list%
private:
	%tx_bs_variable_pointer_list%
};

DEFINE_SDV_OBJECT(CBasicService%class_name%)

#endif // !define %bs_safeguard%

)code";


/**
 * @brief file template for the TX signals (service cpp file)
 */
const char szTXBasicServiceClassTemplate[] = R"code(/**
 * @file %basic_service_cpp%
 * @date %creation_date%
 * %version%
 */
#include <iostream>
#include "bs_%class_name_lowercase%.h"
%vss_from_vd_not_found%
/**
 * @brief Constructor
 */
CBasicService%class_name%::CBasicService%class_name%()
{%tx_bs_variable_pointer_init_list%
	SDV_LOG_TRACE("CBasicService%class_name% created.");
}
%vd_tx_pointer_function_implementations%
)code";

/**
 * @brief file template for the TX signals (Vehicle Device IDL files)
 */
const char szVDTXIDLTemplate[] = R"code(/**
 * @file %_vd_idl_h%
 * @date %creation_date%
 * %version%
 */
#include <interfaces/core.idl>

%vss_device%
)code";

/**
 * @brief file template for the TX signals (Basic service IDL files)
 */
const char szBSTXIDLTemplate[] = R"code(/**
 * @file %_bs_idl_h%
 * @date %creation_date%
 * %version%
 */
#include <interfaces/core.idl>

%vss_service%
)code";
