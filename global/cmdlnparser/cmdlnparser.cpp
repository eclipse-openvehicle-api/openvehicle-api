#include "cmdlnparser.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#pragma push_macro("interface")
#ifdef interface
#undef interface
#endif
#include <Windows.h>
#pragma pop_macro("interface")
#else
#include <unistd.h>
#include <linux/limits.h>
#include <sys/ioctl.h>
#endif
#include <algorithm>
#include <cstring>

CArgumentDefBase::~CArgumentDefBase()
{
}

bool CArgumentDefBase::CompareNameAndAssign(CArgumentIterator& rargit, const std::string& rssArgument,
    const std::string& rssOptionName, bool bPartial) const
{
    if (!m_ptrArgProvide) return false; // No variable definition assigned.

    // Determine wherther the supplied argument is an option or a sub-option
    size_t nPos = 0;
    bool bOption = false;
    bool bSubOption = false;
    if (rssArgument.size() >= 2 && rssArgument[0] == '-' && rssArgument[1] == '-')
    {
        bSubOption = true;
        nPos += 2;
    }
    else if (rssArgument.size() >= 1)
    {
        if (rssArgument[0] == '-')
            bOption = true;
#ifdef _WIN32
        if (rssArgument[0] == '/')
            bOption = true;
#endif
        if (bOption) nPos++;
    }

    // Differentiate between default and option arguments... default arguments do not have a name
    if (!rssOptionName.empty()) // Option
    {
        if (!bOption && !bSubOption) return false;

        // Determine the first non-alpha-numeric character that is not part of the name.
        size_t nNameStartPos = nPos;
        std::string ssArgNameCS, ssArgNameCI;
        while (nPos < rssArgument.size())
        {
            char c = rssArgument[nPos];
            // Protect against multi-byte characters (UTF-8).
            if (static_cast<uint8_t>(c) > 127u || (!std::isalnum(c) && c != '_' && c != '?'))
                break;
            ssArgNameCS += c;
            ssArgNameCI += static_cast<char>(std::tolower(c));
            nPos++;
        }

        // Check for correct length
        if (!bPartial && rssOptionName.size() != (nPos - nNameStartPos))
           return false;

        // Check the name
        bool bFound = false;
        std::string ssArgDefNameCI = rssOptionName;
        for (char& rc : ssArgDefNameCI) rc = static_cast<char>(std::tolower(rc));
        if (m_rCLParser.CheckParseFlag(CCommandLine::EParseFlags::assignment_next_arg) || CheckFlag(EArgumentFlags::flag_option) ||
            CheckFlag(EArgumentFlags::bool_option))
        {
            // Full fit needed
            bFound |= CheckFlag(EArgumentFlags::case_sensitive) ?
                ssArgNameCS == rssOptionName :
                ssArgNameCI == ssArgDefNameCI;
        }
        else
        {
            // Partial fit allowed
            size_t nMaxNameLen = bPartial ? rssOptionName.size() : (nPos - nNameStartPos);
            bFound |= CheckFlag(EArgumentFlags::case_sensitive) ?
                ssArgNameCS.substr(0, nMaxNameLen) == rssOptionName :
                ssArgNameCI.substr(0, nMaxNameLen) == ssArgDefNameCI;
            if (bFound)
                nPos = nNameStartPos + nMaxNameLen;
        }

        // Found? If not, jump out of here
        if (!bFound) return false;
    }

    // Get the value
    std::string ssValue;
    if (m_rCLParser.CheckParseFlag(CCommandLine::EParseFlags::assignment_next_arg) && !CheckFlag(EArgumentFlags::flag_option))
    {
        if (!CheckFlag(EArgumentFlags::bool_option))
        {
            auto optArg = rargit.GetNext();
            if (!optArg)
            {
                SArgumentParseException exception("Missing arument value!");
                exception.AddIndex(rargit.GetIndexOfLastArg());
                exception.AddArgument(rssArgument);
                throw exception;
            }
            ssValue = *optArg;
        }
    }
    else
        ssValue = helper::trim(rssArgument.substr(nPos));

    // Check whether the argument was previously assigned and doesn't allow multiple assignments.
    if (m_ptrArgProvide->IsArgumentAssigned() && !m_ptrArgProvide->AllowMultiArgumentAssign())
        throw SArgumentParseException("Cannot supply more than one value for the option/argument!");

    // Assign.
    m_ptrArgProvide->ArgumentAssign(ssValue);

    // Option is available at the command line.
    m_bAvailableOnCommandLine = true;

    return true;
}

