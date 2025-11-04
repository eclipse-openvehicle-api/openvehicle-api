#ifndef VSS_BS_CODING_TX_H
#define VSS_BS_CODING_TX_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <interfaces/dispatch.h>

#include "codegen_base.h"
#include "vss_helper.h"
#include "vss_tx_templates.h"   
#include "vss_coding.h"

/**
 * @brief VSS code generator coding  class.
 * Beside the definition of the basic service interface also information about the vehicle device is necessary
 * These 2 definitions must match, there must be a corresponding 'signalVD' definition
 * the function container of the basic service signal must match the function container if the 'signalVD'
 * Creates the code out of templates
*/
class CVSSBSCodingTX : public CCodeGeneratorBase, CVSSHelper
{

public:
    /**
    * @brief create basic service content for the IDL file of a TX signal
    * @param[in] vssParts Interface in vss style separated in parts
    * @param[in] vecFunctions container of all functions of a single (basic service) vss definition (basic service)
    * @return content of the basic service part of a IDL Interface of a TX signal
    */
    std::string Code_BS_TXIDLList(const std::vector<std::string>& vssParts, const std::vector <SFunctionBSDefinition>& vecFunctions) const;

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
    * @param[in] vd_vssOriginalNoDot vss string without points of the corresponding vehicle device  
    * @param[in] vssOriginalNoDot vss string without points e.g. VehicleChassisSteeringWheelAngle 
    * @return content of a single interface
    */
    std::string Code_BS_TXIncludes(const std::string& vd_vssOriginalNoDot, const std::string& vssOriginalNoDot) const;

    /**
    * @brief create a single line containing a interface for basic service
    * @param[in] functionName function which is part of the interface
    * @param[in] vssWithColons vss string (complete version) with colons as separator
    * @return content of a single interface
    */
    std::string Code_BS_TXInterface(const std::string& functionName, const std::string& vssWithColons) const;

    /**
    * @brief create a single line containing a interface entry for basic service
    * @param[in] functionName function which is part of the interface
    * @param[in] vssWithColons vss string (complete version) with colons as separator
    * @return content of a single interface entry
    */
    std::string Code_BS_TXInterfaceEntry(const std::string& functionName, const std::string& vssWithColons) const;

    /**
    * @brief create list of interfaces (TX signals) for IDL file (Basic Service)
    * @param[in] spaces string containing only space characters for aligning
    * @param[in] function function definition structure
    * @return content of a single interface entry
    */
    std::string Code_BS_TXIDLInterface(const std::string& spaces, const SFunctionBSDefinition& function) const;

    /**
    * @brief create function definition part (TX signals) for basic service
    * @param[in] function function definition structure
    * @return content of a single function declaration
    */
    std::string Code_BS_TXFunction(const SFunctionBSDefinition& function) const;

    /**
    * @brief create definition of a interface pointer (TX signals) for basic service header file
    * @param[in] functionVD which is part of the interface (Vehicle Device)
    * @param[in] vd_vssWithColons vss string (complete version) with colons as separator (Vehicle Device)
    * @return content of a definition of a interface pointer
    */
    std::string Code_BS_TXVariablePointerFromVD(const SFunctionVDDefinition& functionVD, const std::string& vd_vssWithColons) const;

    /**
    * @brief create function pointer implementation part (TX signals) for basic service cpp file
    * @param[in] function function definition structure (Basic Service)
    * @param[in] functionVD function definition structure from vehicle device (Vehicle Device)
    * @param[in] className class name of the signal (Basic Service)
    * @return content of a single function pointer implementation
    */
    std::string Code_BS_TXFunctionPointerImplementationFromVD(const SFunctionBSDefinition& function, 
        const SFunctionVDDefinition& functionVD, const std::string& className) const;

    /**
    * @brief create check if a interface pointer is valid (TX signals) for IDL file
    * @param[in] functionVD which is part of the interface (Vehicle Device)
    * @param[in] className class name of the signal (Basic Service)
    * @param[in] vssOriginal original vvs string (Basic Service)
    * @param[in] vd_vssWithColons vss string (complete version) with colons as separator (Vehicle Device)
    * @return content of a check if a interface pointer is valid
    */
    std::string Code_BS_TXVariablePointerInitializationFromVD(const SFunctionVDDefinition& functionVD, const std::string& className, 
        const std::string& vssOriginal, const std::string& vd_vssWithColons) const;
};

#endif // !defined VSS_BS_CODING_TX_H

