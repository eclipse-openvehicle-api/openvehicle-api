#include "logger.h"

#ifdef _MSC_VER
#pragma push_macro("interface")
#undef interface
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#pragma execution_character_set( "utf-8" )
#ifdef GetClassInfo
#undef GetClassInfo
#endif
#pragma pop_macro("interface")
#endif

CLogControl g_log_control;

CLogControl::CLogControl()
{
#ifdef _MSC_VER
    SetConsoleOutputCP( 65001 );
#endif
}

void CLogControl::SetVerbosityMode(EVerbosityMode eMode)
{
    m_eVerbosityMode = eMode;
}

EVerbosityMode CLogControl::GetVerbosityMode() const
{
    return m_eVerbosityMode;
}

void CLogControl::IncreaseIndent()
{
    m_iIndent++;
}

void CLogControl::DecreaseIndent()
{
    m_iIndent--;
}

void CLogControl::Log(const std::string& rssText, bool bError) const
{
    // Reporting is determined by the verbosity mode.
    switch (m_eVerbosityMode)
    {
        case EVerbosityMode::report_none:
            // No reporting wanted
            return;
        case EVerbosityMode::report_errors:
            // Only report errors
            if (!bError) return;
            break;
        case EVerbosityMode::report_all:
        default:
            break;
    }

    // Create indentation string
    std::string ssIndent;
    for (int32_t iCnt = 0; !bError && iCnt < m_iIndent; iCnt++)
        ssIndent += "  ";

    // Replace all the newlines by the newline + indentation
    size_t nPos = 0;
    while (nPos < rssText.size() && nPos != std::string::npos)
    {
        // Find the next newline
        size_t nStart = nPos;
        nPos = rssText.find('\n');

        // Log indentation?
        if (m_bNewline) std::clog << ssIndent;
        m_bNewline = false;

        // Newline not found, log the text
        if (nPos == std::string::npos)
        {
            std::clog << rssText.substr(nStart);
            break;
        }

        // Include the newline
        nPos++;
        m_bNewline = true;

        // Log the text
        std::clog << rssText.substr(nStart, nPos - nStart);
    }
}

CLog::~CLog()
{
    if (!m_ssTask.empty())
        *this << "Leaving: " << m_ssTask << std::endl;
    g_log_control.DecreaseIndent();
}

int CLog::CLogStringBuf::sync()
{
    g_log_control.Log(str());
    str(std::string());
    return 0;
}