void CArgumentDefBase::AddExample(const std::string& rssExample)
{
    if (rssExample.empty()) return;
    m_lstExamples.push_back(rssExample);
}

CCommandLine::CCommandLine(uint32_t uiFlags /* = static_cast<uint32_t>(EParseFlags::assignment_character)*/) :
    m_uiParseFlags(uiFlags)
{}

CCommandLine::~CCommandLine()
{}

std::filesystem::path CCommandLine::GetApplicationPath() const
{
    std::filesystem::path path;
#ifdef _WIN32
    wchar_t szAppPath[MAX_PATH];
    GetModuleFileNameW(NULL, szAppPath, MAX_PATH);
    path = szAppPath;
#else
    char szAppPath[PATH_MAX];
    ssize_t nCount = readlink("/proc/self/exe", szAppPath, PATH_MAX);
    path = std::string(szAppPath, (nCount > 0) ? nCount : 0);
#endif
    return path;
}

void CCommandLine::DefineGroup(const std::string& rssTitle, const std::string& rssDescription /*= std::string{}*/)
{
    if (rssTitle.empty()) throw SArgumentParseException("No group title provided!");
    m_ptrCurrentGroup = std::make_shared<SGroupDef>();
    m_ptrCurrentGroup->ssTitle = rssTitle;
    if (!rssDescription.empty()) m_ptrCurrentGroup->ssDescription = rssDescription;
}

void CCommandLine::PrintFixedWidth(size_t nWidth)
{
    m_nFixedWidth = nWidth;
}

size_t CCommandLine::PrintFixedWidth() const
{
    return m_nFixedWidth;
}

void CCommandLine::PrintMaxWidth(size_t nWidth)
{
    m_nFixedWidth = 0;
    m_nMaxWidth = nWidth;
}

size_t CCommandLine::PrintMaxWidth() const
{
    return m_nFixedWidth ? m_nFixedWidth : m_nMaxWidth;
}

void CCommandLine::PrintSyntax(bool bEnable)
{
    m_bSyntaxPrint = bEnable;
}

bool CCommandLine::PrintSyntax() const
{
    return m_bSyntaxPrint;
}

void CCommandLine::PrintHelp(std::ostream& rstream, const std::string& rssHelpText /*= std::string{}*/,
    size_t nArgumentGroup /*= 0*/) const
{
    // Auto detection of the printable width?
    size_t nPrintWidth = m_nFixedWidth;
    if (!nPrintWidth)
    {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO sScreenBufferInfo = {};
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &sScreenBufferInfo))
            nPrintWidth = static_cast<size_t>(sScreenBufferInfo.srWindow.Right) - static_cast<size_t>(sScreenBufferInfo.srWindow.Left) + 1;
#else
        struct winsize ws;
        ioctl(0, TIOCGWINSZ, &ws);
        nPrintWidth = ws.ws_col;
