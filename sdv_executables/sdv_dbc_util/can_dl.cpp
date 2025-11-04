#include "can_dl.h"
#include <support/any.h>
#include <cmath>
#include <thread>
#include <chrono>

void CAN_Extract_Sample()
{
    struct SSignal
    {
        uint32_t uiStartBit;
        uint32_t uiSize;
    };

    // Motorola format:
    // The signal occupies the byte where the start bit determines the MSB and any number of following bytes that start with
    // the most significant bit. For example:
    // Signal 1: start=7 length = 4             = 0xA
    // Signal 2: start=3 length = 8             = 0x5D
    // Signal 3: start=11 length = 6            = 0x31
    // Signal 4: start=21 length = 6            = 0x2D
    // Signal 5: start=31 length = 8            = 0x96
    // Signal 6: start=39 length = 32           = 0x33CC55AA
    //      7   6   5   4   3   2   1   0
    //  0 | Signal 1      | Signal 2 MSB  |     b10100101   0xA5
    //  1 | signal 2 LSB  | Signal 3 MSB  |     b11011100   0xDC
    //  2 | Sig 3 | Signal 4              |     b01101101   0x6D
    //  3 | Signal 5                      |     b10010110   0x96
    //  4 | Signal 6 MSB                  |     b00110011   0x33
    //  5 |                               |     b11001100   0xCC
    //  6 |                               |     b01010101   0x55
    //  7 |                  Signal 6 LSB |     b10101010   0xAA

    SSignal rgMotorolaSigs[] = {
        {7, 4},
        {3, 8},
        {11, 6},
        {21, 6},
        {31, 8},
        {39, 32}
    };
    uint8_t pMotorolaData[8] = { 0xA5, 0xDC, 0x6D, 0x96, 0x33, 0xCC, 0x55, 0xAA };
    for (SSignal& rsSig : rgMotorolaSigs)
    {
        auto fnInverseBitPos = [](uint32_t uiPos) -> uint32_t
        {
            uint32_t uiInverseStartBit = (uiPos >> 3) << 3;
            uint32_t uiInverseStartBitInByte = (8 - ((uiPos + 1) & 7)) & 7;
            return uiInverseStartBit + uiInverseStartBitInByte;
        };
        auto fnFirstByte = [&]()
        {
            return rsSig.uiStartBit >> 3;
        };
        auto fnLastByte = [&]()
        {
            return ((fnInverseBitPos(rsSig.uiStartBit) + rsSig.uiSize - 1)) >> 3;
        };
        auto fnFirstByteMask = [&]()
        {
            return (1 << ((rsSig.uiStartBit & 0x7) + 1)) - 1;
        };
        auto fnShiftRight = [&]()
        {
            return (fnInverseBitPos(fnInverseBitPos(rsSig.uiStartBit) + rsSig.uiSize) + 1) & 0x7;
        };
        auto fnValue = [&]()
        {
            uint64_t uiValue = 0;
            for (size_t nSrcIndex = fnFirstByte(); nSrcIndex <= fnLastByte(); nSrcIndex++)
            {
                uint8_t uiByte = pMotorolaData[nSrcIndex];
                if (nSrcIndex == fnFirstByte()) uiByte &= fnFirstByteMask();
                uiValue = uiValue << 8 | uiByte;
                if (nSrcIndex == fnLastByte()) uiValue >>= fnShiftRight();
            }
            return uiValue;
        };
        std::cout << "Start-bit=" << rsSig.uiStartBit << " Length=" << rsSig.uiSize << " First-byte=" << fnFirstByte() <<
            " Last-byte=" << fnLastByte() << " First-byte-mask=" << fnFirstByteMask() << " Shift-right=" << fnShiftRight() <<
            " Value=0x" << std::hex << fnValue() << std::dec << std::endl;
    }

    // Intel format:
    // The signal occupies the byte where the start bit determines the MSB and any number of following bytes that start with
    // the most significant bit. For example:
    // Signal 1: start=0 length = 4             = 0xA
    // Signal 2: start=4 length = 8             = 0x5D
    // Signal 3: start=12 length = 6            = 0x31
    // Signal 4: start=18 length = 6            = 0x2D
    // Signal 5: start=24 length = 8            = 0x96
    // Signal 6: start=32 length = 32           = 0x33CC55AA
    //      7   6   5   4   3   2   1   0
    //  0 | Signal 2 LSB  | Signal 1      |     b11011010   0xDA
    //  1 | signal 3 LSB  | Signal 2 MSB  |     b00010101   0x15
    //  2 | Signal 4              | Sig 3 |     b10110111   0xB7
    //  3 | Signal 5                      |     b10010110   0x96
    //  4 | Signal 6 MSB                  |     b00110011   0xAA
    //  5 |                               |     b11001100   0x55
    //  6 |                               |     b01010101   0xCC
    //  7 |                  Signal 6 LSB |     b10101010   0x33

    const SSignal rgIntelSigs[] = {
        {0, 4},
        {4, 8},
        {12, 6},
        {18, 6},
        {24, 8},
        {32, 32}
    };
    uint8_t pIntelData[8] = { 0xDA, 0x15, 0xB7, 0x96, 0xAA, 0x55, 0xCC, 0x33 };
    for (const SSignal& rsSig : rgIntelSigs)
    {
        auto fnFirstByte = [&]()
        {
            return rsSig.uiStartBit >> 3;
        };
        auto fnLastByte = [&]()
        {
            return (rsSig.uiStartBit + rsSig.uiSize - 1) >> 3;
        };
        auto fnLastByteMask = [&]()
        {
            return (1 << (((rsSig.uiStartBit + rsSig.uiSize - 1) & 0x7) + 1)) - 1;
        };
        auto fnShiftRight = [&]()
        {
            return rsSig.uiStartBit & 0x7;
        };
        auto fnValue = [&]()
        {
            uint64_t uiValue = 0;
            for (size_t nSrcIndex = fnLastByte(); nSrcIndex >= fnFirstByte() && nSrcIndex <= fnLastByte(); nSrcIndex--)
            {
                uint8_t uiByte = pIntelData[nSrcIndex];
                if (nSrcIndex == fnLastByte()) uiByte &= fnLastByteMask();
                uiValue = uiValue << 8 | uiByte;
                if (nSrcIndex == fnFirstByte()) uiValue >>= fnShiftRight();
            }
            return uiValue;
        };
        std::cout << "Start-bit=" << rsSig.uiStartBit << " Length=" << rsSig.uiSize << " First-byte=" << fnFirstByte() <<
            " Last-byte=" << fnLastByte() << " Last-byte-mask=" << fnLastByteMask() << " Shift-right=" << fnShiftRight() <<
            " Value=0x" << std::hex << fnValue() << std::dec << std::endl;
    }
}

/**
 * @brief Header file template. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szHdrTemplate[] = R"code(/**
 * @file %hdr_path%
 * @date %date%
 * This file defines the data link object between CAN and the V-API devices.
 * This file was generated by the DBC utility from:
 *   %dbc_sources%
 *   %dbc_version%
 */
#ifndef %safeguard%
#define %safeguard%

#include <support/component_impl.h>
#include <interfaces/can.h>
#include <support/interface_ptr.h>
#include <support/signal_support.h>

/**
 * @brief Data link class.
 */
class CDataLink : public sdv::CSdvObject, public sdv::IObjectControl, public sdv::can::IReceive
{
public:
    /**
     * @brief Constructor
     */
    CDataLink();

    /**
     * @brief Destructor
     */
    ~CDataLink();

    // Interface map
    BEGIN_SDV_INTERFACE_MAP()
        SDV_INTERFACE_ENTRY(sdv::IObjectControl)
        SDV_INTERFACE_ENTRY(sdv::can::IReceive)
    END_SDV_INTERFACE_MAP()

    // Declarations
    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("CAN_data_link")
    DECLARE_DEFAULT_OBJECT_NAME("DataLink")
    DECLARE_OBJECT_SINGLETON()

    /**
     * @brief Initialize the object. Overload of sdv::IObjectControl::Initialize.
     * @param[in] ssObjectConfig Optional configuration string.
     */
    void Initialize(const sdv::u8string& ssObjectConfig) override;

    /**
     * @brief Get the current status of the object. Overload of sdv::IObjectControl::GetStatus.
     * @return Return the current status of the object.
     */
    sdv::EObjectStatus GetStatus() const override;

    /**
     * @brief Set the component operation mode. Overload of sdv::IObjectControl::SetOperationMode.
     * @param[in] eMode The operation mode, the component should run in.
     */
    void SetOperationMode(sdv::EOperationMode eMode) override;

    /**
     * @brief Shutdown called before the object is destroyed. Overload of sdv::IObjectControl::Shutdown.
     */
    void Shutdown() override;

    /**
     * @brief Process a receive a CAN message. Overload of sdv::can::IReceive::Receive.
     * @param[in] sMsg Message that was received.
     * @param[in] uiIfcIndex Interface index of the received message.
     */
    virtual void Receive(/*in*/ const sdv::can::SMessage& sMsg, /*in*/ uint32_t uiIfcIndex) override;

