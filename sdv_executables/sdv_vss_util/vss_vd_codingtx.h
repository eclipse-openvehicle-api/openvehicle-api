#ifndef VSS_CODING_TX_H
#define VSS_CODING_TX_H

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
 * Creates the code out of templates
*/
class CVSSVDCodingTX : public CCodeGeneratorBase, CVSSHelper
{

public:
    /**
     * @brief Constructor
     * @param[in] rsPrefix used by cmake library and signal definition in signal_identifier.h file
     */
    CVSSVDCodingTX(const std::string& rsPrefix) : m_ssPrefix(rsPrefix) {}

    /**
    * @brief create vehicle device content for the IDL file of a TX signal
    * @param[in] vssParts Interface in vss style separated in parts
    * @param[in] vecFunctions container of all functions of a single vss definition
    * @return content of the vehicle device part of a IDL Interface of a TX signal
    */
    std::string Code_VD_TXIDLList(const std::vector<std::string>& vssParts, 
        const std::vector <SFunctionVDDefinition>& vecFunctions) const;

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
    std::string Code_VD_TXIncludes(const std::string& vssOriginalNoDot) const;

    /**
    * @brief create a single line containing a interface for vehicle device
    * @param[in] functionName function which is part of the interface
    * @param[in] vssWithColons vss string (complete version) with colons as separator
    * @return content of a single interface
    */
    std::string Code_VD_TXInterface(const std::string& functionName, const std::string& vssWithColons) const;

    /**
    * @brief create a single line containing a interface entry for vehicle device
    * @param[in] functionName function which is part of the interface
    * @param[in] vssWithColons vss string (complete version) with colons as separator
    * @return content of a single interface entry
    */
    std::string Code_VD_TXInterfaceEntry(const std::string& functionName, const std::string& vssWithColons) const;

    /**
    * @brief create list of interfaces (TX signals) for IDL file (Vehicle Device)
    * @param[in] spaces string containing only space characters for aligning
    * @param[in] function function definition structure
    * @return content of a single interface entry
    */
    std::string Code_VD_TXIDLInterface(const std::string& spaces, const SFunctionVDDefinition& function) const;

    /**
    * @brief create function definition part (TX signals) for vehicle device 
    * @param[in] function function definition structure
    * @return content of a single function declaration
    */
    std::string Code_VD_TXFunction(const SFunctionVDDefinition& function) const;

    /**
    * @brief Rest signal (TX signals) for vehicle device cpp file
    * @param[in] function function definition structure
    * @return content of a single signal reset implementation
    */
    std::string Code_VD_TXResetSignal(const SFunctionVDDefinition& function) const;

    /**
    * @brief create function implementation part (TX signals) for vehicle device cpp file
    * @param[in] function function definition structure
    * @param[in] className class name of the signal
    * @return content of a single function implementation
    */
    std::string Code_VD_TXFunctionImplementation(const SFunctionVDDefinition& function, const std::string& className) const;

    /**
    * @brief create single variable definition (TX signals) for vehicle device header
    * @param[in] signalName name of the signal
    * @return content of a single variable definition
    */
    std::string Code_VD_TXVariable(const std::string& signalName) const;

    /**
    * @brief create single variable initialization (TX signals) for vehicle device cpp file
    * @param[in] function which is part of the interface
    * @return content of a single variable initialization
    */
    std::string Code_VD_TXVariableInitialization(const SFunctionVDDefinition& function) const;

    /**
    * @brief create check if all interfaces are valid (TX signals) for vehicle device cpp file
    * @param[in] vecFunctions container of all functions of a single vss definition
    * @return content to check if all interfaces are valid
    */
    std::string Code_VD_TXVariableCheckInitialization(const std::vector <SFunctionVDDefinition>& vecFunctions) const;

    std::string m_ssPrefix; ///< prefix, used by cmake library and signal definition in signal_identifier.h file.
};

#endif // !defined VSS_CODING_TX_H