#endif

        if (m_nMaxWidth > 0) // ignore if it's 0
        {
            // Max width?
            if (nPrintWidth > m_nMaxWidth)
                nPrintWidth = m_nMaxWidth;
        }

        // Minimum is fixed...
        if (nPrintWidth < 40)
            nPrintWidth = 40;
    }

    // Print the help text if provided.
    if (!rssHelpText.empty()) PrintHelpText(rstream, rssHelpText, nPrintWidth);

    // Get the help text of the default argument
    std::string ssDefaultArg;
    if (m_ptrDefaultArg)
        ssDefaultArg = m_ptrDefaultArg->GetHelpText();

    // Print the syntax
    size_t nOptionCnt = m_lstOptionArgs.size();
    if (m_bSyntaxPrint)
    {
        rstream << std::endl << "Syntax: " << GetApplicationPath().filename().u8string();
        if (!ssDefaultArg.empty())
            rstream << " <" << ssDefaultArg << ">";
        if (nOptionCnt)
            rstream << " [options...]";
        rstream << std::endl;
    }

    // Print the options
    if (nOptionCnt)
    {
        // Determine the length of the longest argument
        size_t nStartOfTextBlock = 0;
        auto fnLongestArgName = [&](const std::shared_ptr<CArgumentDefBase>& rptrArgument) -> void
        {
            // Make a combined argument from argument parts
            std::string ssArgument;
            for (const CArgumentDefBase::SOptionName& rsOptionName : rptrArgument->GetOptionNames())
            {
                if (!ssArgument.empty())
                    ssArgument += std::string(", ");
                if (rsOptionName.uiFlags & static_cast<uint32_t>(EArgumentFlags::option_argument))
                    ssArgument += "-";
                else
                    ssArgument += "--";
                ssArgument += rsOptionName.ssName;
            }

            nStartOfTextBlock =
                std::max((ssArgument + rptrArgument->GetArgumentVar()->GetArgumentOptionMarkup()).size(), nStartOfTextBlock);
        };
        for (const std::shared_ptr<CArgumentDefBase>& rptrArgument : m_lstOptionArgs)
        {
            if (!rptrArgument->PartOfArgumentGroup(nArgumentGroup)) continue;
            fnLongestArgName(rptrArgument);
        }

        // Add the space at the beginning and two spaces between argument name and the text
        nStartOfTextBlock += 3;

        // Print the option arguments
        std::string ssGroup;
        auto fnPrintArg = [&](const std::shared_ptr<CArgumentDefBase>& rptrArgument) -> void
        {
            // Get the group information
            std::shared_ptr<SGroupDef> ptrGroup = rptrArgument->GetGroup();
            std::string ssTitle;
            if (ptrGroup) ssTitle = ptrGroup->ssTitle;
            if (ssTitle.empty()) ssTitle = "General options";
            std::string ssDescription;
            if (ptrGroup) ssDescription = ptrGroup->ssDescription;

            // If there is a new group, print the information
            if (ssTitle != ssGroup)
            {
                rstream << std::endl;
                rstream << ssTitle << ":" << std::endl;
                ssGroup = ssTitle;
                if (!ssDescription.empty())
                    rstream << ssDescription << std::endl;
            }

            // Make a combined argument from argument parts
            std::string ssArgument;
            for (const CArgumentDefBase::SOptionName& rsOptionName : rptrArgument->GetOptionNames())
            {
                if (!ssArgument.empty())
                    ssArgument += std::string(", ");
                if (rsOptionName.uiFlags & static_cast<uint32_t>(EArgumentFlags::option_argument))
                    ssArgument += "-";
                else
                    ssArgument += "--";
                ssArgument += rsOptionName.ssName;
            }

            // The argument
            rstream << " ";
            rstream << ssArgument << rptrArgument->GetArgumentVar()->GetArgumentOptionMarkup() << "  ";

            // Calculate the starting point of the current help text (including dash and spaces)
            size_t nTextStart =
                std::max(nStartOfTextBlock, (ssArgument + rptrArgument->GetArgumentVar()->GetArgumentOptionMarkup()).size() + 3);

            // Whitespace between the argument and the help text
            for (size_t n = 3 + (ssArgument + rptrArgument->GetArgumentVar()->GetArgumentOptionMarkup()).size();
                n < nTextStart; n++)
                rstream << " ";

            // If the markup is different than the argument, add the markup to the helptext
            std::string ssHelpText = rptrArgument->GetHelpText() + "\n";

            // Add the option details for this argument
            std::string ssOptionDetails =
                rptrArgument->GetArgumentVar()->GetArgumentOptionDetails(nPrintWidth - nStartOfTextBlock - 1);
            if (ssOptionDetails.size())
            {
                ssHelpText += ssOptionDetails;
                ssHelpText += "\n";
            }

            // If there are examples, add the examples to the helptext
            if (rptrArgument->GetExamples().size())
            {
                ssHelpText += (rptrArgument->GetExamples().size() == 1 ? "Example:\n" : "Examples:\n");
                for (const std::string& rssExample : rptrArgument->GetExamples())
                {
                    ssHelpText += rssExample + "\n";
                }
            }

            // Print the block of text
            PrintBlock(rstream, ssHelpText, nStartOfTextBlock, nTextStart, nPrintWidth - 1);
        };
        for (const std::shared_ptr<CArgumentDefBase>& rptrArgument : m_lstOptionArgs)
        {
            if (!rptrArgument->PartOfArgumentGroup(nArgumentGroup)) continue;
            fnPrintArg(rptrArgument);
        }
    }
}