    /**
     * @brief Process an error frame. Overload of sdv::can::IReceive::Error.
     * @param[in] sError Error frame that was received.
     * @param[in] uiIfcIndex Interface index of the received message.
     */
    virtual void Error(/*in*/ const sdv::can::SErrorFrame& sError, /*in*/ uint32_t uiIfcIndex) override;

private:
    /**
     * @brief Union containing all the compound values needed to convert between the DBC defined types.
     */
    union UValueHelper
    {
        uint64_t    uiUint64Value;      ///< The 64-bit unsingned value.
        int64_t     iInt64Value;        ///< The 64-bit signed value.

        /**
         * @brief The structure mapping the 32-bit value types into the 64-bit.
         */
        struct S32
        {
    #if (!defined(_MSC_VER) || defined(__clang__)) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
            uint32_t uiPadding;     ///< Padding for big endian byte ordering (MSB)
    #endif
            /**
             * @brief The 32-bit union containing the possible 32-bit values.
             */
            union U32Value
            {
                int32_t     iValue;     ///< 32-bit signed integer.
                uint32_t    uiValue;    ///< 32-bit unsigned integer.
                float       fValue;     ///< 32-bit floating point number.
            } u32;  ///< 32-bit union instance.

    #if (defined(_MSC_VER) && !defined(__clang__)) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
            uint32_t uiPadding;     ///< Padding for little endian byte ordering (MSB)
    #endif
        } s32;

        double      dValue;     ///< 64-bit double precision floating point number.
    };
%message_def%
    sdv::EObjectStatus              m_eStatus = sdv::EObjectStatus::initialization_pending;  ///< Keep track of the object status.
    size_t                          m_nIfcIndex = %ifc_index%;              ///< CAN Interface index.
    sdv::can::IRegisterReceiver*    m_pRegister = nullptr;                  ///< CAN receiver registration interface.
    sdv::can::ISend*                m_pSend = nullptr;                      ///< CAN sender interface.
    sdv::core::CDispatchService     m_dispatch;                             ///< Dispatch service
};

DEFINE_SDV_OBJECT(CDataLink)

#endif // !defined %safeguard%
)code";

/**
 * @brief Cpp file template. Code chunks are inserted at the keywords surrounded by %%.
 */
const char szCppTemplate[] = R"code(/**
 * @file %cpp_path%
 * @date %date%
 * This file implements the data link object between CAN and the V-API devices.
 * This file was generated by the DBC utility from:
 *   %dbc_sources%
 *   %dbc_version%
 */
#include "%hdr_path%"
#include <algorithm>
#include <cmath>

#ifdef _MSC_VER
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

CDataLink::CDataLink()%init_var%
{}

CDataLink::~CDataLink()
{
    Shutdown(); // Just in case
}

void CDataLink::Initialize(const sdv::u8string& /*ssObjectConfig*/)
{
    if (m_eStatus != sdv::EObjectStatus::initialization_pending) return;

    // Get the CAN communication object.
    sdv::TInterfaceAccessPtr ptrCANObject = sdv::core::GetObject("CAN_Communication_Object");
    if (!ptrCANObject)
    {
		SDV_LOG_ERROR("CDataLink::Initialize() failure, 'CAN_Communication_Object' not found");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    %init_ifc_index%// Get the CAN receiver registration interface.
    m_pRegister = ptrCANObject.GetInterface<sdv::can::IRegisterReceiver>();
    if (!m_pRegister)
    {
		SDV_LOG_ERROR("CDataLink::Initialize() failure, 'sdv::can::IRegisterReceiver' interface not found");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }
    m_pRegister->RegisterReceiver(static_cast<sdv::can::IReceive*>(this));

    // Get the CAN transmit interface
    m_pSend = ptrCANObject.GetInterface<sdv::can::ISend>();
    if (!m_pSend)
    {
		SDV_LOG_ERROR("CDataLink::Initialize() failure, 'sdv::can::ISend' interface not found");
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }

    // Initialize messages
    bool bSuccess = true;%init_msg%

    m_eStatus = bSuccess ? sdv::EObjectStatus::initialized : sdv::EObjectStatus::initialization_failure;
}

sdv::EObjectStatus CDataLink::GetStatus() const
{
    return m_eStatus;
}

void CDataLink::SetOperationMode(sdv::EOperationMode eMode)
{
    switch (eMode)
    {
    case sdv::EOperationMode::configuring:
        if (m_eStatus == sdv::EObjectStatus::running || m_eStatus == sdv::EObjectStatus::initialized)
            m_eStatus = sdv::EObjectStatus::configuring;
        break;
    case sdv::EOperationMode::running:
        if (m_eStatus == sdv::EObjectStatus::configuring || m_eStatus == sdv::EObjectStatus::initialized)
            m_eStatus = sdv::EObjectStatus::running;
        break;
    default:
        break;
    }
}

void CDataLink::Shutdown()
{
    m_eStatus = sdv::EObjectStatus::shutdown_in_progress;

    // Unregister receiver interface.
    if (m_pRegister) m_pRegister->UnregisterReceiver(static_cast<sdv::can::IReceive*>(this));
    m_pRegister = nullptr;

    m_pSend = nullptr;

    // Terminate messages%term_msg%

    // Update the status
    m_eStatus = sdv::EObjectStatus::destruction_pending;
}

void CDataLink::Receive(/*in*/ [[maybe_unused]] const sdv::can::SMessage& sMsg, /*in*/ uint32_t uiIfcIndex)
{
    if (static_cast<size_t>(uiIfcIndex) != m_nIfcIndex) return;
    %receive_switch_begin%%receive_switch%%receive_switch_end%
}

void CDataLink::Error(/*in*/ [[maybe_unused]] const sdv::can::SErrorFrame& sError, /*in*/ uint32_t uiIfcIndex)
{
    if (static_cast<size_t>(uiIfcIndex) != m_nIfcIndex) return;

    // TODO: Currently no error frame handling...
}

%msg_impl%
)code";

