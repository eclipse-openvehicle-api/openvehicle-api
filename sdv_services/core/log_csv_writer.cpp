#include "log_csv_writer.h"
#include <sstream>

CLogCSVWriter::CLogCSVWriter(std::shared_ptr<std::ostream> ostream, const char separator /*= ';'*/)
    : m_streamOutput(std::move(ostream))
    , m_cSeparator(separator)
{
    if ((m_cSeparator <= 0x08) || (m_cSeparator >= 0x0A && m_cSeparator <= 0x1F)
        || (m_cSeparator >= 0x30 && m_cSeparator <= 0x39) || (m_cSeparator >= 0x41 && m_cSeparator <= 0x5A)
        || (m_cSeparator >= 0x61 && m_cSeparator <= 0x7A) || static_cast<uint8_t>(m_cSeparator) >= 0x7F
        || m_cSeparator == '\"' || m_cSeparator == '.' || m_cSeparator == ':' || m_cSeparator == ' ')
    {
        throw SForbiddenSeparatorException((std::string("Forbidden Separator '") + m_cSeparator + "'").c_str());
    }

    std::string intro = std::string("sep=") + m_cSeparator + "\nTimestamp" + m_cSeparator + "Severity" + m_cSeparator
        + "Tag" + m_cSeparator + "Message\n";

    std::unique_lock<std::mutex> lock(m_mtxOutput);
    *m_streamOutput << intro;
    m_streamOutput->flush();
}

void CLogCSVWriter::Write(const std::string& message, const sdv::core::ELogSeverity severity,
    const std::chrono::time_point<std::chrono::system_clock> timestamp)
{
    std::string output = GetDateTime(timestamp) + m_cSeparator + GetSeverityString(severity) +
        m_cSeparator + "\"" + m_cSeparator + "\"" + Escape(message) + "\"\n";

    std::unique_lock<std::mutex> lock(m_mtxOutput);
    *m_streamOutput << output;
    m_streamOutput->flush();
}

std::string CLogCSVWriter::GetDateTime(const std::chrono::time_point<std::chrono::system_clock> timestamp)
{
    using namespace std::chrono;
    std::stringstream output;
    microseconds micros = duration_cast<microseconds>(timestamp.time_since_epoch());
    uint32_t fractionalSeconds = micros.count() % 1000000;
    std::string		  timeFormat		= "%Y.%m.%d %H:%M:%S.";
    std::time_t		  time				= system_clock::to_time_t(timestamp);
    output << std::put_time(std::localtime(&time), timeFormat.c_str());
    output << std::setw(6) << std::setfill('0') << fractionalSeconds;
    return output.str();
}

std::string CLogCSVWriter::GetSeverityString(sdv::core::ELogSeverity severity)
{
    switch (severity)
    {
    case sdv::core::ELogSeverity::trace:    return "Trace";
    case sdv::core::ELogSeverity::debug:    return "Debug";
    case sdv::core::ELogSeverity::info:     return "Info";
    case sdv::core::ELogSeverity::warning:  return "Warning";
    case sdv::core::ELogSeverity::error:    return "Error";
    case sdv::core::ELogSeverity::fatal:    return "Fatal";
    default: return "Unknown";
    }
}

std::string CLogCSVWriter::Escape(const std::string& toEscape)
{
    std::string		  escaped = toEscape;
    const std::string with("\"\"");
    for (std::string::size_type pos{}; std::string::npos != (pos = escaped.find('\"', pos)); pos += with.length())
    {
        escaped.replace(pos, 1, with.data(), with.length());
    }
    return escaped;
}
