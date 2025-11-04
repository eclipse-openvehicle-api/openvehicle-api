#ifndef VSS_VD_CODING_RX_H
#define VSS_VD_CODING_RX_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <interfaces/dispatch.h>

#include "codegen_base.h"
#include "vss_helper.h"
#include "vss_rx_templates.h"
#include "vss_coding.h"

   
/**
 * @brief VSS code generator coding class for vehicle devices.
 * Creates the code out of templates
*/
class CVSSVDCodingRX :  public CCodeGeneratorBase, CVSSHelper
{

public:
    /**
     * @brief Constructor
     * @param[in] rsPrefix used by cmake library and signal definition in signal_identifier.h file
     */
    CVSSVDCodingRX(const std::string& rsPrefix) : m_ssPrefix(rsPrefix) {}

    /**
    * @brief create device content for the IDL file of a RX signal
    * @param[in] vssParts Interface in vss style separated in parts
    * @param[in] vecFunctions container of all functions of a single vss definition
    * @return content of the Device part of a IDL Interface of a RX signal
    */
    std::string Code_RXIDLDeviceList(const std::vector<std::string>& vssParts, const std::vector <SFunctionVDDefinition>& vecFunctions) const;

    /**
    * @brief create service content for the IDL file of a RX signal
    * @param[in] vssParts Interface in vss style separated in parts
    * @param[in] vecFunctions container of all functions of a single vss definition
    * @return content of the Device part of a IDL Interface of a RX signal
    */
    std::string Code_RXIDLServiceList(const std::vector<std::string>& vssParts, const std::vector <SFunctionVDDefinition>& vecFunctions) const;

    /**
    * @brief fill the KeyWordMap with all required strings
    * @param[in] signal signal definition structure to fill the KeyWordMap
    * @param[in] mapKeywords KeyWordMap to be filled
    */
    void GetKeyWordMap(const SSignalVDDefinition& signal, CKeywordMap& mapKeywords) const;

protected:
    /**
    * @brief create single or multiple lines containing include files
    * @param[in] vssOriginalNoDot vss string without points e.g. VehicleChassisSteeringWheelAngle
    * @return content of a single interface
    */
    std::string Code_VD_RXIncludes(const std::string& vssOriginalNoDot) const;

    /**
    * @brief create a single line containing a interface
    * @param[in] functionName function which is part of the interface
    * @param[in] vssShortenWithColons vss string (short version) with colons as separator
    * @return content of a single interface
    */
    std::string Code_VD_RXInterface(const std::string& functionName, const std::string& vssShortenWithColons) const;

    /**
    * @brief create a single line containing a interface entry
    * @param[in] functionName function which is part of the interface
    * @param[in] vssWithColons vss string (complete version) with colons as separator
    * @return content of a single interface entry
    */
    std::string Code_VD_RXInterfaceEntry(const std::string& functionName, const std::string& vssWithColons) const;

    /**
    * @brief Get the register and unregister part for the RX VD header
    * @param[in] vssWithColons vss string (complete version) with colons as separator
    * @param[in] function function definition structure
    * @return content of a single register and unregister part.
    */
    std::string Code_VD_RXReAndUnregisterEvent(const std::string& vssWithColons, const SFunctionVDDefinition& function) const;

    /**
    * @brief Get the private header part of the vehicle device header
    * @param[in] function function definition structure
    * @param[in] vssWithColons vss string (complete version) with colons as separator
    * @param[in] class_name class name which is part of the interface
    * @return content of private header part of the vehicle device header
    */
    std::string Code_VD_RXPrivateHeaderPart(const SFunctionVDDefinition& function, const std::string& vssWithColons, 
        const std::string& class_name) const;

    /**
    * @brief create device content for the IDL file of a RX signal
    * @param[in] spaces string containing only space characters for aligning
    * @param[in] function function definition structure
    * @return content of the Device part of a IDL Interface of a RX signal
    */
    std::string Code_RXIDLDeviceInterface(const std::string& spaces, const SFunctionVDDefinition& function) const;

    /**
    * @brief create service content for the IDL file of a RX signal
    * @param[in] spaces string containing only space characters for aligning
    * @param[in] vssWithColons vss string (complete version) with colons as separator 
    * @param[in] function function definition structure
    * @return content of the Device part of a IDL Interface of a RX signal
    */
    std::string Code_RXIDLServiceInterface(const std::string& spaces, const std::string& vssWithColons, 
        const SFunctionVDDefinition& function) const;

    /**
    * @brief subscribe signals (RX type) for vehicle device
    * @param[in] class_name class name which is part of the interface
    * @param[in] function function definition structure
    * @return content of a initializing code
    */
    std::string Code_VD_RXSubscribeSignal(const std::string& class_name, const SFunctionVDDefinition& function) const;

    /**
    * @brief subscribe signals (RX type) for vehicle device
    * @param[in] vecFunctions list of functions of the vss interface
    * @return content of a initializing code
    */
    std::string Code_VD_RXCheckSignalSubscribtions(const std::vector <SFunctionVDDefinition>& vecFunctions) const;

    /**
    * @brief Rest signal (RX signals) for vehicle device cpp file
    * @param[in] function function definition structure
    * @return content of a single signal reset implementation
    */
    std::string Code_VD_RXResetSignal(const SFunctionVDDefinition& function) const;

    /**
    * @brief create register/unregister and callback function code for vehicle device
    * @param[in] class_name class name which is part of the interface
    * @param[in] function function definition structure
    * @param[in] vssWithColons vss string with colons as separator
    * @return content of a single signal register/unregister and callback code
    */
    std::string Code_VD_RXRegister(const std::string& class_name, const SFunctionVDDefinition& function, 
        const std::string& vssWithColons) const;

    std::string m_ssPrefix; ///< prefix, used by cmake library and signal definition in signal_identifier.h file.
};

#endif // !defined VSS_VD_CODING_RX_H