CCanDataLinkGen::CCanDataLinkGen(const std::filesystem::path& rpathOutputDir, const dbc::CDbcParser& rparser,
    const std::string& rsVersion, const std::string& rssIfcName, size_t nIfcIdx, const std::vector<std::string>& rvecNodes) :
    m_rparser(rparser)
{
    // Create project directory
    if (!rpathOutputDir.empty())
        m_pathProject = rpathOutputDir;
    m_pathProject /= "can_dl";
    for (size_t nCnt = 0; nCnt < 5; nCnt++)
    {
        if (!std::filesystem::exists(m_pathProject))
            std::filesystem::create_directories(m_pathProject);
        else
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // Open the streams
    m_pathHeader = m_pathProject / "datalink.h";
    m_pathCpp = m_pathProject / "datalink.cpp";
    m_fstreamHeader.open(m_pathHeader, std::ios::out | std::ios::trunc);
    m_fstreamCpp.open(m_pathCpp, std::ios::out | std::ios::trunc);

    CKeywordMap mapKeywords;
    mapKeywords["hdr_path"] = m_pathHeader.filename().generic_u8string();
    mapKeywords["cpp_path"] = m_pathCpp.filename().generic_u8string();
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream sstreamDate;
    sstreamDate << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    mapKeywords["date"] = sstreamDate.str();
    auto vecSources = rparser.GetSources();
    std::stringstream sstreamDbcSources;
    for (const dbc::CDbcSource& rsource : vecSources)
    {
        if (!sstreamDbcSources.str().empty())
            sstreamDbcSources << ", ";
        sstreamDbcSources << "\"" << rsource.Path().filename().generic_u8string() << "\"";
    }
    mapKeywords["dbc_sources"] = sstreamDbcSources.str();
    mapKeywords["dbc_version"] = CodeDBCFileVersion(rsVersion);

    // Safeguard
    // Safeguards start with "__IDL_GENERATED__", add the file name, add the date and time and end with "__"
    std::stringstream sstreamSafeguard;
    sstreamSafeguard << "__DBC_GENERATED__DATALINK_H__";
    sstreamSafeguard << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S") << "_";
    sstreamSafeguard << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    sstreamSafeguard << "__";
    mapKeywords["safeguard"] = sstreamSafeguard.str();

    // Interface index
    mapKeywords["ifc_index"] = to_string(nIfcIdx);
    mapKeywords["init_ifc_index"] = CodeInitInterfaceIndex(rssIfcName);

    // Generate message structures, signal definitions, message switch
    std::stringstream sstreamReceiveSwitchBegin;
    std::stringstream sstreamReceiveSwitch;
    std::stringstream sstreamReceiveSwitchEnd;
    std::stringstream sstreamMessageDef;
    std::stringstream sstreamInitMsg;
    std::stringstream sstreamTermMsg;
    std::stringstream sstreamMsgImpl;
    std::stringstream sstreamInitVarImpl;

    auto vecMsgIDs = rparser.GetMessageIDs();
    for (uint32_t uiRawMsgID : vecMsgIDs)
    {
        auto prMessage = rparser.GetMsgDef(uiRawMsgID);
        if (!prMessage.second) continue;

        // Run through the transmitter nodes and check whether the receiver of the message is defined in our node list.
        bool bPartOfTransmitNode = false;
        for (const std::string& rssTransmitter : prMessage.first.vecTransmitters)
        {
            if (rvecNodes.empty() || std::find(rvecNodes.begin(), rvecNodes.end(), rssTransmitter) != rvecNodes.end())
            {
                bPartOfTransmitNode = true;
                break;
            }
        }

        // Run through the signal definitions and check whether the receiver of the message is defined in our node list.
        bool bPartOfReceiveNode = false;
        for (const dbc::SSignalDef& rSignal : prMessage.first.vecSignals)
        {
            if (std::find_if(rSignal.vecReceivers.begin(), rSignal.vecReceivers.end(), [&](const std::string& rssRcvNode)
                {
                    if (rvecNodes.empty()) return true;
                    return std::find(rvecNodes.begin(), rvecNodes.end(), rssRcvNode) != rvecNodes.end();
                }) != rSignal.vecReceivers.end())
            {
                bPartOfReceiveNode = true;
                break;
            }
        }

        if (!bPartOfReceiveNode && !bPartOfTransmitNode) continue;

        if (sstreamInitVarImpl.str().empty())
            sstreamInitVarImpl << R"code( :
    )code";
        else
            sstreamInitVarImpl << R"code(,
    )code";

        // Create message definitions
        if (bPartOfReceiveNode)
        {
            sstreamMessageDef << CodeRxMessageDefinition(prMessage.first);
            sstreamInitMsg << CodeInitRxMessage(prMessage.first);
            sstreamTermMsg << CodeTermRxMessage(prMessage.first);
            sstreamMsgImpl << CodeRxMessageFunctions(prMessage.first);
            sstreamInitVarImpl << CodeInitVarRxMessage(prMessage.first);
            sstreamReceiveSwitch << R"code(
    case )code" << rparser.ExtractMsgId(uiRawMsgID).first << R"code(: // )code" << prMessage.first.ssName << R"code(
        m_sRxMsg)code" << prMessage.first.ssName << R"code(.Process(sMsg.seqData);
        break;)code";
        }
        if (bPartOfTransmitNode)
        {
            if (bPartOfReceiveNode)
                sstreamInitVarImpl << R"code(,
    )code";
            sstreamMessageDef << CodeTxMessageDefinition(prMessage.first);
            sstreamInitMsg << CodeInitTxMessage(prMessage.first);
            sstreamTermMsg << CodeTermTxMessage(prMessage.first);
            sstreamMsgImpl << CodeTxMessageFunctions(prMessage.first);
            sstreamInitVarImpl << CodeInitVarTxMessage(prMessage.first);
        }
    }
    if (!sstreamReceiveSwitch.str().empty())
    {
        sstreamReceiveSwitchBegin << R"code(
    switch (sMsg.uiID)
    {)code";
        sstreamReceiveSwitchEnd << R"code(
    default:
        break;
    })code";
    }

    mapKeywords["message_def"] = sstreamMessageDef.str();
    mapKeywords["receive_switch_begin"] = sstreamReceiveSwitchBegin.str();
    mapKeywords["receive_switch"] = sstreamReceiveSwitch.str();
    mapKeywords["receive_switch_end"] = sstreamReceiveSwitchEnd.str();
    mapKeywords["msg_impl"] = sstreamMsgImpl.str();
    mapKeywords["init_msg"] = sstreamInitMsg.str();
    mapKeywords["term_msg"] = sstreamTermMsg.str();
    mapKeywords["init_var"] = sstreamInitVarImpl.str();

    // Replace keywords and stream the code files.
    m_fstreamHeader << ReplaceKeywords(szHdrTemplate, mapKeywords);
    m_fstreamCpp << ReplaceKeywords(szCppTemplate, mapKeywords);

    // Generate DBC-signal registration
    // Add CAN input processing function
    // Add CAN output timer triggers

}

std::string CCanDataLinkGen::ReplaceKeywords(const std::string& rssStr, const CKeywordMap& rmapKeywords, char cMarker /*= '%'*/)
{
    std::stringstream sstream;
    size_t nPos = 0;
    while (nPos < rssStr.size())
    {
        // Find the initial separator
        size_t nSeparator = rssStr.find(cMarker, nPos);
        sstream << rssStr.substr(nPos, nSeparator == std::string::npos ? nSeparator : nSeparator - nPos);
        nPos = nSeparator;
        if (nSeparator == std::string::npos) continue;
        nPos++;

        // Find the next separator.
        nSeparator = rssStr.find(cMarker, nPos);
        if (nSeparator == std::string::npos)
        {
            // Internal error: missing second separator during code building.
            continue;
        }

        // Find the keyword in the keyword map (between the separator and the position).
        CKeywordMap::const_iterator itKeyword = rmapKeywords.find(rssStr.substr(nPos, nSeparator - nPos));
        if (itKeyword == rmapKeywords.end())
        {
            // Internal error: invalid keyword during building code.
            nPos = nSeparator + 1;
            continue;
        }
        sstream << itKeyword->second;
        nPos = nSeparator + 1;
    }
    return sstream.str();
}

std::string CCanDataLinkGen::CodeInitInterfaceIndex(const std::string& rssIfcName)
{
    if (rssIfcName.empty()) return std::string();
    CKeywordMap mapKeywords;
    mapKeywords["ifc_name"] = rssIfcName;
    return ReplaceKeywords(R"code(sdv::can::IInformation* pInfo = ptrCANObject.GetInterface<sdv::can::IInformation>();
    if (!pInfo)
    {
        // CAN information interface not found.
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }
    sdv::sequence<sdv::string> seqInterfaces = pInfo->GetInterfaces();
    size_t nIndex = 0;
    for (; nIndex < seqInterfaces.size(); nIndex++)
    {
        if (seqInterfaces[nIndex] == "%ifc_name%")
        {
            m_nIfcIndex = nIndex;
            break;
        }
    }
    if (nIndex >= seqInterfaces.size())
    {
        // Interface with supplied name not found.
        m_eStatus = sdv::EObjectStatus::initialization_failure;
        return;
    }
)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeRxMessageDefinition(const dbc::SMessageDef& rsMsg)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;
    mapKeywords["msg_id"] = to_string(dbc::CDbcParser::ExtractMsgId(rsMsg.uiId).first);
    std::stringstream sstreamSignalDecl;
    for (const dbc::SSignalDef& rsSignal : rsMsg.vecSignals)
        sstreamSignalDecl << CodeSignalDecl(rsSignal);
    mapKeywords["sig_decl"] = std::move(sstreamSignalDecl.str());

    return ReplaceKeywords(R"code(
    /**
     * @brief RX CAN message definition of: %msg_name% [id=%msg_id%]
     */
    struct SRxMsg_%msg_name%
    {
        /**
         * @brief Constructor
         * @param[in] rdispatch Reference to the dispatch service.
         */
        SRxMsg_%msg_name%(sdv::core::CDispatchService& rdispatch);

        /**
         * @brief Initialize the message by registering all signals.
         */
        bool Init();

        /**
         * @brief Terminate the message by unregistering all signals.
         */
        void Term();

        /**
         * @brief Process received data.
         * @param[in] rseqData Reference to the message data to process.
         */
        void Process(const sdv::sequence<uint8_t>& rseqData);

        sdv::core::CDispatchService&    m_rdispatch;        ///< Reference to the dispatch service.
        %sig_decl%
    } m_sRxMsg%msg_name%;
)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeTxMessageDefinition(const dbc::SMessageDef& rsMsg)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;
    mapKeywords["msg_id"] = to_string(dbc::CDbcParser::ExtractMsgId(rsMsg.uiId).first);
    std::stringstream sstreamSignalDecl;
    for (const dbc::SSignalDef& rsSignal : rsMsg.vecSignals)
        sstreamSignalDecl << CodeSignalDecl(rsSignal);
    mapKeywords["sig_decl"] = std::move(sstreamSignalDecl.str());

    return ReplaceKeywords(R"code(
    /**
     * @brief TX CAN message definition of: %msg_name% [id=%msg_id%]
     */
    struct STxMsg_%msg_name%
    {
        /**
         * @brief Constructor
         * @param[in] rdispatch Reference to the dispatch service.
         */
        STxMsg_%msg_name%(sdv::core::CDispatchService& rdispatch);

        /**
         * @brief Initialize the message by registering all signals.
         * @param[in] pSend The send-interface of the CAN.
         */
        bool Init(sdv::can::ISend* pSend);

        /**
         * @brief Terminate the message by unregistering all signals.
         */
        void Term();

        /**
         * @brief Transmit data.
         * @param[in] pCanSend Pointer to the CAN send interface.
         */
        void Transmit();

        sdv::core::CDispatchService&    m_rdispatch;            ///< Reference to the dispatch service.
        sdv::core::CTrigger             m_trigger;              ///< Message trigger being called by the dispatch service.
        sdv::can::ISend*        m_pSend = nullptr;      ///< Message sending interface.
        %sig_decl%
    } m_sTxMsg%msg_name%;
)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeSignalDecl(const dbc::SSignalDef& rsSig)
{
    CKeywordMap mapKeywords;
    mapKeywords["sig_name"] = rsSig.ssName;
    mapKeywords["sig_unit"] = rsSig.ssUnit;
    return ReplaceKeywords(R"code(
        /// Signal %sig_name% with unit %sig_unit%
        sdv::core::CSignal      m_sig%sig_name%;)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeInitVarRxMessage(const dbc::SMessageDef& rsMsg)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;

    return ReplaceKeywords(R"code(m_sRxMsg%msg_name%(m_dispatch))code", mapKeywords);
}

std::string CCanDataLinkGen::CodeInitVarTxMessage(const dbc::SMessageDef& rsMsg)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;

    return ReplaceKeywords(R"code(m_sTxMsg%msg_name%(m_dispatch))code", mapKeywords);
}

