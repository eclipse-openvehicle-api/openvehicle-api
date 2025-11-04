#ifndef SOFTCAR_FMU_H
#define SOFTCAR_FMU_H

#ifdef WIN32
#include <Rpc.h>
#endif

#include "../global/dbcparser/dbcparser.h"
#include <fstream>
#include "codegen_base.h"

/**
 * @brief Signal definition structure
*/
struct signalDefinition
{
    std::string message_name;   ///< Message name
    std::string name;           ///< Signal name
    std::string attributes;     ///< Attributes
    uint32_t index = 0;         ///< Index
    bool isInput = false;       ///< Is input signal
    bool isDouble = false;      ///< for FMU we distinguish only between integer and double (float)
    uint32_t uiSize = 0;        ///< everything > 8 we declare as isDouble
    dbc::SSignalDef::EValueType signalType = dbc::SSignalDef::EValueType::signed_integer; ///< Signal type
};

/**
 * @brief CAN data link generator class.
*/
class CSoftcarFMUGen : public CCodeGeneratorBase
{
public:
    /**
     * @brief Constructor
     * @param[in] rpathOutputDir Reference to the outpur directory.
     * @param[in] rparser Reference to the DBC parser containing the definitions.
     * @param[in] ssModelIdentifier Reference to the modelIdentifier of the FMU.
     * @param[in] rsVersion Reference to a string representing a version of the dbc file (could be empty).
     * @param[in] rvecNodes Reference to a list of nodes this data link object is representing. An empty list will implement
     * sending and receiving functions for all nodes.
     */
    CSoftcarFMUGen(const std::filesystem::path& rpathOutputDir, const dbc::CDbcParser& rparser, const std::string& ssModelIdentifier, const std::string& rsVersion, const std::vector<std::string>& rvecNodes);

private:
    /**
    * @brief Keyword map for keyword replacement in a string.
    */
    typedef std::map<std::string, std::string> CKeywordMap;

    /**
    * @brief Add keywords to the map.
    * @param[in] rsModelIdentifier Reference to the modelIdentifier of the FMU.
    * @param[in] rsVersion Reference to a string representing a version of the dbc file (could be empty).
    * @param[in] rvecNodes Reference to a list of nodes this data link object is representing. An empty list will implement
    * @param[in] rmapKeywords Map with keywords to replace.
    * @return Returns the string with replacements.
    */
    void UpdateKeywordMap(const std::string& rsModelIdentifier, const std::string& rsVersion, 
        const std::vector<std::string>& rvecNodes, CKeywordMap& rmapKeywords) const;

    /**
    * @brief Create files in the resources folder.
    * @param[in] rRootPath parent folder
    * @return Returns the string with replacements.
    */
    void CreateResourcesFiles(const std::filesystem::path& rRootPath) const;

    /**
    * @brief Create files in the source folder.
    * @param[in] rRootPath parent folder
    * @return Returns the string with replacements.
    */
    void CreateSourceFiles(const std::filesystem::path& rRootPath) const;

    /**
    * @brief Create files in the include folder.
    * @param[in] rRootPath parent folder
    * @return Returns the string with replacements.
    */
    void CreateIncludeFiles(const std::filesystem::path& rRootPath) const;

    /**
    * @brief Create files in the include folder.
    * @param[in] rRootPath parent folder
    * @param[in] rsModelIdentifier Reference to the modelIdentifier of the FMU.
    * @param[in] rmapKeywords Map with keywords to replace.
    * @return Returns the string with replacements.
    */
    void CreateFMUFiles(const std::filesystem::path& rRootPath, const std::string& rsModelIdentifier, CKeywordMap& rmapKeywords) const;

    /**
    * @brief Cleanup the directory except of the root cmake file
    * @param[in] rpathRootDirectory root folder.
    * @return Returns true if folder exists or could be created, otherwise false.
    */
    bool CleanUpDirectory(const std::filesystem::path& rpathRootDirectory) const;

    /**
    * @brief Create subfolder if not exists
    * @param[in] rpathRootDirectory root folder.
    * @param[in] rpathSubDirectory sub folder.
    * @return Returns true if folder exists or could be created, otherwise false.
    */
    bool CreateDirectories(const std::filesystem::path& rpathRootDirectory, const std::filesystem::path& rpathSubDirectory) const;

    /**
    * @brief Create content of the signal_identifier file
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string Code_AllSignalsIdentifierList(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Create single signal line of the signal_identifier file
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string Code_SignalIdentifierList(const signalDefinition& rSignal) const;

    /**
    * @brief Replace keywords in a string.
    * @param[in] rsStr Reference to the string containing the keywords.
    * @param[in] rmapKeywords Map with keywords to replace.
    * @param[in] cMarker Character to identify the keyword with (placed before and after the keyword; e.g. %keyword%).
    * @return Returns the string with replacements.
    */
    static std::string ReplaceKeywords(const std::string& rsStr, const CKeywordMap& rmapKeywords, char cMarker = '%');

    /**
    * @brief Composes the fmi build description xml
    * @param[in] rsModelIdentifier Reference to model identifier.* 
    * @return The code to insert.
    */
    std::string CodeBuildDescriptionXML(const std::string& rsModelIdentifier) const;

    /**
    * @brief Composes a dbc file version information string
    * @param[in] rsVersion Reference to a string representing a version of the dbc file.
    * @return The code to insert.
    */
    std::string CodeDBCFileVersion(const std::string& rsVersion) const;

    /**
    * @brief returns a basic model identifier if not available
    * @param[in] rsModelIdentifier Reference to a string representing a version of the dbc file.
    * @return The code to insert.
    */
    std::string CodeModelIdentifier(const std::string& rsModelIdentifier) const;

