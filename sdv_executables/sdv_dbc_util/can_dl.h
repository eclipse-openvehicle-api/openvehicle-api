#ifndef CAN_DL_H
#define CAN_DL_H

#include "../global/dbcparser/dbcparser.h"
#include <fstream>
#include "codegen_base.h"

/**
 * @brief CAN data link generator class.
*/
class CCanDataLinkGen : public CCodeGeneratorBase
{
public:
    /**
     * @brief Constructor
     * @param[in] rpathOutputDir Reference to the outpur directory.
     * @param[in] rparser Reference to the DBC parser containing the definitions.
     * @param[in] rssVersion Reference to a string representing a version of the dbc file (could be empty).
     * @param[in] rssIfcName Reference to the string holding the interface name (could be empty).
     * @param[in] nIfcIdx The interface index (valid if no name is provided).
     * @param[in] rvecNodes Reference to a list of nodes this data link object is representing. An empty list will implement
     * sending and receiving functions for all nodes.
     */
    CCanDataLinkGen(const std::filesystem::path& rpathOutputDir, const dbc::CDbcParser& rparser, const std::string& rssVersion,
        const std::string& rssIfcName, size_t nIfcIdx, const std::vector<std::string>& rvecNodes);

private:
    /**
    * @brief Keyword map for keyword replacement in a string.
    */
    typedef std::map<std::string, std::string> CKeywordMap;

    /**
    * @brief Replace keywords in a string.
    * @param[in] rssStr Reference to the string containing the keywords.
    * @param[in] rmapKeywords Map with keywords to replace.
    * @param[in] cMarker Character to identify the keyword with (placed before and after the keyword; e.g. %keyword%).
    * @return Returns the string with replacements.
    */
    static std::string ReplaceKeywords(const std::string& rssStr, const CKeywordMap& rmapKeywords, char cMarker = '%');

    /**
    * @brief validates that the string represents a float
    * @param[in] rssInput Reference to the string representing a float.
    * @return Returns the input string. If '.' not found it adds ".0" to the string
    */
   std::string StringMustBeFloatValue(const std::string& rssInput);

    /**
     * @brief Composes a dbc file version information string
     * @param[in] rssVersion Reference to a string representing a version of the dbc file.
     * @return The code to insert.
     */
    std::string CodeDBCFileVersion(const std::string& rssVersion);

    /**
     * @brief Provide code to determine the interface index based on the interface name (if supplied).
     * @param[in] rssIfcName Reference to the string holding the interface name (could be empty).
     * @return The code to insert during initialization.
     */
    std::string CodeInitInterfaceIndex(const std::string& rssIfcName);

    /**
    * @brief Provide code for the RX message definition.
    * @param[in] rsMsg Reference to the DBC message to provide the message definition for.
    * @return The code to insert.
    */
    std::string CodeRxMessageDefinition(const dbc::SMessageDef& rsMsg);

    /**
    * @brief Provide code for the TX message definition.
    * @param[in] rsMsg Reference to the DBC message to provide the message definition for.
    * @return The code to insert.
    */
    std::string CodeTxMessageDefinition(const dbc::SMessageDef& rsMsg);

    /**
     * @brief Provide code for the signal declaration.
     * @param[in] rsSig Reference to the DBC signal to provide a declaration for.
     * @return The code to insert.
    */
    std::string CodeSignalDecl(const dbc::SSignalDef& rsSig);

    /**
     * @brief Initialize the RX message variable.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
     * @return The code to insert.
     */
    std::string CodeInitVarRxMessage(const dbc::SMessageDef& rsMsg);

    /**
     * @brief Initialize the TX message variable.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
     * @return The code to insert.
     */
    std::string CodeInitVarTxMessage(const dbc::SMessageDef& rsMsg);

    /**
     * @brief Initialize the RX message structure.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
     * @return The code to insert.
     */
    std::string CodeInitRxMessage(const dbc::SMessageDef& rsMsg);

    /**
    * @brief Initialize the TX message structure.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
    * @return The code to insert.
    */
    std::string CodeInitTxMessage(const dbc::SMessageDef& rsMsg);

    /**
    * @brief Terminate the RX message structure.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
    * @return The code to insert.
    */
    std::string CodeTermRxMessage(const dbc::SMessageDef& rsMsg);

    /**
    * @brief Terminate the TX message structure.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
    * @return The code to insert.
    */
    std::string CodeTermTxMessage(const dbc::SMessageDef& rsMsg);

    /**
     * @brief Provide the RX message structure functions.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
     * @return The code to insert.
     */
    std::string CodeRxMessageFunctions(const dbc::SMessageDef& rsMsg);

    /**
     * @brief Provide the TX message structure functions.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
     * @return The code to insert.
     */
    std::string CodeTxMessageFunctions(const dbc::SMessageDef& rsMsg);

    /**
     * @brief Initialize the signal registration.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
     * @param[in] rsSig Reference to the DBC signal to provide the registration for.
     * @return The code to insert.
     */
    std::string CodeRegisterRxSignal(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig);

    /**
    * @brief Initialize the signal registration.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
    * @param[in] rsSig Reference to the DBC signal to provide the registration for.
    * @return The code to insert.
    */
    std::string CodeRegisterTxSignal(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig);

    /**
    * @brief Remove the signal registration.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
     * @param[in] rsSig Reference to the DBC signal to provide the unregistration for.
    * @return The code to insert.
    */
    std::string CodeUnregisterSignal(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig);

    /**
     * @brief Process the signal data for the specific message.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
     * @param[in] rsSig Reference to the DBC signal to provide the processing for.
     * @return The code to insert.
     */
    std::string CodeProcessRxSignal(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig);

    /**
     * @brief Initialize the TX trigger with the signal..
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
     * @param[in] rsSig Reference to the DBC signal to provide the trigger initialization for.
     * @return The code to insert.
     */
    std::string InitTrigger(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig);

    /**
     * @brief Compose the message from the signal data.
     * @param[in] rsMsg Reference to the DBC message to provide the code for.
     * @param[in] rsSig Reference to the DBC signal to provide the composition for.
     * @return The code to insert.
     */
    std::string CodeComposeTxSignal(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig);

    std::filesystem::path       m_pathProject;      ///< Project file path
    std::filesystem::path       m_pathHeader;       ///< Header file path
    std::filesystem::path       m_pathCpp;          ///< C++ file path
    std::ofstream               m_fstreamHeader;    ///< The header file
    std::ofstream               m_fstreamCpp;       ///< The C++ file
    const dbc::CDbcParser&      m_rparser;          ///< Reference to the DBC parser.
};



#endif // !defined CAN_DL_H
