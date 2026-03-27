/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors:
 *   Thomas Pfleiderer - initial API and implementation
 ********************************************************************************/

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
%rx_vd_interface_list%{
public:

	BEGIN_SDV_INTERFACE_MAP()
%rx_vd_interface_entry_list%	END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::vehicle_bus)
	DECLARE_OBJECT_CLASS_NAME("%vss_original%_Device")

	/**
	 * @brief Constructor
	 */
	CVehicleDevice%class_name%();

    /**
     * @brief Initialization event, called after object configuration was loaded. Overload of sdv::CSdvObject::OnInitialize.
     * @return Returns 'true' when the initialization was successful, 'false' when not.
     */
    virtual bool OnInitialize() override;

    /**
     * @brief Shutdown the object. Overload of sdv::CSdvObject::OnShutdown.
     */
    virtual void OnShutdown() override;
%rx_vd_register_events_list%
private:
%rx_vd_private_vd_header_part%
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

bool CVehicleDevice%class_name%::OnInitialize()
{
	sdv::core::CDispatchService dispatch;
%rx_vd_subscribe_signals%
%rx_check_subscriptions%
	return true;
}

void CVehicleDevice%class_name%::OnShutdown()
{
%rx_reset_signals%
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

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::device)
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
