/**
 * @brief file template for the RX signals (device header)
 */
const char szRXVehicleDeviceHeaderTemplate[] = R"code(/**
 * @file %abstract_device_h%
 * @date %creation_date%
 * %version%
 */
#ifndef %vd_safeguard%
#define %vd_safeguard%

#include <iostream>
#include <set>
#include <support/component_impl.h>
#include <support/signal_support.h>
%rx_vd_includes_list%
#include "../signal_identifier.h"

/**
 * @brief Vehicle device %vss_original%
 */
class CVehicleDevice%class_name%
	: public sdv::CSdvObject
	, public sdv::IObjectControl
%rx_vd_interface_list%{
public:

	BEGIN_SDV_INTERFACE_MAP()
		SDV_INTERFACE_ENTRY(sdv::IObjectControl)
%rx_vd_interface_entry_list%	END_SDV_INTERFACE_MAP()

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
%rx_vd_register_events_list%
private:
%rx_vd_private_vd_header_part%
	std::atomic<sdv::EObjectStatus> m_status = { sdv::EObjectStatus::initialization_pending };  ///< To update the object status when it changes.
};

DEFINE_SDV_OBJECT(CVehicleDevice%class_name%)

#endif // !define %vd_safeguard%
)code";

/**
 * @brief file template for the RX signals (device cpp file)
 */
const char szRXVehicleDeviceClassTemplate[] = R"code(/**
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
%rx_vd_subscribe_signals%
%rx_check_subscriptions%
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
%rx_reset_signals%
	m_status = sdv::EObjectStatus::destruction_pending;
}
%rx_vd_register%
)code";

/**
 * @brief file template for the RX signals (service header)
 */
const char szRXBasicServiceHeaderTemplate[] = R"code(/**
 * @file %basic_service_h%
 * @date %creation_date%
 * %version%
 */
#ifndef %bs_safeguard%
#define %bs_safeguard%

#include <iostream>
#include <set>
#include <support/component_impl.h>
#include <support/signal_support.h>
%rx_bs_includes_list%
/**
 * @brief Basic Service %vss_original%
 */
class CBasicService%class_name%
	: public sdv::CSdvObject
%rx_bs_interface_list%{
public:

	BEGIN_SDV_INTERFACE_MAP()
%rx_bs_interface_entry_list%    END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::BasicService)
	DECLARE_OBJECT_CLASS_NAME("%vss_original%_Service")

	/**
	 * @brief Constructor
	 */
	CBasicService%class_name%();

	/**
	* @brief User-Defined Destructor
	*/
	~CBasicService%class_name%();
%rx_bs_register_events_list%
private:
%rx_bs_private_vd_header_part%
};

DEFINE_SDV_OBJECT(CBasicService%class_name%)

#endif // !define %bs_safeguard%
)code";

/**
 * @brief file template for the RX signals (service cpp file)
 */
const char szRXBasicServiceClassTemplate[] = R"code(/**
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
{%rx_bs_constructor%}

/**
* @brief User-Defined Destructor
*/
CBasicService%class_name%::~CBasicService%class_name%()
{%rx_bs_deconstructor%}
%rx_bs_getandsetfunctions%%rx_bs_register%
)code";

/**
 * @brief file template for the RX signals (Vehicle Device IDL files)
 */
const char szVDRXIDLTemplate[] = R"code(
/**
 * @file %_vd_idl_h%
 * @date %creation_date%
 * %version%
 */
#include "interfaces/core.idl"

%vss_device%
)code";

/**
 * @brief file template for the RX signals (Basic Service IDL files)
 */
const char szBSRXIDLTemplate[] = R"code(
/**
 * @file %_bs_idl_h%
 * @date %creation_date%
 * %version%
 */
#include "interfaces/core.idl"
#include "%vd_idl_file%"
%vss_device%
%vss_service%
)code";
