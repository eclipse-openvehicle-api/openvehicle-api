#ifndef VSS_BS_CODING_RX_H
#define VSS_BS_CODING_RX_H

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
 * @brief VSS code generator coding  class.
 * Beside the definition of the basic service interface also information about the vehicle device is necessary
 * These 2 definitions must match, there must be a corresponding 'signalVD' definition
 * the function container of the basic service signal must match the function container if the 'signalVD'
 * Creates the code out of templates
*/
class CVSSBSCodingRX : public CCodeGeneratorBase, CVSSHelper
{

public:
    /**
    * @brief create service content for the IDL file of a RX signal
    * @param[in] vssParts Interface in vss style separated in parts
    * @param[in] vecFunctions container of all functions of a single vss definition
    * @return content of the Device part of a IDL Interface of a RX signal
    */
    std::string Code_RXIDLServiceList(const std::vector<std::string>& vssParts, const std::vector <SFunctionBSDefinition>& vecFunctions) const;

    /**
    * @brief fill the KeyWordMap with all required strings
    * @param[in] signal signal definition structure to fill the KeyWordMap
    * @param[in] signalVD signal definition structure from vehicle device
    * @param[in] mapKeywords KeyWordMap to be filled
    */
    void GetKeyWordMap(const SSignalBSDefinition& signal, const SSignalVDDefinition& signalVD, CKeywordMap& mapKeywords) const;

protected:
    /**
    * @brief create single or multiple lines containing include files
    * @param[in] vssOriginalNoDot vss string without points e.g. VehicleChassisSteeringWheelAngle
    * @return content of a single interface
    */
    std::string Code_BS_RXIncludes(const std::string& vssOriginalNoDot) const;

    /**
    * @brief create a single line containing a interface
    * @param[in] functionName function which is part of the interface
    * @param[in] vssWithColons vss string (complete version) with colons as separator
    * @param[in] vd_functionName function name (Vehicle Device)
    * @param[in] vd_vssWithColons vss string (complete version) with colons as separator (Vehicle Device)
    * @return content of a single interface
    */
    std::string Code_BS_RXInterface(const std::string& functionName, const std::string& vssWithColons,
        const std::string& vd_functionName, const std::string& vd_vssWithColons) const;

    /**
    * @brief create a single line containing a interface entry
    * @param[in] functionName function which is part of the interface
    * @param[in] vssWithColons vss string (short version) with colons as separator
    * @return content of a single interface entry
    */
    std::string Code_BS_RXInterfaceEntry(const std::string& functionName, const std::string& vssWithColons) const;

    /**
    * @brief Get the register and unregister part for the RX VD header
    * @param[in] vssWithColons vss string with colons as separator
    * @param[in] function function definition structure (Basic Service)
    * @param[in] functionVD function definition structure of vehicle device (Vehicle Device)
    * @return content of a single register and unregister part.
    */
    std::string Code_BS_RXReAndUnregisterEvent(const std::string& vssWithColons, const SFunctionBSDefinition& function, 
        const SFunctionVDDefinition& functionVD) const;

    /**
    * @brief Get the private header part of the basic service header
    * @param[in] function function definition structure
    * @param[in] vssWithColons vss string with colons as separator
    * @return content of private header part of the basic service  header
    */
    std::string Code_BS_RXPrivateHeaderPart(const SFunctionBSDefinition& function, const std::string& vssWithColons) const;

    /**
    * @brief create service content for the IDL file of a RX signal
    * @param[in] spaces string containing only space characters for aligning
    * @param[in] vssWithColons vss string (complete version) with colons as separator 
    * @param[in] function function definition structure
    * @return content of the Device part of a IDL Interface of a RX signal
    */
    std::string Code_RXIDLServiceInterface(const std::string& spaces, const std::string& vssWithColons, 
        const SFunctionBSDefinition& function) const;

    /**
    * @brief unregister signal Event
    * @param[in] signalVD signal definition structure to fill the KeyWordMap (Vehicle Device)
    * @param[in] vd_vssWithColons vss string (complete version) with colons as separator (Vehicle Device)
    * @param[in] functionVD function definition structure (Vehicle Device)
    * @param[in] function function definition structure (Basic Service)
    * @return content of a unregister code
    */
    std::string Code_BS_RXConstructor(const SSignalVDDefinition& signalVD, const std::string& vd_vssWithColons,
        const SFunctionVDDefinition& functionVD, const SFunctionBSDefinition& function) const;

    /**
    * @brief unregister signal Event
    * @param[in] signalVD signal definition structure to fill the KeyWordMap (Vehicle Device)
    * @param[in] vd_vssWithColons vss string (complete version) with colons as separator (Vehicle Device)
    * @param[in] functionVD function definition structure (Vehicle Device)
    * @return content of a unregister code
    */
    std::string Code_BS_RXDestructor(const SSignalVDDefinition& signalVD, const std::string& vd_vssWithColons, 
        const SFunctionVDDefinition& functionVD) const;

    /**
    * @brief set and get functions for basic service
    * @param[in] class_name class name which is part of the interface (Basic Service)
    * @param[in] function function definition structure (Basic Service)
    * @param[in] functionVD function definition structure (Vehicle Device)
    * @return content of a single set and get function code
    */
    std::string Code_BS_RXGetAndSetFunctions(const std::string& class_name, const SFunctionBSDefinition& function, 
        const SFunctionVDDefinition& functionVD) const;

    /**
    * @brief create register/unregister code for basic service
    * @param[in] class_name class name which is part of the interface
    * @param[in] function function definition structure
    * @param[in] vssWithColons vss string with colons as separator
    * @return content of a single signal register/unregister code
    */
    std::string Code_BS_RXRegister(const std::string& class_name, const SFunctionBSDefinition& function, 
        const std::string& vssWithColons) const;
};

#endif // !defined VSS_BS_CODING_RX_H