std::string CCanDataLinkGen::CodeInitRxMessage(const dbc::SMessageDef& rsMsg)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;

    return ReplaceKeywords(R"code(
    bSuccess &= m_sRxMsg%msg_name%.Init();)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeInitTxMessage(const dbc::SMessageDef& rsMsg)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;

    return ReplaceKeywords(R"code(
    bSuccess &= m_sTxMsg%msg_name%.Init(m_pSend);)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeTermRxMessage(const dbc::SMessageDef& rsMsg)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;

    return ReplaceKeywords(R"code(
    m_sRxMsg%msg_name%.Term();)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeTermTxMessage(const dbc::SMessageDef& rsMsg)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;

    return ReplaceKeywords(R"code(
    m_sTxMsg%msg_name%.Term();)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeRxMessageFunctions(const dbc::SMessageDef& rsMsg)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;
    std::stringstream sstreamSignalRegister;
    std::stringstream sstreamSignalUnregister;
    std::stringstream sstreamSignalProcessing;
    for (const dbc::SSignalDef& rsSignal : rsMsg.vecSignals)
    {
        sstreamSignalRegister << CodeRegisterRxSignal(rsMsg, rsSignal);
        sstreamSignalUnregister << CodeUnregisterSignal(rsMsg, rsSignal);
        sstreamSignalProcessing << CodeProcessRxSignal(rsMsg, rsSignal);
    }
    mapKeywords["sig_register"] = std::move(sstreamSignalRegister.str());
    mapKeywords["sig_unregister"] = std::move(sstreamSignalUnregister.str());
    mapKeywords["msg_len"] = to_string(rsMsg.uiSize);
    mapKeywords["process_signals"] = std::move(sstreamSignalProcessing.str());

    return ReplaceKeywords(R"code(
CDataLink::SRxMsg_%msg_name%::SRxMsg_%msg_name%(sdv::core::CDispatchService& rdispatch) :
    m_rdispatch(rdispatch)
{}

bool CDataLink::SRxMsg_%msg_name%::Init()
{
    // Register signals
    [[maybe_unused]] bool bSuccess = true;%sig_register%
    return bSuccess;
}

void CDataLink::SRxMsg_%msg_name%::Term()
{
    // Unregister signals%sig_unregister%
}

void CDataLink::SRxMsg_%msg_name%::Process(const sdv::sequence<uint8_t>& rseqData)
{
    // Check for the correct size.
    if (rseqData.size() != %msg_len%)
    {
        // TODO: Error. Delivered data has different size as compared to the specification.
        return;
    }

    // Helper variable
    [[maybe_unused]] UValueHelper uValueHelper;

    // Start a transaction
    sdv::core::CTransaction transaction = m_rdispatch.CreateTransaction();%process_signals%

    // Finalize the transaction
    transaction.Finish();
}
)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeTxMessageFunctions(const dbc::SMessageDef& rsMsg)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;
    auto prCanId = dbc::CDbcParser::ExtractMsgId(rsMsg.uiId);
    mapKeywords["msg_id"] = to_string(prCanId.first);
    mapKeywords["msg_id_ext"] = prCanId.second ? "true" : "false";
    mapKeywords["msg_len"] = to_string(rsMsg.uiSize);
    std::stringstream sstreamSignalRegister;
    std::stringstream sstreamSignalUnregister;
    std::stringstream sstreamSignalComposition;
    std::stringstream sstreamTriggerInit;
    for (const dbc::SSignalDef& rsSignal : rsMsg.vecSignals)
    {
        sstreamSignalRegister << CodeRegisterTxSignal(rsMsg, rsSignal);
        sstreamSignalUnregister << CodeUnregisterSignal(rsMsg, rsSignal);
        sstreamSignalComposition << CodeComposeTxSignal(rsMsg, rsSignal);
        sstreamTriggerInit << InitTrigger(rsMsg, rsSignal);
    }
    mapKeywords["sig_register"] = std::move(sstreamSignalRegister.str());
    mapKeywords["sig_unregister"] = std::move(sstreamSignalUnregister.str());
    mapKeywords["compose_signals"] = std::move(sstreamSignalComposition.str());

    // DBC files are quite messy keeping track of capital letters. Convert all strings to lower-case.
    auto fnLower = [](const std::string& rss)
    {
        std::string ssRet = rss;
        for (char& rc : ssRet)
            rc = static_cast<char>(std::tolower(rc));
        return ssRet;
    };

    // Determine the trigger attributes
    bool bCyclic = false;
    uint32_t uiCycleTime = 100;
    bool bSpontaneous = false;
    uint32_t uiDelayTime = 0;
    bool bOnlyIfActive = false;
    for (const dbc::SAttributeValue& rsAttribute : rsMsg.vecAttributes)
    {
        if (!rsAttribute.ptrAttrDef) continue;  // No interpretation possible.
        if (rsAttribute.ptrAttrDef->ssName == "GenMsgDelayTime")
        {
            if (rsAttribute.ptrAttrDef->eType == dbc::SAttributeDef::EType::integer)
                uiDelayTime = static_cast<uint32_t>(rsAttribute.iValue);
            if (rsAttribute.ptrAttrDef->eType == dbc::SAttributeDef::EType::hex_integer)
                uiDelayTime = rsAttribute.uiValue;
            if (rsAttribute.ptrAttrDef->eType == dbc::SAttributeDef::EType::floating_point)
                uiDelayTime = static_cast<uint32_t>(rsAttribute.dValue);
        }
        if (rsAttribute.ptrAttrDef->ssName == "GenMsgCycleTime")
        {
            if (rsAttribute.ptrAttrDef->eType == dbc::SAttributeDef::EType::integer)
                uiCycleTime = static_cast<uint32_t>(rsAttribute.iValue);
            if (rsAttribute.ptrAttrDef->eType == dbc::SAttributeDef::EType::hex_integer)
                uiCycleTime = rsAttribute.uiValue;
            if (rsAttribute.ptrAttrDef->eType == dbc::SAttributeDef::EType::floating_point)
                uiCycleTime = static_cast<uint32_t>(rsAttribute.dValue);
        }
        if (rsAttribute.ptrAttrDef->ssName == "GenMsgSendType")
        {
            size_t nValue = 1;   // Representing 'spontaneous'.
            switch (rsAttribute.ptrAttrDef->eType)
            {
            case dbc::SAttributeDef::EType::string:
                if (fnLower(rsAttribute.ssValue) == "cyclic" || fnLower(rsAttribute.ssValue) == "zyklisch")
                    nValue = 0;
                else if (fnLower(rsAttribute.ssValue) == "triggered" || fnLower(rsAttribute.ssValue) == "ereignisgesteuert")
                    nValue = 1;
                else if (fnLower(rsAttribute.ssValue) == "cyclicifactive")
                    nValue = 2;
                else if (fnLower(rsAttribute.ssValue) == "cyclicandtriggered" || fnLower(rsAttribute.ssValue) == "zyklischimmer")
                    nValue = 3;
                else if (fnLower(rsAttribute.ssValue) == "cyclicifactiveandtriggered")
                    nValue = 4;
                else
                    nValue = 5;    // None
                break;
            case dbc::SAttributeDef::EType::enumerator:
                for (size_t nIndex = 0; nIndex < rsAttribute.ptrAttrDef->sEnumValues.vecEnumValues.size(); nIndex++)
                    if (fnLower(rsAttribute.ssValue) == fnLower(rsAttribute.ptrAttrDef->sEnumValues.vecEnumValues[nIndex]))
                        nValue = nIndex;
                break;
            case dbc::SAttributeDef::EType::integer:
                nValue = static_cast<size_t>(rsAttribute.iValue);
                break;
            case dbc::SAttributeDef::EType::hex_integer:
                nValue = static_cast<size_t>(rsAttribute.uiValue);
                break;
            default:
                break;
            }

            // Deal with the value
            switch (nValue)
            {
            case 0: // cyclic
                bCyclic = true;
                break;
            case 2: // cyclicIfActive
                bCyclic = true;
                bOnlyIfActive = true;
                break;
            case 3: // cyclicAndTriggered
                bCyclic = true;
                bSpontaneous = true;
                break;
            case 4: // cyclicIfActiveAndTriggered
                bCyclic = true;
                bSpontaneous = true;
                bOnlyIfActive = true;
                break;
            case 1: // triggered
            default: // all others
                bSpontaneous = true;
                break;
            }
        }
    }

    // Prevent a situation that the object will never be transmitted.
    if (!bCyclic && !bSpontaneous) bSpontaneous = true;

    // CreateTxTrigger definition
    std::stringstream sstreamCreateTrigger;
    sstreamCreateTrigger << "CreateTxTrigger([&] { Transmit(); }, " <<
        (bSpontaneous ? "true, " : "false, ") <<
        uiDelayTime << ", " <<
        ((bCyclic && uiCycleTime) ? uiCycleTime : 0ul) << ", " <<
        ((bCyclic && bOnlyIfActive) ? "true" : "false") <<
        ")";
    mapKeywords["create_trigger"] = std::move(sstreamCreateTrigger.str());
    mapKeywords["init_trigger"] = std::move(sstreamTriggerInit.str());

    return ReplaceKeywords(R"code(
CDataLink::STxMsg_%msg_name%::STxMsg_%msg_name%(sdv::core::CDispatchService& rdispatch) :
    m_rdispatch(rdispatch)
{}

bool CDataLink::STxMsg_%msg_name%::Init(sdv::can::ISend* pSend)
{
    if (!pSend) return false;
    m_pSend = pSend;

    // Register signals
    bool bSuccess = true;%sig_register%

    // Initialize the trigger
    m_trigger = m_rdispatch.%create_trigger%;%init_trigger%
    bSuccess &= m_trigger;

    return bSuccess;
}

void CDataLink::STxMsg_%msg_name%::Term()
{
    // Reset the trigger
    m_trigger.Reset();

    // Unregister signals%sig_unregister%
}

void CDataLink::STxMsg_%msg_name%::Transmit()
{
    if (!m_pSend) return;

    // Compose CAN message
    sdv::can::SMessage msg;
    msg.uiID = %msg_id%;
    msg.bExtended = %msg_id_ext%;
    msg.bCanFd = false;         // TODO: Currently not supported
    msg.seqData.resize(%msg_len%);
    std::fill(msg.seqData.begin(), msg.seqData.end(), static_cast<uint8_t>(0));

    // Helper variable
    [[maybe_unused]] UValueHelper uValueHelper;

    // Start a transaction
    sdv::core::CTransaction transaction = m_rdispatch.CreateTransaction();%compose_signals%

    // Finalize the transaction
    transaction.Finish();

    // Transmit the message
    // TODO: Determine CAN interface index...
    m_pSend->Send(msg, 0);
}
)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeRegisterRxSignal(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;
    mapKeywords["sig_name"] = rsSig.ssName;

    return ReplaceKeywords(R"code(
    m_sig%sig_name% = m_rdispatch.RegisterRxSignal("%msg_name%.%sig_name%");
    bSuccess &= m_sig%sig_name% ? true : false;)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeRegisterTxSignal(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;
    mapKeywords["sig_name"] = rsSig.ssName;

    // Check for a start default value (attribute GeSigStartValue).
    std::string ssDefValue;
    std::for_each(rsSig.vecAttributes.begin(), rsSig.vecAttributes.end(), [&](const dbc::SAttributeValue& rsAttrValue)
        {
            if (!rsAttrValue.ptrAttrDef) return;
            if (rsAttrValue.ptrAttrDef->ssName != "GenSigStartValue") return;
            switch (rsAttrValue.ptrAttrDef->eType)
            {
            case dbc::SAttributeDef::EType::integer:
                if (rsSig.dFactor == std::round(rsSig.dFactor) && rsSig.dOffset == std::round(rsSig.dOffset))
                    ssDefValue = to_string(rsAttrValue.iValue * static_cast<int32_t>(rsSig.dFactor) +
                        static_cast<int32_t>(rsSig.dOffset));
                else
                    ssDefValue = to_string(static_cast<double>(rsAttrValue.iValue) * rsSig.dFactor + rsSig.dOffset);
                break;
            case dbc::SAttributeDef::EType::hex_integer:
                if (rsSig.dFactor == std::round(rsSig.dFactor) && rsSig.dOffset == std::round(rsSig.dOffset))
                    ssDefValue = to_string(rsAttrValue.uiValue * static_cast<uint32_t>(rsSig.dFactor) +
                    static_cast<uint32_t>(rsSig.dOffset));
                else
                    ssDefValue = to_string(static_cast<double>(rsAttrValue.uiValue) * rsSig.dFactor + rsSig.dOffset);
                break;
            case dbc::SAttributeDef::EType::floating_point:
                ssDefValue = to_string(rsAttrValue.dValue * rsSig.dFactor + rsSig.dOffset);
                break;
            case dbc::SAttributeDef::EType::string:
                ssDefValue = std::string("\"") + rsAttrValue.ssValue + "\"";
                break;
            case dbc::SAttributeDef::EType::enumerator:
                for (size_t nIndex = 0; nIndex < rsAttrValue.ptrAttrDef->sEnumValues.vecEnumValues.size(); nIndex++)
                {
                    if (rsAttrValue.ptrAttrDef->sEnumValues.vecEnumValues[nIndex] == rsAttrValue.ssValue)
                    {
                        ssDefValue = to_string(nIndex);
                        break;
                    }
                    if (rsAttrValue.ptrAttrDef->sEnumValues.vecEnumValues[nIndex] == rsAttrValue.ptrAttrDef->sEnumValues.ssDefault)
                        ssDefValue = to_string(nIndex);
                }
                break;
            default:
                break;
            }
        });
    mapKeywords["def_value"] = ssDefValue.empty() ? "sdv::any_t()" : ssDefValue;

    return ReplaceKeywords(R"code(
    m_sig%sig_name% = m_rdispatch.RegisterTxSignal("%msg_name%.%sig_name%", %def_value%);
    bSuccess &= m_sig%sig_name% ? true : false;)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeUnregisterSignal(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;
    mapKeywords["sig_name"] = rsSig.ssName;

    return ReplaceKeywords(R"code(
    m_sig%sig_name%.Reset();)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeProcessRxSignal(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;
    mapKeywords["sig_name"] = rsSig.ssName;
    auto prSignalTypeDef = m_rparser.GetSignalTypeDef(rsSig.ssSignalTypeDef);
    const dbc::SSignalTypeBase& rsSigType = prSignalTypeDef.second ?
        static_cast<const dbc::SSignalTypeBase&>(prSignalTypeDef.first) :
        static_cast<const dbc::SSignalTypeBase&>(rsSig);

    // Signals with zero length do not get processed
    if (!rsSigType.uiSize) return std::string();
    mapKeywords["sig_size"] = to_string(rsSigType.uiSize);

    // Check for correct size and store the default value
    switch (rsSigType.eValType)
    {
    case dbc::SSignalDef::EValueType::signed_integer:
        if (rsSigType.uiSize > 64) return std::string();   // Invalid size
        if (rsSigType.uiSize <= 1) return std::string();   // Invalid size
        mapKeywords["default_value"] = to_string(prSignalTypeDef.second ? static_cast<int64_t>(prSignalTypeDef.first.dDefaultValue) : 0l);
        mapKeywords["sign_bit"] = to_string(rsSigType.uiSize - 1);
        break;
    case dbc::SSignalDef::EValueType::unsigned_integer:
        if (rsSigType.uiSize > 64) return std::string();   // Invalid size
        mapKeywords["default_value"] = to_string(prSignalTypeDef.second ? static_cast<uint32_t>(prSignalTypeDef.first.dDefaultValue) : 0ul) + "u";
        break;
    case dbc::SSignalDef::EValueType::ieee_float:
        if (rsSigType.uiSize != 32) return std::string();   // Invalid size
        mapKeywords["default_value"] = to_string(prSignalTypeDef.second ? static_cast<float>(prSignalTypeDef.first.dDefaultValue) : 0.0f);
        break;
    case dbc::SSignalDef::EValueType::ieee_double:
        if (rsSigType.uiSize != 64) return std::string();   // Invalid size
        mapKeywords["default_value"] = to_string(prSignalTypeDef.second ? prSignalTypeDef.first.dDefaultValue : 0.0);
        break;
    default:
        return std::string(); // Invalid type
    }

    // Create the algorithm stream
    std::stringstream sstreamAlgorithm;
    sstreamAlgorithm << R"code(

    // Process %msg_name%.%sig_name%)code";

    // Dependable on the endianness, the signal conversion differs.
    if (rsSig.eByteOrder == dbc::SSignalDef::EByteOrder::big_endian)
    {
        // Helper function to inverse the bit position (which is mirrored for the Motorola format).
        auto fnInverseBitPos = [](uint32_t uiPos) -> uint32_t
        {
            uint32_t uiInverseStartBit = (uiPos >> 3) << 3;
            uint32_t uiInverseStartBitInByte = (8 - ((uiPos + 1) & 7)) & 7;
            return uiInverseStartBit + uiInverseStartBitInByte;
        };

        // Helper function to determine the first byte that contains the value.
        auto fnFirstByte = [&]()
        {
            return rsSig.uiStartBit >> 3;
        };
        mapKeywords["first_byte"] = to_string(fnFirstByte());

        // Helper function to determine the last byte that contains the value.
        auto fnLastByte = [&]()
        {
            return ((fnInverseBitPos(rsSig.uiStartBit) + rsSigType.uiSize - 1)) >> 3;
        };
        mapKeywords["last_byte"] = to_string(fnLastByte());

        // Helper function to determine the mask identifying the start of the value.
        auto fnFirstByteMask = [&]()
        {
            return (1 << ((rsSig.uiStartBit & 0x7) + 1)) - 1;
        };
        mapKeywords["first_byte_mask"] = to_string(fnFirstByteMask());

        // Helper function to determine the amount of bits the value needs to shift to the right to reach into focus.
        auto fnShiftRight = [&]()
        {
            return (fnInverseBitPos(fnInverseBitPos(rsSig.uiStartBit) + rsSigType.uiSize) + 1) & 0x7;
        };
        mapKeywords["shift_right"] = to_string(fnShiftRight());

        // Add all the bytes
        for (size_t nSrcIndex = fnFirstByte(); nSrcIndex <= fnLastByte(); nSrcIndex++)
        {
            if (nSrcIndex == fnFirstByte())
            {
                sstreamAlgorithm << R"code(
    uValueHelper.uiUint64Value = rseqData[%first_byte%])code";
                if (fnFirstByteMask() != 0xff)
                    sstreamAlgorithm << R"code( & %first_byte_mask%)code";
                sstreamAlgorithm << ";";
            }
            else
                sstreamAlgorithm << R"code(
    uValueHelper.uiUint64Value = uValueHelper.uiUint64Value << 8 | rseqData[)code" << nSrcIndex << R"code(];)code";
            if (nSrcIndex == fnLastByte() && fnShiftRight())
                sstreamAlgorithm << R"code(
    uValueHelper.uiUint64Value >>= %shift_right%;)code";
        }
    }
    else
    {
        // Helper function to determine the first byte that contains the value.
        auto fnFirstByte = [&]()
        {
            return rsSig.uiStartBit >> 3;
        };
        mapKeywords["first_byte"] = to_string(fnFirstByte());

        // Helper function to determine the last byte that contains the value.
        auto fnLastByte = [&]()
        {
            return (rsSig.uiStartBit + rsSigType.uiSize - 1) >> 3;
        };
        mapKeywords["last_byte"] = to_string(fnLastByte());

        // Helper function to determine the mask identifying the end of the value.
        auto fnLastByteMask = [&]()
        {
            return (1 << (((rsSig.uiStartBit + rsSigType.uiSize - 1) & 0x7) + 1)) - 1;
        };
        mapKeywords["last_byte_mask"] = to_string(fnLastByteMask());

        // Helper function to determine the amount of bits the value needs to shift to the right to reach into focus.
        auto fnShiftRight = [&]()
        {
            return rsSig.uiStartBit & 0x7;
        };
        mapKeywords["shift_right"] = to_string(fnShiftRight());

        // Add all the bytes
        for (size_t nSrcIndex = fnLastByte(); nSrcIndex >= fnFirstByte() && nSrcIndex <= fnLastByte(); nSrcIndex--)
        {
            if (nSrcIndex == fnLastByte())
            {
                sstreamAlgorithm << R"code(
    uValueHelper.uiUint64Value = rseqData[%last_byte%])code";
                if (fnLastByteMask() != 0xff)
                    sstreamAlgorithm << R"code( & %last_byte_mask%)code";
                sstreamAlgorithm << ";";
            }
            else
                sstreamAlgorithm << R"code(
    uValueHelper.uiUint64Value = uValueHelper.uiUint64Value << 8 | rseqData[)code" << nSrcIndex << R"code(];)code";
            if (nSrcIndex == fnFirstByte() && fnShiftRight())
                sstreamAlgorithm << R"code(
    uValueHelper.uiUint64Value >>= %shift_right%;)code";
        }
    }

    // Value to hex-string (64-bit)
    auto fnToHexString = [](uint64_t uiValue) -> std::string
    {
        std::stringstream sstream;
        sstream << "0x" << std::setfill('0') << std::setw(16) << std::hex << uiValue << "ull";
        return sstream.str();
    };

    switch (rsSigType.eValType)
    {
    case dbc::SSignalDef::EValueType::signed_integer:
        // MSB bit determines the sign.
        if (rsSigType.uiSize != 64)
        {
            // If the provided bit is '1', fill the more significant bits with 1 as well.
            mapKeywords["msb_sign"] = fnToHexString(((1ull << (64ull - rsSigType.uiSize)) - 1) << rsSigType.uiSize);
            sstreamAlgorithm << R"code(
    if (uValueHelper.uiUint64Value & (0x1ull << %sign_bit%ull)) // Extend sign to 64 bit.
        uValueHelper.uiUint64Value |= %msb_sign%;)code";
        }
        if (std::round(rsSigType.dFactor) != rsSigType.dFactor ||
            std::round(rsSigType.dOffset) != rsSigType.dOffset)
        {
            mapKeywords["sig_factor"] = to_string(rsSigType.dFactor);
            mapKeywords["sig_offset"] = to_string(rsSigType.dOffset);
            mapKeywords["sig_min"] = to_string(rsSigType.dMinimum);
            mapKeywords["sig_max"] = to_string(rsSigType.dMaximum);
            if (rsSigType.uiSize > 32)
                mapKeywords["sig_helper_value"] = "static_cast<double>(uValueHelper.iInt64Value)";
            else
                mapKeywords["sig_helper_value"] = "static_cast<double>(uValueHelper.s32.u32.iValue)";
        }
        else
        {
            mapKeywords["sig_factor"] = to_string(static_cast<int32_t>(rsSigType.dFactor));
            mapKeywords["sig_offset"] = to_string(static_cast<int32_t>(rsSigType.dOffset));
            mapKeywords["sig_min"] = to_string(static_cast<int32_t>(rsSigType.dMinimum));
            mapKeywords["sig_max"] = to_string(static_cast<int32_t>(rsSigType.dMaximum));
            if (rsSigType.uiSize > 32)
                mapKeywords["sig_helper_value"] = "uValueHelper.iInt64Value";
            else
                mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.iValue";
        }
        break;
    case dbc::SSignalDef::EValueType::unsigned_integer:
        if (std::round(rsSigType.dFactor) != rsSigType.dFactor ||
            std::round(rsSigType.dOffset) != rsSigType.dOffset)
        {
            mapKeywords["sig_factor"] = to_string(rsSigType.dFactor);
            mapKeywords["sig_offset"] = to_string(rsSigType.dOffset);
            mapKeywords["sig_min"] = to_string(rsSigType.dMinimum);
            if (rsSigType.uiSize > 32)
            {
                mapKeywords["sig_helper_value"] = "static_cast<double>(uValueHelper.uiUint64Value)";
                mapKeywords["sig_max"] = to_string(rsSigType.dMaximum);
            }
            else
            {
                mapKeywords["sig_helper_value"] = "static_cast<double>(uValueHelper.s32.u32.uiValue)";
                mapKeywords["sig_max"] = StringMustBeFloatValue(to_string(rsSigType.dMaximum));
            }
        }
        else
        {
            if (rsSigType.dFactor < 0.0 || rsSigType.dOffset < 0.0)
            {
                mapKeywords["sig_factor"] = to_string(static_cast<int64_t>(rsSigType.dFactor)) + "ll";
                mapKeywords["sig_offset"] = to_string(static_cast<int64_t>(rsSigType.dOffset)) + "ll";
                mapKeywords["sig_min"] = to_string(static_cast<int64_t>(rsSigType.dMinimum)) + "ll";
                mapKeywords["sig_max"] = to_string(static_cast<int64_t>(rsSigType.dMaximum)) + "ll";
                if (rsSigType.uiSize > 32)
                    mapKeywords["sig_helper_value"] = "static_cast<int64_t>(uValueHelper.uiUint64Value)";
                else
                    mapKeywords["sig_helper_value"] = "static_cast<int64_t>(uValueHelper.s32.u32.uiValue)";
            }
            else
            {
                mapKeywords["sig_factor"] = to_string(static_cast<uint32_t>(rsSigType.dFactor)) + "u";
                mapKeywords["sig_offset"] = to_string(static_cast<uint32_t>(rsSigType.dOffset)) + "u";
                mapKeywords["sig_min"] = to_string(static_cast<uint32_t>(std::max(rsSigType.dMinimum, 0.0))) + "u";
                mapKeywords["sig_max"] = to_string(static_cast<uint32_t>(std::max(rsSigType.dMaximum, 0.0))) + "u";
                if (rsSigType.uiSize > 32)
                    mapKeywords["sig_helper_value"] = "uValueHelper.uiUint64Value";
                else
                    mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.uiValue";
            }
        }
        break;
    case dbc::SSignalDef::EValueType::ieee_float:
        if (rsSigType.uiSize != 32) return std::string();   // Invalid size
        sstreamAlgorithm << R"code(
    if ((uValueHelper.s32.u32.uiValue & 0x7f800000) == 0x7f800000) // NaN
        uValueHelper.s32.u32.fValue = %default_value%;)code";
        mapKeywords["sig_factor"] = to_string(static_cast<float>(rsSigType.dFactor));
        mapKeywords["sig_offset"] = to_string(static_cast<float>(rsSigType.dOffset));
        mapKeywords["sig_min"] = to_string(static_cast<float>(rsSigType.dMinimum));
        mapKeywords["sig_max"] = to_string(static_cast<float>(rsSigType.dMaximum));
        mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.fValue";
        break;
    case dbc::SSignalDef::EValueType::ieee_double:
        if (rsSigType.uiSize != 64) return std::string();   // Invalid size
        sstreamAlgorithm << R"code(
    if ((uValueHelper.uiUint64Value & 0x7ff0000000000000ull) == 0x7ff0000000000000ull) // NaN
        uValueHelper.dValue = %default_value%;)code";
        mapKeywords["sig_factor"] = to_string(rsSigType.dFactor);
        mapKeywords["sig_offset"] = to_string(rsSigType.dOffset);
        mapKeywords["sig_min"] = to_string(rsSigType.dMinimum);
        mapKeywords["sig_max"] = to_string(rsSigType.dMaximum);
        mapKeywords["sig_helper_value"] = "uValueHelper.dValue";
        break;
    default:
        return std::string();
        break;
    }

    // Generate the code for the conversion from logical to physical and assign the value to the signal.
    sstreamAlgorithm << R"code(
    m_sig%sig_name%.Write()code";
    if (rsSigType.dMinimum != rsSigType.dMaximum)
        sstreamAlgorithm << "std::min(std::max(";
    sstreamAlgorithm << "%sig_helper_value%";
    if (rsSigType.dFactor != 1.0)
        sstreamAlgorithm << " * %sig_factor%";
    if (rsSigType.dOffset != 0.0)
        sstreamAlgorithm << " + %sig_offset%";
    if (rsSigType.dMinimum != rsSigType.dMaximum)
        sstreamAlgorithm << ", %sig_min%), %sig_max%)";
    sstreamAlgorithm << ", transaction);";

    return ReplaceKeywords(sstreamAlgorithm.str(), mapKeywords);
}