void CCommandLine::PrintHelpText(std::ostream& rstream, const std::string& rssHelpText, size_t nPrintWidth /*= 0*/)
{
    // Auto detection of the printable width?
    if (!nPrintWidth)
    {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO sScreenBufferInfo = {};
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &sScreenBufferInfo))
            nPrintWidth = static_cast<size_t>(sScreenBufferInfo.srWindow.Right) - static_cast<size_t>(sScreenBufferInfo.srWindow.Left) + 1;
#else
        struct winsize ws;
        ioctl(0, TIOCGWINSZ, &ws);
        nPrintWidth = ws.ws_col;
#endif
        if (nPrintWidth < 40)
            nPrintWidth = 80;
    }

    // Print the help text if provided.
    size_t nPos = 0;
    size_t nLen = rssHelpText.empty() ? 0 : rssHelpText.size();
    size_t nScreenPos = 0;
    std::string ssSpace;
    std::string ssWord;
    std::string ssIndent;
    auto fnPrintWord = [&]()
    {
        // Something to do?
        if (!ssWord.size()) return;

        // Indentation detection
        if (!nScreenPos)
        {
            if (ssSpace.size())
                ssIndent = ssSpace;
            else
                ssIndent.clear();
        }

        // Newline needed?
        size_t nLastPos = nScreenPos + ssSpace.size() + ssWord.size();
        if (nLastPos >= nPrintWidth - 1)
        {
            rstream << std::endl << ssIndent;
            nScreenPos = ssIndent.size();
            ssSpace.clear();
        }
        else
        {
            rstream << ssSpace;
            nScreenPos += ssSpace.size();
            ssSpace.clear();
        }
        rstream << ssWord;
        nScreenPos += ssWord.size();
        ssWord.clear();
    };

    // Build spaces and words and stream them
    while (nPos < nLen)
    {
        char c = rssHelpText[nPos++];
        if (std::strchr("\t\n\r\f\v ", c))
        {
            // Is there a word? Print the word...
            if (!ssWord.empty())
                fnPrintWord();

            // Add the space....
            switch (c)
            {
            case '\n':  // Newline
                rstream << std::endl;
                nScreenPos = 0;
                ssSpace.clear();
                break;
            case ' ':   // Space
                ssSpace += c;
                break;
            case '\t':  // Tab - convert to space
                ssSpace += std::string(4 - nScreenPos % 4, ' ');
                break;
            default:    // Skip all other space characters
                break;
            }

            continue;
        }

        // Collect all characters for the word
        ssWord += c;
    }
    fnPrintWord();
}

