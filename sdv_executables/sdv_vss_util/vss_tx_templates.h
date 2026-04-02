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
 * @brief Platform abstraction %vss_shorten_no_dot%
 */
class CVehicleDevice%class_name%
	: public sdv::CSdvObject
%tx_vd_interface_list%{
public:

	BEGIN_SDV_INTERFACE_MAP()
%tx_vd_interface_entry_list%	END_SDV_INTERFACE_MAP()

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
	%tx_vd_function_list%
private:
	%tx_variable_list%
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

CVehicleDevice%class_name%::CVehicleDevice%class_name%()
{
}

bool CVehicleDevice%class_name%::OnInitialize()
{
	sdv::core::CDispatchService dispatch;
%tx_variable_init_list%
%tx_variable_check_list%
	return true;
}

void CVehicleDevice%class_name%::OnShutdown()
{
%tx_reset_signals%
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
%tx_bs_interface_entry_list%	END_SDV_INTERFACE_MAP()

	DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::actuator)
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