std::string CCanDataLinkGen::InitTrigger(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;
    mapKeywords["sig_name"] = rsSig.ssName;

    return ReplaceKeywords(R"code(
    m_trigger.AddSignal(m_sig%sig_name%);)code", mapKeywords);
}

std::string CCanDataLinkGen::CodeComposeTxSignal(const dbc::SMessageDef& rsMsg, const dbc::SSignalDef& rsSig)
{
    CKeywordMap mapKeywords;
    mapKeywords["msg_name"] = rsMsg.ssName;
    mapKeywords["sig_name"] = rsSig.ssName;
    auto prSignalTypeDef = m_rparser.GetSignalTypeDef(rsSig.ssSignalTypeDef);
    const dbc::SSignalTypeBase& rsSigType = prSignalTypeDef.second ?
        static_cast<const dbc::SSignalTypeBase&>(prSignalTypeDef.first) :
        static_cast<const dbc::SSignalTypeBase&>(rsSig);

    // Signals with zero length do not get processed
    if (!rsSigType.uiSize) return std::string();
    mapKeywords["sig_size"] = to_string(rsSigType.uiSize);

    // Check for correct size and store the default value
    switch (rsSigType.eValType)
    {
    case dbc::SSignalDef::EValueType::signed_integer:
        if (rsSigType.uiSize > 64) return std::string();   // Invalid size
        if (rsSigType.uiSize <= 1) return std::string();   // Invalid size
        mapKeywords["default_value"] = to_string(prSignalTypeDef.second ? static_cast<int64_t>(prSignalTypeDef.first.dDefaultValue) : 0l);
        mapKeywords["sign_bit"] = to_string(rsSigType.uiSize - 1);
        break;
    case dbc::SSignalDef::EValueType::unsigned_integer:
        if (rsSigType.uiSize > 64) return std::string();   // Invalid size
        mapKeywords["default_value"] = to_string(prSignalTypeDef.second ? static_cast<uint64_t>(prSignalTypeDef.first.dDefaultValue) : 0ul) + "u";
        break;
    case dbc::SSignalDef::EValueType::ieee_float:
        if (rsSigType.uiSize != 32) return std::string();   // Invalid size
        mapKeywords["default_value"] = to_string(prSignalTypeDef.second ? static_cast<float>(prSignalTypeDef.first.dDefaultValue) : 0.0f);
        break;
    case dbc::SSignalDef::EValueType::ieee_double:
        if (rsSigType.uiSize != 64) return std::string();   // Invalid size
        mapKeywords["default_value"] = to_string(prSignalTypeDef.second ? prSignalTypeDef.first.dDefaultValue : 0.0);
        break;
    default:
        return std::string(); // Invalid type
    }

    // Create the algorithm stream
    std::stringstream sstreamAlgorithm;
    sstreamAlgorithm << R"code(

    // Compose %msg_name%.%sig_name%)code";

    bool bHelperCast = false;
    bool bRound = false;
    switch (rsSigType.eValType)
    {
    case dbc::SSignalDef::EValueType::signed_integer:
        if (std::round(rsSigType.dFactor) != rsSigType.dFactor ||
            std::round(rsSigType.dOffset) != rsSigType.dOffset)
        {
            mapKeywords["sig_factor"] = to_string(rsSigType.dFactor);
            mapKeywords["sig_offset"] = to_string(rsSigType.dOffset);
            mapKeywords["sig_min"] = to_string(rsSigType.dMinimum);
            mapKeywords["sig_max"] = to_string(rsSigType.dMaximum);
            mapKeywords["sig_physical_cast"] = "double";
            if (rsSigType.uiSize > 32)
            {
                mapKeywords["sig_helper_value"] = "uValueHelper.iInt64Value";
                mapKeywords["sig_helper_cast"] = "int64_t";
            }
            else
            {
                mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.iValue";
                mapKeywords["sig_helper_cast"] = "int32_t";
            }
            bHelperCast = true;
            bRound = true;
        }
        else
        {
            mapKeywords["sig_factor"] = to_string(static_cast<int64_t>(rsSigType.dFactor)) + "ll";
            mapKeywords["sig_offset"] = to_string(static_cast<int64_t>(rsSigType.dOffset)) + "ll";
            mapKeywords["sig_min"] = to_string(static_cast<int64_t>(rsSigType.dMinimum)) + "ll";
            mapKeywords["sig_max"] = to_string(static_cast<int64_t>(rsSigType.dMaximum)) + "ll";
            mapKeywords["sig_physical_cast"] = "long long int";
            if (rsSigType.uiSize > 32)
            {
                mapKeywords["sig_helper_value"] = "uValueHelper.iInt64Value";
                mapKeywords["sig_helper_cast"] = "int64_t";
            }
            else
            {
                mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.iValue";
                mapKeywords["sig_helper_cast"] = "int32_t";
            }
            bHelperCast = true;
        }
        break;
    case dbc::SSignalDef::EValueType::unsigned_integer:
        if (std::round(rsSigType.dFactor) != rsSigType.dFactor ||
            std::round(rsSigType.dOffset) != rsSigType.dOffset)
        {
            mapKeywords["sig_factor"] = to_string(rsSigType.dFactor);
            mapKeywords["sig_offset"] = to_string(rsSigType.dOffset);
            mapKeywords["sig_min"] = to_string(rsSigType.dMinimum);
            mapKeywords["sig_max"] = to_string(rsSigType.dMaximum);
            mapKeywords["sig_physical_cast"] = "double";
            if (rsSigType.uiSize > 32)
            {
                mapKeywords["sig_helper_value"] = "uValueHelper.uiUint64Value";
                mapKeywords["sig_helper_cast"] = "uint64_t";
            }
            else
            {
                mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.uiValue";
                mapKeywords["sig_helper_cast"] = "uint32_t";
            }
            bHelperCast = true;
            bRound = true;
        }
        else
        {
            mapKeywords["sig_factor"] = to_string(static_cast<int64_t>(rsSigType.dFactor)) + "ll";
            mapKeywords["sig_offset"] = to_string(static_cast<int64_t>(rsSigType.dOffset)) + "ll";
            mapKeywords["sig_min"] = to_string(static_cast<int64_t>(rsSigType.dMinimum)) + "ll";
            mapKeywords["sig_max"] = to_string(static_cast<int64_t>(rsSigType.dMaximum)) + "ll";
            mapKeywords["sig_physical_cast"] = "long long int";
            if (rsSigType.uiSize > 32)
            {
                mapKeywords["sig_helper_value"] = "uValueHelper.uiUint64Value";
                mapKeywords["sig_helper_cast"] = "uint64_t";
            }
            else
            {
                mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.uiValue";
                mapKeywords["sig_helper_cast"] = "uint32_t";
            }
            bHelperCast = true;
        }
        break;
    case dbc::SSignalDef::EValueType::ieee_float:
        if (rsSigType.uiSize != 32) return std::string();   // Invalid size
        mapKeywords["sig_factor"] = to_string(static_cast<float>(rsSigType.dFactor));
        mapKeywords["sig_offset"] = to_string(static_cast<float>(rsSigType.dOffset));
        mapKeywords["sig_min"] = to_string(static_cast<float>(rsSigType.dMinimum));
        mapKeywords["sig_max"] = to_string(static_cast<float>(rsSigType.dMaximum));
        mapKeywords["sig_physical_cast"] = "float";
        mapKeywords["sig_helper_value"] = "uValueHelper.s32.u32.fValue";
        break;
    case dbc::SSignalDef::EValueType::ieee_double:
        if (rsSigType.uiSize != 64) return std::string();   // Invalid size
        mapKeywords["sig_factor"] = to_string(rsSigType.dFactor);
        mapKeywords["sig_offset"] = to_string(rsSigType.dOffset);
        mapKeywords["sig_min"] = to_string(rsSigType.dMinimum);
        mapKeywords["sig_max"] = to_string(rsSigType.dMaximum);
        mapKeywords["sig_physical_cast"] = "double";
        mapKeywords["sig_helper_value"] = "uValueHelper.dValue";
        break;
    default:
        return std::string();
        break;
    }

    // Generate the code for the conversion from physical to logical.
    sstreamAlgorithm << R"code(
    %sig_helper_value% = )code";
    if (bHelperCast)
        sstreamAlgorithm << "static_cast<%sig_helper_cast%>(";
    if (bRound)
        sstreamAlgorithm << "std::round(";
    if (rsSigType.dOffset != 0.0)
        sstreamAlgorithm << "(";
    if (rsSigType.dMinimum != rsSigType.dMaximum)
        sstreamAlgorithm << "std::min(std::max(";
    sstreamAlgorithm << "static_cast<%sig_physical_cast%>(m_sig%sig_name%.Read(transaction))";
    if (rsSigType.dMinimum != rsSigType.dMaximum)
        sstreamAlgorithm << ", %sig_min%), %sig_max%)";
    if (rsSigType.dOffset != 0.0)
        sstreamAlgorithm << " - %sig_offset%)";
    if (rsSigType.dFactor != 1.0)
        sstreamAlgorithm << " / %sig_factor%";
    if (bRound)
        sstreamAlgorithm << ")";
    if (bHelperCast)
        sstreamAlgorithm << ")";
    sstreamAlgorithm << ";";

    // Dependable on the endianness, the signal conversion differs.
    if (rsSig.eByteOrder == dbc::SSignalDef::EByteOrder::big_endian)
    {
        // Helper function to inverse the bit position (which is mirrored for the Motorola format).
        auto fnInverseBitPos = [](uint32_t uiPos) -> uint32_t
        {
            uint32_t uiInverseStartBit = (uiPos >> 3) << 3;
            uint32_t uiInverseStartBitInByte = (8 - ((uiPos + 1) & 7)) & 7;
            return uiInverseStartBit + uiInverseStartBitInByte;
        };

        // Helper function to determine the first byte that contains the value.
        auto fnFirstByte = [&]()
        {
            return rsSig.uiStartBit >> 3;
        };
        mapKeywords["first_byte"] = to_string(fnFirstByte());

        // Helper function to determine the last byte that contains the value.
        auto fnLastByte = [&]()
        {
            return ((fnInverseBitPos(rsSig.uiStartBit) + rsSigType.uiSize - 1)) >> 3;
        };
        mapKeywords["last_byte"] = to_string(fnLastByte());

        // Helper function to determine the mask identifying the start of the value.
        auto fnFirstByteMask = [&]()
        {
            return (1 << ((rsSig.uiStartBit & 0x7) + 1)) - 1;
        };
        mapKeywords["first_byte_mask"] = to_string(fnFirstByteMask());

        // Helper function to determine the amount of bits the value needs to shift to the left to reach the right bit-position.
        auto fnShiftLeft = [&]()
        {
            return (fnInverseBitPos(fnInverseBitPos(rsSig.uiStartBit) + rsSigType.uiSize) + 1) & 0x7;
        };
        mapKeywords["shift_left"] = to_string(fnShiftLeft());

        // Add all the bytes
        for (size_t nSrcIndex = fnLastByte(); nSrcIndex >= fnFirstByte() && nSrcIndex <= fnLastByte(); nSrcIndex--)
        {
            if (nSrcIndex == fnLastByte() && fnShiftLeft())
                sstreamAlgorithm << R"code(
    uValueHelper.uiUint64Value <<= %shift_left%;)code";
            if (nSrcIndex == fnFirstByte())
            {
                sstreamAlgorithm << R"code(
    msg.seqData[%first_byte%] |= uValueHelper.uiUint64Value & )code";
                if (fnFirstByteMask() != 0xff)
                    sstreamAlgorithm << "%first_byte_mask%";
                else
                    sstreamAlgorithm << "0xff";
                sstreamAlgorithm << ";";
            }
            else
                sstreamAlgorithm << R"code(
    msg.seqData[)code" << nSrcIndex << R"code(] |= uValueHelper.uiUint64Value & 0xff;
    uValueHelper.uiUint64Value >>= 8;)code";
        }
    }
    else
    {
        // Helper function to determine the first byte that contains the value.
        auto fnFirstByte = [&]()
        {
            return rsSig.uiStartBit >> 3;
        };
        mapKeywords["first_byte"] = to_string(fnFirstByte());

        // Helper function to determine the last byte that contains the value.
        auto fnLastByte = [&]()
        {
            return (rsSig.uiStartBit + rsSigType.uiSize - 1) >> 3;
        };
        mapKeywords["last_byte"] = to_string(fnLastByte());

        // Helper function to determine the mask identifying the end of the value.
        auto fnLastByteMask = [&]()
        {
            return (1 << (((rsSig.uiStartBit + rsSigType.uiSize - 1) & 0x7) + 1)) - 1;
        };
        mapKeywords["last_byte_mask"] = to_string(fnLastByteMask());

        // Helper function to determine the amount of bits the value needs to shift to the left to reach the right bit-position.
        auto fnShiftLeft = [&]()
        {
            return rsSig.uiStartBit & 0x7;
        };
        mapKeywords["shift_left"] = to_string(fnShiftLeft());

        // Add all the bytes
        for (size_t nSrcIndex = fnFirstByte(); nSrcIndex <= fnLastByte(); nSrcIndex++)
        {
            if (nSrcIndex == fnFirstByte() && fnShiftLeft())
                sstreamAlgorithm << R"code(
    uValueHelper.uiUint64Value <<= %shift_left%;)code";
            if (nSrcIndex == fnLastByte())
            {
                sstreamAlgorithm << R"code(
    msg.seqData[%last_byte%] |= uValueHelper.uiUint64Value & )code";
                if (fnLastByteMask() != 0xff)
                    sstreamAlgorithm << "%last_byte_mask%";
                else
                    sstreamAlgorithm << "0xff";
                sstreamAlgorithm << ";";
            }
            else
                sstreamAlgorithm << R"code(
    msg.seqData[)code" << nSrcIndex << R"code(] |= uValueHelper.uiUint64Value & 0xff;
    uValueHelper.uiUint64Value >>= 8;)code";
        }
    }

    return ReplaceKeywords(sstreamAlgorithm.str(), mapKeywords);
}

std::string CCanDataLinkGen::CodeDBCFileVersion(const std::string& rsVersion)
{
    if (rsVersion.empty())
    {
        return "";
    }

    CKeywordMap mapKeywords;
    mapKeywords["dbc_version"] = rsVersion;

    return ReplaceKeywords(R"code(DBC file version: %dbc_version%)code", mapKeywords);
}

std::string CCanDataLinkGen::StringMustBeFloatValue(const std::string& rssInput)
{
    if (rssInput.find_first_of(".") != std::string::npos)
    {
        return rssInput;
    }

    return rssInput + ".0";
}

