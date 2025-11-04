#ifndef INCLUDES_H
#define INCLUDES_H

#include "../../include/gtest_custom.h"
#include <support/sdv_core.h>
#include <support/app_control.h>
#include <support/pssup.h>
#include "../../../global/exec_dir_helper.h"
#include "../../../sdv_executables/sdv_control/startup_shutdown.h"

/**
 * @brief App control helper with automated server shutdown
 */
class CAppControlHelper : public sdv::app::CAppControl
{
public:
    /**
     * @brief Constructor
     * @param[in] bAutoStartServer When set, the constructor will start the server and the destructor will shutdown the server.
     * @param[in] uiInstanceID The instance ID to use or 0 for the default instance.
     */
    CAppControlHelper(bool bAutoStartServer = false, uint32_t uiInstanceID = 0u) : m_bAutoStartServer(bAutoStartServer),
        m_uiInstanceID(uiInstanceID)
    {
        // Initialize system
        std::string ssStartup = R"code(
[LogHandler]
ViewFilter = "Fatal"

[Application]
Mode = "Maintenance"
)code";
        if (uiInstanceID) ssStartup += "Instance = " + std::to_string(uiInstanceID) + "\n";
        Startup(ssStartup);

        // Automatically start the server
        if (bAutoStartServer)
        {
            // Startup the server
            int iRet = StartupSDVServer(CreateContext("STARTUP"));
            m_bServerStarted = iRet == 0 ? true : false;
            EXPECT_EQ(iRet, 0);
        }
    }

    /**
     * @brief Destructor
     */
    ~CAppControlHelper()
    {
        // Automatically shut down the server
        if (m_bServerStarted)
        {
            // Shutdown the server
            ShutdownSDVServer(CreateContext("SHUTDOWN"));
        }

        // Wait shortly to allow the shutdown to take place.
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Terminate (in case the shutdown got stuck).
        TerminateServerProcess();

        // Shutdown
        Shutdown();
    }

    /**
     * @brief Terminate a process with the ID.
     */
    void TerminateServerProcess(sdv::process::TProcessID tServerProcessID = GetServerProcessID())
    {
        // Get the process control service and terminate the process
        sdv::process::IProcessControl* pProcessControl = sdv::core::GetObject<sdv::process::IProcessControl>("ProcessControlService");
        if (tServerProcessID && pProcessControl) pProcessControl->Terminate(tServerProcessID);
    }

    /**
     * @brief Returns whether the server was started automatically.
     * @return The server start status.
     */
    bool ServerAutoStarted() const
    {
        return m_bServerStarted;
    }

    /**
     * @brief Create a context structure
     * @param[in] rssCommands One or more commands separated by a space.
     * @return The context structure.
    */
    SContext CreateContext(const std::string& rssCommands) const
    {
        SContext sContext;
        sContext.bSilent = true;
        sContext.bServerSilent = true;
        sContext.pathInstallDir = "../tests/bin/" + std::to_string(m_uiInstanceID ? m_uiInstanceID : 1000u);
        if (m_uiInstanceID)
            sContext.uiInstanceID = m_uiInstanceID;
        size_t nPos = 0;
        while (true)
        {
            size_t nSeparator = rssCommands.find_first_of(" \f\n\r\t\v", nPos);
            sContext.seqCmdLine.push_back(rssCommands.substr(nPos, nSeparator - nPos));
            if (nSeparator == std::string::npos) break;
            nPos = nSeparator + 1;
        }
        return sContext;
    }

private:
    bool        m_bAutoStartServer = false;         ///< When set, the constructor will start the server and the destructor will
                                                    ///< shutdown the server.
    bool        m_bServerStarted = false;           ///< When set, the server was started successfully by this class.
    uint32_t    m_uiInstanceID = 0;                 ///< The instance ID to use for the connection.
};

/**
 * @brief Interpret a table returned by the list command.
 * @param[in] rssTable Reference to the string holding the table.
 * @return Vector with rows containing a vector each with cells.
 */
inline std::vector<std::vector<std::string>> InterpretTable(const std::string& rssTable)
{
    // Read a cell; call until string is empty. Newline detected is set when the last cell on the line or in the table has been
    // read.
    size_t nPos = 0;
    auto fnReadCell = [&nPos, rssTable](bool& rbStartNewLine) -> std::string
    {
        // Skip whitespace except newline
        nPos = rssTable.find_first_not_of(" \f\r\t\v", nPos);

        // Check for a newline
        if (nPos != std::string::npos && rssTable[nPos] == '\n')
        {
            rbStartNewLine = true;
            nPos++;

            // Skip a header bar
            if (rbStartNewLine && nPos != rssTable.size() && rssTable[nPos] == '-')
            {
                nPos = rssTable.find_first_of("\n", nPos);
                if (nPos != std::string::npos) nPos++;
            }
        }

        // Skip whitespace
        nPos = rssTable.find_first_not_of(" \f\n\r\t\v", nPos);
        if (nPos > rssTable.size()) return {};

        // Find text
        std::string ssCell;
        size_t nStart = nPos;
        size_t nStop = 0;
        while (true)
        {
            // Find the next whitespace
            nPos = rssTable.find_first_of(" \f\n\r\t\v", nPos);
            nStop = nPos;
            if (nPos == std::string::npos)
                break;

            // If there at the most one space to the next word, this is considered to belong to the text
            if (rssTable[nPos] != ' ') break;
            nPos++;
            if (nPos == rssTable.size()) break;
            if (std::isspace(rssTable[nPos])) break;
        }

        // Return the cel text
        return rssTable.substr(nStart, nStop - nStart);
    };

    // Run through the table and add rows with cells.
    bool bStartNewRow = true;
    std::vector<std::vector<std::string>> vecTable;
    std::string ssCell;
    while (!(ssCell = fnReadCell(bStartNewRow)).empty())
    {
        // Start a new row?
        if (bStartNewRow)
            vecTable.resize(vecTable.size() + 1);
        bStartNewRow = false;

        // Get a reference to the last row
        if (vecTable.empty()) break; // Should not happen!
        std::vector<std::string>& rvecRow = vecTable.back();

        // Insert the cell
        rvecRow.push_back(std::move(ssCell));
    }

    // Return the table
    return vecTable;
}


#endif // !defined INCLUDES_H