    /**
    * @brief Get List of variables for the fmi xml file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeFMIFile_VariableList(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get List of variables for the fmi xml file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeFMIFile_SignalVariableList(const signalDefinition& rSignal) const;

    /**
    * @brief Get List of variables for the value reference in the config.h file.
    * @param[in] rsMsg Reference to the DBC message to provide the code for.
    * @param[in] rIndex Reference to the index of the variable starting with 0.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeConfigH_Rx_ValueReference(const dbc::SMessageDef& rsMsg, uint32_t& rIndex,
        std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get List of variables for the value reference in the config.h file.
    * @param[in] rsMsg Reference to the DBC message to provide the code for.
    * @param[in] rIndex Reference to the index of the variable starting with 0.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeConfigH_Tx_ValueReference(const dbc::SMessageDef& rsMsg, uint32_t& rIndex,
        std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get List of variables for the value reference in the config.h file.
    * @param[in] rsSig Reference to the DBC signal to provide the information for.
    * @param[in] index Reference to the index of the variable starting with 0.
    * @param[in] rsMessageName Reference to the message name.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeConfigH_Rx_SignalForValueReference(const dbc::SSignalDef& rsSig, const uint32_t index,
        const std::string& rsMessageName, std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get List of variables for the value reference in the config.h file.
    * @param[in] rsSig Reference to the DBC signal to provide the information for.
    * @param[in] index Reference to the index of the variable starting with 0.
    * @param[in] rsMessageName Reference to the message name.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeConfigH_Tx_SignalForValueReference(const dbc::SSignalDef& rsSig, const uint32_t index,
        const std::string& rsMessageName, std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get List of variables for the model data in the config.h file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeConfigH_ModelDataList(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get List of variables for the model data  in the config.h file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeConfigH_SignalForModelData(const signalDefinition& rSignal) const;

    /**
    * @brief Get List of variables for the mapping trigger file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeVariableList(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get List of variables for the mapping trigger file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeVariableName(const signalDefinition& rSignal) const;

    /**
    * @brief Get List of global signals definitions for the model.cpp file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GlobalDefinitionList(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get global signal definitions for the model.cpp file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GlobalSignalDefinition(const signalDefinition& rSignal) const;

    /**
    * @brief Get List of registering the global signals for the model.cpp file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GlobalRegisterList(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Check list of registered global signals for the model.cpp file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GlobalRegisterCheckList(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get registering the a global signal for the model.cpp file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GlobalRegisterSignal(const signalDefinition& rSignal) const;

    /**
    * @brief Get check of a global signal for the model.cpp file.
    * @param[in] rSignal Reference to the signal definition.
    * @param[in] first should be true for the first call, otherwise false.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GlobalRegisterSignalCheck(const signalDefinition& rSignal, bool first) const;

    /**
    * @brief Get List of resetting the global signals for the model.cpp file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GlobalResetList(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get resetting the a global signal for the model.cpp file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GlobalResetSignal(const signalDefinition& rSignal) const;

    /**
    * @brief Get List of signals for the update event for the model.cpp file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeModelCPP_EventUpdateList(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get Signal for writing int the update event method for the model.cpp file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeModelCPP_EventUpdateSignalDefinitionWrite(const signalDefinition& rSignal) const;

    /**
    * @brief Get Signal for reading the update event method for the model.cpp file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeModelCPP_EventUpdateSignalDefinitionRead(const signalDefinition& rSignal) const;

    /**
    * @brief Get List of signals for the GetFloat64 method for the model.cpp file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GetFloat64(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get List of signals for the GetInt32 method for the model.cpp file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GetInt32(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get List of signals for the SetFloat64 method for the model.cpp file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeModelCPP_SetFloat64(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get List of signals for the SetInt32 method for the model.cpp file.
    * @param[in] rvecSignalDefinitions Reference to the container for all signal definitions.
    * @return The code to insert.
    */
    std::string CodeModelCPP_SetInt32(const std::vector<signalDefinition>& rvecSignalDefinitions) const;

    /**
    * @brief Get Signal for GetFloat64 method for the model.cpp file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GetFloat64SignalDefinition(const signalDefinition& rSignal) const;

    /**
    * @brief Get Signal for GetInt32 method for the model.cpp file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeModelCPP_GetInt32SignalDefinition(const signalDefinition& rSignal) const;

    /**
    * @brief Get Signal for SetFloat64 method for the model.cpp file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeModelCPP_SetFloat64SignalDefinition(const signalDefinition& rSignal) const;

    /**
    * @brief Get Signal for SetInt32 method for the model.cpp file.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeModelCPP_SetInt32SignalDefinition(const signalDefinition& rSignal) const;

    /**
    * @brief Get comment for the OpenAPILoad function for the model.cpp file.
    * @return The code to insert.
    */
    std::string CodeModelCPP_OpenAPILoadFunction() const;

    /**
    * @brief Get default value of a tx signal. Used as start value un the build description.
    * @param[in] rSignal Reference to the signal definition.
    * @return The code to insert.
    */
    std::string CodeGetDefaultValueOfTxSignal(const dbc::SSignalDef& rSignal) const;

    /**
    * @brief Get minimum and maximum of a signal
    * @param[in] rSignal Reference to the signal definition.
    * @param[in] isDouble Reference to bool variable. Signal is either integer or double.
    * @return The code to insert.
    */
    std::string GetAttributes(const dbc::SSignalDef& rSignal, const bool& isDouble) const;

    /**
    * @brief returns guid string
    * @return The guid.
    */
    std::string newUUID() const;

    const dbc::CDbcParser& m_rparser;  ///< Reference to the DBC parser.
};

#endif // !defined SOFTCAR_FMU_H
