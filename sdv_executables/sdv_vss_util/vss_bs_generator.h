#ifndef VSS_BS_GENERATOR_H
#define VSS_BS_GENERATOR_H

#include <fstream>
#include "codegen_base.h"
#include "vss_helper.h"

/**
 * @brief Basic Service generator.
 */
class CVSSBSGenerator : public CCodeGeneratorBase, CVSSHelper
{
public:
    /**
     * @brief Constructor
     * @param[in] signals Reference to a vector containing all signals (basic services)
     * @param[in] signalsVD Reference to a vector containing all signals (vehicle device)
     * @param[in] rpathOutputDir Reference to the outpur directory.
     * @param[in] rsPrefix used by cmake library and signal definition in signal_identifier.h file
     * @param[in] rsVersion optional version tag, will be wriiten in header of the files
     * @param[in] enableComponentCreation optional version tag, will be wriiten in header of the files 
     */

    CVSSBSGenerator(const std::vector<SSignalBSDefinition>& signals, const std::vector<SSignalVDDefinition>& signalsVD, const std::filesystem::path& rpathOutputDir, const std::string& rsPrefix, 
        const std::string& rsVersion, const bool enableComponentCreation) :
        m_enableComponentCreation(enableComponentCreation),
        m_ssPrefix(rsPrefix),
        m_ssVersion(rsVersion), 
        m_pathProject(rpathOutputDir), 
        m_signals(signals), 
        m_signalsVD(signalsVD)        
    {}


    /**
     * @brief Auto generate the Files
     * @return true on success, otherwise false
     */
    bool GeneratedCode();

private:

    /**
    * @brief create a file name including prefix and extension
    * @param[in] prefix
    * @param[in] filename
    * @param[in] extension
    * @return combined file name
    */
    std::string AppendExtension(const std::string& prefix, const std::string& filename, const std::string& extension) const;

    /**
    * @brief create vehicle device and basic service .h an .cpp files
    *        create IDL files
    * @param[in] ssVersion optional information will be placed in the header of the files
    */
    void CreateFiles(const std::string& ssVersion);

    /**
    * @brief create IDL files and vehicle device and basic service files of RX signals
    * @param[in] ssVersion optional information will be placed in the header of the files
    */
    void CreateRXFiles(const std::string& ssVersion) const;

    /**
    * @brief create basic service files of a single RX signal
    * @param[in] signal single signal definition
    * @param[in] ssVersion optional information will be placed in the header of the files
    */
    void CreateBasicServiceFilesForRXSignal(const SSignalBSDefinition& signal, const std::string& ssVersion) const;

    /**
    * @brief create IDL file of a single RX signal (basic service)
    * @param[in] signal single signal definition
    * @param[in] ssVersion optional information will be placed in the header of the files
    */
    void CreateIDLBasicServiceFileForRXSignal(const SSignalBSDefinition& signal, const std::string& ssVersion) const;

    /**
    * @brief create IDL files and vehicle device and basic service files of TX signals
    * @param[in] ssVersion optional information will be placed in the header of the files
    */
    void CreateTXFiles(const std::string& ssVersion) const;

    /**
    * @brief create basic service files of a single TX signal
    * @param[in] signal single signal definition
    * @param[in] ssVersion optional information will be placed in the header of the files
    */
    void CreateBasicServiceFilesForTXSignal(const SSignalBSDefinition& signal, const std::string& ssVersion) const;

    /**
    * @brief create IDL file for a single TX signal (basic service)
    * @param[in] signal single signal definition
    * @param[in] ssVersion optional information will be placed in the header of the files
    */
    void CreateIDLBasicServiceFileForTXSignal(const SSignalBSDefinition& signal, const std::string& ssVersion) const;

    /**
    * @brief find a vss definition in the signal list of the vehicle device collection and return the signal
    * @param[in] vssDefinition vss name
    * @return signal if a signal was found, otherwise a empty signal definition 
    */
    SSignalVDDefinition GetVDSignal(const std::string& vssDefinition) const
    {
        SSignalVDDefinition noSignal;
        for (const auto& signal : m_signalsVD)
        {
            if (signal.vssDefinition.compare(vssDefinition) == 0)
            {
                return signal;
            }
        }
        return noSignal;
    }

    bool m_enableComponentCreation;                ///< if set, the code for the components are created, otherwise only idl files 
    std::string m_ssPrefix;                        ///< prefix, used by cmake library and signal definition in signal_identifier.h file.
    std::string m_ssVersion;                       ///< optional version tag, will be wriiten in header of the files 
    std::filesystem::path  m_pathProject;          ///< Project file path
    std::vector <SSignalBSDefinition> m_signals;   ///< all signals
    std::vector <SSignalVDDefinition> m_signalsVD; ///< all signals
    std::vector <SSignalBSDefinition> m_TXsignals; ///< vector containing the RX signals
    std::vector <SSignalBSDefinition> m_RXsignals; ///< vector containing the TX signals
};

#endif // !defined VSS_BS_GENERATOR_H