void CCommandLine::DumpArguments(std::ostream& rstream, bool bAll /*= true*/) const
{
    rstream << "Dumping arguments:\n";

    // Determine the length of the longest argument
    size_t nStartOfTextBlock = 9;	// Minimum is 9
    auto fnLongestArgName = [&](const std::shared_ptr<CArgumentDefBase>& rptrArgument) -> void
    {
        // Make a combined argument from argument parts
        std::string ssArgument;
        for (const CArgumentDefBase::SOptionName& rsOptionName : rptrArgument->GetOptionNames())
        {
            if (!ssArgument.empty())
                ssArgument += std::string(", ");
            if (rsOptionName.uiFlags & static_cast<uint32_t>(EArgumentFlags::option_argument))
                ssArgument += "-";
            else
                ssArgument += "--";
            ssArgument += rsOptionName.ssName;
        }

        nStartOfTextBlock = std::max(ssArgument.size(), nStartOfTextBlock);
    };
    for (const std::shared_ptr<CArgumentDefBase>& rptrArgument : m_lstOptionArgs)
        fnLongestArgName(rptrArgument);

    // Add a double space (" ") at the beginning and the dash and space ("- ") between argument name and the value
    nStartOfTextBlock += 4;

    // Maximum start block is 30
    nStartOfTextBlock = std::min(static_cast<size_t>(30), nStartOfTextBlock);

    // Print header
    rstream << " Argument";
    for (size_t n = 10; n < nStartOfTextBlock; n++)
        rstream << " ";
    rstream << " | Set | Value\n";
    for (size_t n = 0; n < nStartOfTextBlock; n++)
        rstream << "-";
    rstream << "+-----+------------------------------\n";

    // Print the arguments
    auto fnDumpArgs = [&](const std::shared_ptr<CArgumentDefBase>& rptrArgument) -> void
    {
        // Make a combined argument from argument parts
        std::string ssArgument;
        for (const CArgumentDefBase::SOptionName& rsOptionName : rptrArgument->GetOptionNames())
        {
            if (!ssArgument.empty())
                ssArgument += std::string(", ");
            if (rsOptionName.uiFlags & static_cast<uint32_t>(EArgumentFlags::option_argument))
                ssArgument += "-";
            else
                ssArgument += "--";
            ssArgument += rsOptionName.ssName;
        }

        if (!bAll && !rptrArgument->GetArgumentVar()->IsArgumentAssigned()) return;

        // The argument
        std::string ssArgumentName = ssArgument.empty() ? std::string("<default>") : (std::string("-") + ssArgument);
        rstream << " " << ssArgumentName;

        // Calculate the starting point of the current help text
        size_t nTextStart = std::max(nStartOfTextBlock, ssArgumentName.size() + 3);

        // Whitespace between the argument and the help text
        for (size_t n = 1 + ssArgumentName.size(); n < nTextStart; n++)
            rstream << " ";

        // Print whether the argument was set or not
        rstream << (rptrArgument->GetArgumentVar()->IsArgumentAssigned() ? "| yes | " : "| no  | ");

        // The value
        rstream << rptrArgument->GetArgumentVar()->GetArgumentValueString() << std::endl;
    };
    if (m_ptrDefaultArg) fnDumpArgs(m_ptrDefaultArg);
    for (const std::shared_ptr<CArgumentDefBase>& rptrArgument : m_lstOptionArgs)
        fnDumpArgs(rptrArgument);
}

std::vector<std::string> CCommandLine::IncompatibleArguments(size_t nArgumentGroup, bool bFull /*= true*/) const
{
    std::vector<std::string> vecIncompatible;
    for (auto& prArgument : m_lstSupplied)
    {
        // Only valid for options, not for default arguments
        if (prArgument.first.get().CheckFlag(EArgumentFlags::default_argument)) continue;

        // Is the argument compatible?
        if (prArgument.first.get().PartOfArgumentGroup(nArgumentGroup)) continue;

        // Full option requested?
        if (bFull)
        {
            vecIncompatible.push_back(prArgument.second);
            continue;
        }

        // Make lower case string
        auto fnMakeLower = [](const std::string& rss)
        {
            std::string rssTemp;
            rssTemp.reserve(rss.size());
            for (char c : rss)
                rssTemp += static_cast<char>(std::tolower(c));
            return rssTemp;
        };

        // Find the option text
        std::string ssOptionText;
        std::string ssArgumentLC = fnMakeLower(prArgument.second);
        for (auto& rsOptionText : prArgument.first.get().GetOptionNames())
        {
            std::string ssOptionNameLC = fnMakeLower(rsOptionText.ssName);
            std::string ssOptionLC = std::string("-") + ssOptionNameLC;
            if (rsOptionText.uiFlags & static_cast<uint32_t>(EArgumentFlags::option_argument) &&
                ssArgumentLC.substr(0, ssOptionLC.size()) == ssOptionLC)
            {
                ssOptionText = prArgument.second.substr(0, ssOptionLC.size());
                break;
            }
#ifdef _WIN32
            ssOptionLC = std::string("/") + ssOptionNameLC;
            if (rsOptionText.uiFlags & static_cast<uint32_t>(EArgumentFlags::option_argument) &&
                ssArgumentLC.substr(0, ssOptionLC.size()) == ssOptionLC)
            {
                ssOptionText = prArgument.second.substr(0, ssOptionLC.size());
                break;
            }
#endif
            ssOptionLC = std::string("--") + ssOptionNameLC;
            if (rsOptionText.uiFlags & static_cast<uint32_t>(EArgumentFlags::sub_option_argument) &&
                ssArgumentLC.substr(0, ssOptionLC.size()) == ssOptionLC)
            {
                ssOptionText = prArgument.second.substr(0, ssOptionLC.size());
                break;
            }
        }

        // Just in case... should have been
        if (ssOptionText.empty())
            ssOptionText = prArgument.second;

        // Store the options
        vecIncompatible.push_back(ssOptionText);
    }

    return vecIncompatible;
}

