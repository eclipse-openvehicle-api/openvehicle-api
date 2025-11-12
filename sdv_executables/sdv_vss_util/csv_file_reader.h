#ifndef CSV_FILE_READER_H
#define CSV_FILE_READER_H

#include <fstream>
#include <regex>
#include "vss_helper.h"


/**
 * @brief Reads the cvs file and parses the signal definitions.
*/
class CCSVFileReader : public CVSSHelper
{

public:
    /**
     * @brief Constructor
     * @param[in] ssFileName Reference to the cvs file which contains the VSS definition
     * @param[in] bSilent if true no console output on errors & warnings
     * @param[in] bVerbose if true validates names and print out warnings in case of code style is not matched
     */
    CCSVFileReader(const std::string& ssFileName, const bool bSilent, const bool bVerbose);

    /**
     * @brief Get signal definitions for vehicle devices
     * @return Returns a reference to the variable containing the list of signals;
     */
    const std::vector<SSignalVDDefinition>& GetVDSignals() const
    {
        return m_vdSignals;
    }

    /**
     * @brief Get signal definitions for basic services
     * @return Returns a reference to the variable containing the list of signals;
     */
    const std::vector<SSignalBSDefinition>& GetBSSignals() const
    {
        return m_bsSignals;
    }

private:

    /**
     * @brief Type definitions for the columns in the vehicle device sheets
     */
    typedef enum vssVDColumns
    {
        column_type,
        column_className,
        column_functionName,
        column_signalName,
        column_vssDefinition,
        column_direction,
        column_signalCType,
        column_canSignalName,
        column_vdDefinition = column_canSignalName,
        column_vdFormula,
        column_end
    } vssVDColumns;


    /**
     * @brief prints help to the console
     */
    void Help();

    /**
    * @brief read a csv excel file with ';' separator
    * @param[in] ssFileName  name of the cvs file to be read
    * @return True if signals definitions could be read and are valid, otherwise false;
    */
    bool ReadCSVVSSFile(const std::string& ssFileName);

    /**
    * @brief read and interprets a signal line of a cvs file
    * @param[in] ssLine content of a complete line
    * @param[in] index line number, may be used for default values
    * @param[in] endColumn the maximum of columns
    */
    void ReadLine(const std::string& ssLine, const uint32_t index, size_t endColumn);

    /**
    * @brief If a signal definition is found in the list for the vehicle devices add the function/signal to this definition
    * @param[in] signals collection of vehicle device signals
    * @param[in] vssDefinition definition to look for
    * @param[in] direction signal direction, either rx or tx
    * @param[in] parts Reference to the list of parts of a single line of the cvs file
    * @param[in] index line number
    * @return True if signal was found, otherwise false
    */
    bool AddToExistingVDSignal(std::vector <SSignalVDDefinition>& signals, const std::string& vssDefinition, 
        const sdv::core::ESignalDirection& direction, const std::vector<std::string>& parts, const uint32_t index);

    /**
    * @brief If a signal definition is found in the list for the basic services add the function/signal to this definition
    * @param[in] signals collection of basic service signals
    * @param[in] vssDefinition definition to look for
    * @param[in] direction signal direction, either rx or tx
    * @param[in] parts Reference to the list of parts of a single line of the cvs file
    * @param[in] index line number
    * @return True if signal was found, otherwise false
    */
    bool AddToExistingBSSignal(std::vector <SSignalBSDefinition>& signals, const std::string& vssDefinition,
        const sdv::core::ESignalDirection& direction, const std::vector<std::string>& parts, const uint32_t index);

    /**
    * @brief parse the parts of a line of the vehicle device sheet to get the signal definitions
    * @param[in] parts Reference to the list of parts of a single line of the cvs file
    * @param[in] index line number
    */
    void ParseColumns(const std::vector<std::string>& parts, const uint32_t index);

    /**
    * @brief add function definition to the container of an existing signal (vehicle device)
    * @param[in] signal the function definition should be added
    * @param[in] functionName functione name
    * @param[in] signalName signal name
    * @param[in] canSignalName  can signal name
    * @param[in] idlType idl type
    * @param[in] formula code to convert signal value in vehicle device
    * @param[in] index line number
    * @return True if function definition was added, otherwise false
    */
    bool AddFunctionVDDefinition(SSignalVDDefinition& signal, const std::string& functionName, const std::string& signalName, 
        const std::string& canSignalName, const std::string& idlType, const std::string& formula, const uint32_t index);

    /**
    * @brief add function definition to the container of an existing signal (basic service)
    * @param[in] signal the function definition should be added
    * @param[in] functionName functione name
    * @param[in] signalName signal name
    * @param[in] canSignalName  can signal name
    * @param[in] idlType idl type
    * @param[in] index line number
    * @return True if function definition was added, otherwise false
    */
    bool AddFunctionBSDefinition(SSignalBSDefinition& signal, const std::string& functionName,
        const std::string& signalName, const std::string& canSignalName, const std::string& idlType, const uint32_t index);

    std::vector <SSignalVDDefinition> m_vdSignals;   ///< all signals for vehicle devices 
    std::vector <SSignalBSDefinition> m_bsSignals; ///< all signals for basic services

    bool m_verbose = false;
    bool m_silent = false;
};

#endif // !defined CSV_FILE_READER_H
