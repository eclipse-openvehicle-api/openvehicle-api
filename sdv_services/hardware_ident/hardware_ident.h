/**
* @file process_control.h
* @author Sudipta Babu Durjoy FRD DISS21 (mailto:sudipta.durjoy@zf.com)
* @brief
* @version 1.0
* @date 2023-10-23
*
* @copyright Copyright ZF Friedrichshafen AG (c) 2023
*
*/

#ifndef HARDWARE_IDENT_H
#define HARDWARE_IDENT_H

#include <interfaces/hw_ident.h>
#include <support/component_impl.h>

/**
* @brief Hardware ID class
*/
class CHardwareIdent : public sdv::CSdvObject, public sdv::hardware::IHardwareID
{
public:

    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::hardware::IHardwareID)
    END_SDV_INTERFACE_MAP()

    // Object declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::SystemObject)
    DECLARE_OBJECT_CLASS_NAME("HardwareIdentificationService")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Gets the hardware ID of the current hardware.
     * It's same for the all processes running in the same hardware and different for the processes of each different hardwares.
     * @return Return the hardware ID.
     */
    uint64_t GetHardwareID() override;
};
DEFINE_SDV_OBJECT(CHardwareIdent)

#endif // !define HARDWARE_IDENT_H
