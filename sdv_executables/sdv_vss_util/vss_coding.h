#ifndef VSS_CODING_H
#define VSS_CODING_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <interfaces/dispatch.h>

#include "codegen_base.h"
#include "vss_helper.h"
   
/**
 * @brief VSS code generator coding  class.
 * Creates the code out of templates
*/
class CVSSCodingCommon : public CCodeGeneratorBase, CVSSHelper
{

public:
    /**
     * @brief Constructor
     * @param[in] rsPrefix used by cmake library and signal definition in signal_identifier.h file
     */
    CVSSCodingCommon(const std::string& rsPrefix) : m_ssPrefix(rsPrefix) {}

    /**
    * @brief fill the KeyWordMap with all required strings
    * @param[in] signal signal definition structure to fill the KeyWordMap
    * @param[in] mapKeywords KeyWordMap to be filled
    * @param[in] rsVersion optional information will be placed in the header of the files
    */
    void GetCommonKeyWordMap(const SSignalVDDefinition& signal, CKeywordMap& mapKeywords, const std::string& rsVersion) const;

    /**
    * @brief fill the KeyWordMap with all required strings
    * @param[in] signal signal definition structure to fill the KeyWordMap
    * @param[in] mapKeywords KeyWordMap to be filled
    * @param[in] rsVersion optional information will be placed in the header of the files
    */
    void GetCommonKeyWordMap(const SSignalBSDefinition& signal, CKeywordMap& mapKeywords, const std::string& rsVersion) const;

    /**
    * @brief create content of the signal_identifier.h file
    * @param[in] allSignals container of all signal definitions
    * @param[in] ssFileName file name of the signal_identifier.h file
    * @param[in] ssVersion optional information will be placed in the header of the files 
    * @return content of the signal_identifier.h file
    */
    std::string Code_SignalIdentifier(const std::vector <SSignalVDDefinition>& allSignals, const std::string& ssFileName, 
        const std::string& ssVersion) const;

    /**
    * @brief get the version line which will be added in the header of the files
    * @param[in] ssVersion optional information will be placed in the header of the files 
    * @return line for the header of the files to know the file is auto generated
    */
    std::string Code_VSSFileVersion(const std::string& ssVersion) const;

    /**
    * @brief generates the unique define to avoid duplicate loading of a header file
    * @param[in] ssFileName file name which will be included in the unique define
    * @return unique define string 
    */    
    std::string Code_SafeGuard(const std::string& ssFileName) const;

    /**
    * @brief generates the cmakelist.txt file
    * @param[in] targetLibName project cmake name
    * @param[in] targetComponentName is the name of the header and cpp file
    * @return content of the CMakeList.txt file
    */
    std::string Code_CMakeProject(const std::string& targetLibName, const std::string& targetComponentName) const;

protected:

    /**
    * @brief create a signal line for the signal_identifier file
    * @param[in] allSignals container of all signals for aligning of the string
    * @param[in] signal single signal the function belongs to
    * @param[in] func the function which should be returned in a line
    * @param[in] signalTitle signal title string in file signal_identifier.h
    * @param[in] canSignalTitle can signal title string in file signal_identifier.h
    * @param[in] cTypeTitle c-type title string in file signal_identifier.h
    * @return content of a single function for the signal_identifier file
    */
    std::string Code_SignalIdentifierList(const std::vector <SSignalVDDefinition>& allSignals, const SSignalVDDefinition& signal,
        const SFunctionVDDefinition& func, const std::string& signalTitle, const std::string& canSignalTitle, const std::string& cTypeTitle) const;

    /**
    * @brief create a list of all signals for the signal_identifier file
    * @param[in] allSignals container of all signals
    * @param[in] signalTitle signal title string in file signal_identifier.h
    * @param[in] canSignalTitle can signal title string in file signal_identifier.h
    * @param[in] cTypeTitle c-type title string in file signal_identifier.h
    * @return content of list of all signals for the signal_identifier file
    */
    std::string Code_AllSignalsIdentifierList(const std::vector <SSignalVDDefinition>& allSignals,
        const std::string& signalTitle, const std::string& canSignalTitle, const std::string& cTypeTitle) const;

    std::string m_ssPrefix; ///< prefix, used by cmake library and signal definition in signal_identifier.h file.
};

#endif // !defined VSS_CODING_H