void PrintBlock(std::ostream& rstream, const std::string& rssText, size_t nIndentPos, size_t nCurrentPos, size_t nMaxPos)
{
    // Scan through the help text and add the text word for word, adding \n if necessary
    size_t nScanPos = 0, nPrintPos = 0;
    do
    {
        // Determine whether the next text is white space
        bool bDoProcess = false;
        bool bEOL = false;
        switch (rssText[nScanPos])
        {
        case ' ':
        case '\t':
        case '\n':
            bDoProcess = true;
            break;
        default:
            break;
        }

        // Special case, the last character
        if (nScanPos == rssText.size() - 1)
        {
            if (!bDoProcess) // Include when this is not whitespace
                nScanPos++;

            bEOL = true;
            bDoProcess = true;
        }

        // When not whitespace, check the next scan
        if (!bDoProcess)
        {
            nScanPos++;
            continue;
        }

        // Check whether the text still fits (or if not, whether we are not exactly at
        // the beginning of a new text block
        if (nCurrentPos + (nScanPos - nPrintPos) > nMaxPos &&
            nCurrentPos != nIndentPos)
        {
            // Insert a newline
            rstream << std::endl;

            // And fill up whitespace
            for (size_t n = 0; n < nIndentPos; n++)
                rstream << " ";

            nCurrentPos = nIndentPos;
        }

        // Print the text until the end of the line or the scan pos
        size_t nTextLen = std::min(nScanPos - nPrintPos, nMaxPos - nCurrentPos);
        rstream << rssText.substr(nPrintPos, nTextLen);
        nPrintPos += nTextLen;
        nScanPos = nPrintPos;
        nCurrentPos += nTextLen;
        if (nScanPos >= rssText.size()) continue;

        // Print the whitespace at nScanPos
        char chTemp = rssText[nScanPos]; // Needed since the scan position changes
        switch (chTemp)
        {
        case ' ':
            // Increase the position
            nCurrentPos++;

            // Beyond the end of the line?
            if (nCurrentPos < nMaxPos)
                rstream << " ";
            break;
        case '\t':
            // Beyond the end of the line?
            if (nCurrentPos + (8 - nCurrentPos % 8) > nMaxPos)
                nCurrentPos = nMaxPos + 1;
            nPrintPos++;
            nScanPos++;
            break;
        case '\n':
            rstream << std::endl;
            nPrintPos++;
            nScanPos++;
            // Do not indent any more with the last character
            for (size_t n = 0; !bEOL && n < nIndentPos; n++)
                rstream << " ";
            nCurrentPos = nIndentPos;
            break;
        default:
            break;
        }
        // Newline needed?
        if (nCurrentPos > nMaxPos)
        {
            rstream << std::endl;
            // Do not indent any more with the last character
            for (size_t n = 0; !bEOL && n < nIndentPos; n++)
                rstream << " ";
            nCurrentPos = nIndentPos;
        }
        // Print the tab and increase the scan position
        switch (chTemp)
        {
        case '\t':
            nCurrentPos += 8 - nCurrentPos % 8;
            rstream << "\t";
            break;
        case '\n':
            break;
        case ' ':
            nPrintPos++;
            nScanPos++;
            break;
        default:
            break;
        }
    // Hypothetical comparison; is always true. This is wanted. Suppress CppCheck warning
    // cppcheck-suppress knownConditionTrueFalse
    } while (nScanPos < rssText.size());
}
