#include "dbcparser.h"
#include <fstream>
#include <algorithm>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef _MSC_VER
    // Prevent warnings about uninitialized members within unions during static code analysis.
    #pragma warning(push)
    #pragma warning(disable : 26495)
#endif

/**
 * @brief DBC namespace
 */
namespace dbc
{
    /**
     * @brief Internal namespace
     */
    namespace internal
    {
        /**
         * @brief Check for equality, ignoring the case.
         * @param[in] rss1 Reference to the first string.
         * @param[in] rss2 Reference to the second string.
         * @return The result of the comparison.
        */
        inline bool EqualNC(const std::string& rss1, const std::string& rss2)
        {
            if (rss1.length() != rss2.length()) return false;
            for (size_t n = 0; n < rss1.length(); n++)
                if (std::tolower(rss1[n]) != std::tolower(rss2[n])) return false;
            return true;
        }
    }

    CDbcSource::CDbcSource(const std::filesystem::path& rpathDbcfile) : m_pathFile(rpathDbcfile)
    {
        std::ifstream stream;
        stream.open(rpathDbcfile);
        if (!stream.is_open()) throw SDbcParserException("Failed to open the %1 file for reading.", rpathDbcfile);

        // Read the complete source
        std::stringstream sstream;
        sstream << stream.rdbuf();
        m_ssContent = std::move(sstream.str());
    }

    CDbcSource::CDbcSource(const std::string& rssContent) : m_ssContent(rssContent)
    {}

    bool CDbcSource::IsValid() const
    {
        return !m_ssContent.empty();
    }

    const std::filesystem::path& CDbcSource::Path() const
    {
        return m_pathFile;
    }

    const std::string& CDbcSource::Content() const
    {
        return m_ssContent;
    }

    const char* CDbcSource::ContentPtr() const
    {
        return m_ssContent.c_str() + std::min(m_nPos, m_ssContent.size());
    }

    char CDbcSource::CurrentChar() const
    {
        return IsEOF() ? '\0' : m_ssContent[m_nPos];
    }

    const size_t& CDbcSource::Pos() const
    {
        return m_nPos;
    }

    size_t& CDbcSource::Pos()
    {
        return m_nPos;
    }

    void CDbcSource::ResetPos()
    {
        m_nPos = 0;
    }

    bool CDbcSource::IsEOF() const
    {
        return m_nPos >= m_ssContent.size();
    }

    CDbcSource::SPosLock::SPosLock(CDbcSource& rThis) :
        pSource(&rThis), nStoredPos(rThis.Pos())
    {}

    CDbcSource::SPosLock::SPosLock(SPosLock&& rsPosLock) noexcept :
        pSource(rsPosLock.pSource), nStoredPos(rsPosLock.nStoredPos)
    {
        rsPosLock.pSource = nullptr;
        rsPosLock.nStoredPos = 0;
    }

    CDbcSource::SPosLock::~SPosLock()
    {
        // Automatically rollback
        Rollback();
    }

    CDbcSource::SPosLock& CDbcSource::SPosLock::operator=(SPosLock&& rsPosLock) noexcept
    {
        pSource = rsPosLock.pSource;
        nStoredPos = rsPosLock.nStoredPos;
        rsPosLock.pSource = nullptr;
        rsPosLock.nStoredPos = 0;
        return *this;
    }

    void CDbcSource::SPosLock::Promote()
    {
        // Invalidate. Do not rollback to the stored position.
        pSource = nullptr;
        nStoredPos = 0;
    }

    void CDbcSource::SPosLock::Rollback()
    {
        // Can rollback?
        if (pSource) pSource->Pos() = nStoredPos;

        // Invalidate.
        pSource = nullptr;
        nStoredPos = 0;
    }

    CDbcSource::SPosLock CDbcSource::CreatePosLock()
    {
        return SPosLock(*this);
    }

    size_t CDbcSource::CalcLine() const
    {
        size_t nLine = m_ssContent.empty() ? 0 : 1;
        nLine += std::count(m_ssContent.begin(), IsEOF() ? m_ssContent.end() : m_ssContent.begin() + m_nPos, '\n');
        return nLine;
    }

    size_t CDbcSource::CalcColumn() const
    {
        size_t nBOL = m_nPos ? m_ssContent.find_last_of('\n', m_nPos - 1) : std::string::npos;
        if (nBOL == std::string::npos)
            nBOL = 0;
        else
            nBOL++;
        size_t nCol = 0;
        for (size_t nIndex = nBOL; nIndex < std::min(m_nPos, m_ssContent.size()); nIndex++)
        {
            if (m_ssContent[nIndex] == '\t')
            {
                // Increase until nIndex is at the tab position.
                nCol += 4 - nCol % 4;
            }
            else
                nCol++;
        }

        return nCol + 1;
    }

    SAttributeDef::SAttributeDef(SAttributeDef::EType eTypeParam) : eType(eTypeParam)
    {
        switch (eType)
        {
        case EType::integer:
            sIntValues.iMinimum = 0;
            sIntValues.iMaximum = 0;
            sIntValues.iDefault = 0;
            break;
        case EType::hex_integer:
            sHexValues.uiMinimum = 0u;
            sHexValues.uiMaximum = 0u;
            sHexValues.uiDefault = 0u;
            break;
        case EType::floating_point:
            sFltValues.dMinimum = 0.0;
            sFltValues.dMaximum = 0.0;
            sFltValues.dDefault = 0.0;
            break;
        case EType::string:
            new (&sStringValues.ssDefault) std::string;
            break;
        case EType::enumerator:
            new (&sEnumValues.vecEnumValues) std::vector<std::string>;
            new (&sEnumValues.ssDefault) std::string;
            break;
        default:
            throw SDbcParserException("Internal error: invalid attribute definition type.");
            break;
        }
    }

    SAttributeDef::~SAttributeDef()
    {
        switch (eType)
        {
        case EType::string:
            sStringValues.ssDefault.~basic_string();
            break;
        case EType::enumerator:
            sEnumValues.vecEnumValues.~vector();
            sEnumValues.ssDefault.~basic_string();
            break;
        default:
            break;
        }
    }

    SAttributeDef::SAttributeDef(const SAttributeDef& rAttrDef) : eObjType(rAttrDef.eObjType), ssName(rAttrDef.ssName), eType(rAttrDef.eType)
    {
        switch (eType)
        {
        case EType::integer:
            sIntValues.iMinimum = rAttrDef.sIntValues.iMinimum;
            sIntValues.iMaximum = rAttrDef.sIntValues.iMaximum;
            sIntValues.iDefault = rAttrDef.sIntValues.iDefault;
            break;
        case EType::hex_integer:
            sHexValues.uiMinimum = rAttrDef.sHexValues.uiMinimum;
            sHexValues.uiMaximum = rAttrDef.sHexValues.uiMaximum;
            sHexValues.uiDefault = rAttrDef.sHexValues.uiDefault;
            break;
        case EType::floating_point:
            sFltValues.dMinimum = rAttrDef.sFltValues.dMinimum;
            sFltValues.dMaximum = rAttrDef.sFltValues.dMaximum;
            sFltValues.dDefault = rAttrDef.sFltValues.dDefault;
            break;
        case EType::string:
            new (&sStringValues.ssDefault) std::string(rAttrDef.sStringValues.ssDefault);
            break;
        case EType::enumerator:
            new (&sEnumValues.vecEnumValues) std::vector<std::string>(rAttrDef.sEnumValues.vecEnumValues);
            new (&sEnumValues.ssDefault) std::string(rAttrDef.sEnumValues.ssDefault);
            break;
        default:
            throw SDbcParserException("Internal error: invalid attribute definition type.");
            break;
        }
    }

    SAttributeDef::SAttributeDef(SAttributeDef&& rAttrDef) : eObjType(rAttrDef.eObjType), ssName(std::move(rAttrDef.ssName)), eType(rAttrDef.eType)
    {
        switch (eType)
        {
        case EType::integer:
            sIntValues.iMinimum = rAttrDef.sIntValues.iMinimum;
            sIntValues.iMaximum = rAttrDef.sIntValues.iMaximum;
            sIntValues.iDefault = rAttrDef.sIntValues.iDefault;
            rAttrDef.sIntValues.iMinimum = 0;
            rAttrDef.sIntValues.iMaximum = 0;
            rAttrDef.sIntValues.iDefault = 0;
            break;
        case EType::hex_integer:
            sHexValues.uiMinimum = rAttrDef.sHexValues.uiMinimum;
            sHexValues.uiMaximum = rAttrDef.sHexValues.uiMaximum;
            sHexValues.uiDefault = rAttrDef.sHexValues.uiDefault;
            rAttrDef.sHexValues.uiMinimum = 0u;
            rAttrDef.sHexValues.uiMaximum = 0u;
            rAttrDef.sHexValues.uiDefault = 0u;
            break;
        case EType::floating_point:
            sFltValues.dMinimum = rAttrDef.sFltValues.dMinimum;
            sFltValues.dMaximum = rAttrDef.sFltValues.dMaximum;
            sFltValues.dDefault = rAttrDef.sFltValues.dDefault;
            rAttrDef.sFltValues.dMinimum = 0.0;
            rAttrDef.sFltValues.dMaximum = 0.0;
            rAttrDef.sFltValues.dDefault = 0.0;
            break;
        case EType::string:
            new (&sStringValues.ssDefault) std::string(std::move(rAttrDef.sStringValues.ssDefault));
            break;
        case EType::enumerator:
            new (&sEnumValues.vecEnumValues) std::vector<std::string>(std::move(rAttrDef.sEnumValues.vecEnumValues));
            new (&sEnumValues.ssDefault) std::string(std::move(rAttrDef.sEnumValues.ssDefault));
            break;
        default:
            throw SDbcParserException("Internal error: invalid attribute definition type.");
            break;
        }
    }

    SAttributeDef& SAttributeDef::operator=(const SAttributeDef& rAttrDef)
    {
        // Destroy the current definition
        this->~SAttributeDef();

        // Construct new definition
        new (this) SAttributeDef(rAttrDef);

        return *this;
    }

    SAttributeDef& SAttributeDef::operator=(SAttributeDef&& rAttrDef)
    {
        // Destroy the current definition
        this->~SAttributeDef();

        // Construct new definition
        new (this) SAttributeDef(std::move(rAttrDef));

        return *this;
    }

    SAttributeValue::SAttributeValue(TAttributeDefPtr& ptrAttrDefParam) : ptrAttrDef(ptrAttrDefParam)
    {
        switch (ptrAttrDef->eType)
        {
        case SAttributeDef::EType::integer:
            iValue = 0;
            break;
        case SAttributeDef::EType::hex_integer:
            uiValue = 0u;
            break;
        case SAttributeDef::EType::floating_point:
            dValue = 0;
            break;
        case SAttributeDef::EType::string:
            new (&ssValue) std::string;
            break;
        case SAttributeDef::EType::enumerator:
            new (&ssValue) std::string;
            break;
        default:
            throw SDbcParserException("Internal error: invalid attribute definition type.");
            break;
        }
    }

    SAttributeValue::~SAttributeValue()
    {
        switch (ptrAttrDef->eType)
        {
        case SAttributeDef::EType::string:
            ssValue.~basic_string();
            break;
        case SAttributeDef::EType::enumerator:
            ssValue.~basic_string();
            break;
        default:
            break;
        }
    }

    SAttributeValue::SAttributeValue(const SAttributeValue& rAttrVal)  : ptrAttrDef(rAttrVal.ptrAttrDef)
    {
        switch (ptrAttrDef->eType)
        {
        case SAttributeDef::EType::integer:
            iValue = rAttrVal.iValue;
            break;
        case SAttributeDef::EType::hex_integer:
            uiValue = rAttrVal.uiValue;
            break;
        case SAttributeDef::EType::floating_point:
            dValue = rAttrVal.dValue;
            break;
        case SAttributeDef::EType::string:
            new (&ssValue) std::string(rAttrVal.ssValue);
            break;
        case SAttributeDef::EType::enumerator:
            new (&ssValue) std::string(rAttrVal.ssValue);
            break;
        default:
            throw SDbcParserException("Internal error: invalid attribute definition type.");
            break;
        }
    }

    SAttributeValue::SAttributeValue(SAttributeValue&& rAttrVal) : ptrAttrDef(rAttrVal.ptrAttrDef)
    {
        switch (ptrAttrDef->eType)
        {
        case SAttributeDef::EType::integer:
            iValue = rAttrVal.iValue;
            break;
        case SAttributeDef::EType::hex_integer:
            uiValue = rAttrVal.uiValue;
            break;
        case SAttributeDef::EType::floating_point:
            dValue = rAttrVal.dValue;
            break;
        case SAttributeDef::EType::string:
            new (&ssValue) std::string(std::move(rAttrVal.ssValue));
            break;
        case SAttributeDef::EType::enumerator:
            new (&ssValue) std::string(std::move(rAttrVal.ssValue));
            break;
        default:
            throw SDbcParserException("Internal error: invalid attribute definition type.");
            break;
        }
    }

    SAttributeValue& SAttributeValue::operator=(const SAttributeValue& rAttrVal)
    {
        // Destroy the current value
        this->~SAttributeValue();

        // Construct new value
        new (this) SAttributeValue(rAttrVal);

        return *this;
    }

    SAttributeValue& SAttributeValue::operator=(SAttributeValue&& rAttrVal)
    {
        // Destroy the current value
        this->~SAttributeValue();

        // Construct new value
        new (this) SAttributeValue(std::move(rAttrVal));

        return *this;
    }

    CDbcParser::CDbcParser(bool bNoDefaultDef /*= false*/)
    {
        if (!bNoDefaultDef)
        {
            // Parse the standard attribute definitions
            CDbcSource source(std::string(R"code(
BA_DEF_ BO_ "VFrameFormat" ENUM "StandardCAN","ExtendedCAN","StandardFD","ExtendedFD";
BA_DEF_ BO_ "GenMsgSendType" ENUM "cyclic","triggered","cyclicIfActive","cyclicAndTriggered","cyclicIfActiveAndTriggered","none";
BA_DEF_ BO_ "GenMsgCycleTime" INT 0 10000;
BA_DEF_ BO_ "GenMsgStartDelayTime" INT 0 10000;
BA_DEF_ BO_ "GenMsgDelayTime" INT 0 0;
BA_DEF_ SG_ "GenSigSendType" ENUM "none","Cyclic","OnWrite","OnChange";
BA_DEF_ SG_ "GenSigCycleTime" INT 0 10000;
BA_DEF_ SG_ "GenSigDelayTime" INT 0 10000;
BA_DEF_ SG_ "GenSigStartDelayTime" INT 0 10000;
BA_DEF_ SG_ "GenSigStartValue" INT 0 0;
BA_DEF_DEF_ "VFrameFormat" "StandardCAN";
BA_DEF_DEF_ "GenMsgSendType" "none";
BA_DEF_DEF_ "GenMsgCycleTime" 0;
BA_DEF_DEF_ "GenMsgStartDelayTime" 0;
BA_DEF_DEF_ "GenMsgDelayTime" 0;
BA_DEF_DEF_ "GenSigSendType" "none";
BA_DEF_DEF_ "GenSigDelayTime" 0;
BA_DEF_DEF_ "GenSigCycleTime" 0;
BA_DEF_DEF_ "GenSigStartDelayTime" 0;
BA_DEF_DEF_ "GenSigStartValue" 0;
            )code"));
            Parse(source);
        }
    }

    void CDbcParser::Parse(CDbcSource& rSource)
    {
        // Add to the list of sources.
        m_vecSources.push_back(rSource);

        // Reset the ID of the Vector independent message
        m_uiIndepMsgId = 0xffffffff;
        try
        {
            SkipWhitespace(rSource);
            while (!rSource.IsEOF())
            {
                // Expecting a keyword
                std::string ssKeyword = GetIdentifier(rSource);
                if (ssKeyword.empty())
                    throw SDbcParserException("Keyword expected");

                if (ssKeyword == "VERSION") ReadVersion(rSource);
                else if (ssKeyword == "NS_") ReadNewSymbols(rSource);
                else if (ssKeyword == "BS_") ReadBitTiming(rSource);
                else if (ssKeyword == "BU_") ReadNodeDef(rSource);
                else if (ssKeyword == "VAL_TABLE_") ReadValTable(rSource);
                else if (ssKeyword == "BO_") ReadMessageDef(rSource);
                else if (ssKeyword == "SIG_VALTYPE_") ReadSignalValueTypeDef(rSource);
                else if (ssKeyword == "BO_TX_BU_") ReadMessageTransmitters(rSource);
                else if (ssKeyword == "VAL_") ReadValueDescriptions(rSource);
                else if (ssKeyword == "EV_") ReadEnvVarDef(rSource);
                else if (ssKeyword == "ENVVAR_DATA_") ReadEnvVarData(rSource);
                else if (ssKeyword == "SGTYPE_") ReadSignalTypeDef(rSource);
                else if (ssKeyword == "SIG_GROUP_") ReadSignalGroupDef(rSource);
                else if (ssKeyword == "CM_") ReadCommentDef(rSource);
                else if (ssKeyword == "BA_DEF_") ReadAttrDef(rSource);
                else if (ssKeyword == "BA_DEF_DEF_") ReadAttrDefaultDef(rSource);
                else if (ssKeyword == "BA_") ReadAttributes(rSource);
                else if (ssKeyword == "SG_MUL_VAL_") ReadSignalMultiplexDef(rSource);
                else throw SDbcParserException(rSource, "Unknown keyword '%1'.", ssKeyword);

                SkipWhitespace(rSource);
            }
        }
        catch (SDbcParserException& rsException)
        {
            // Add the source
            if (!rsException.Source().IsValid())
                rsException.Source(rSource);

            // Retrow
            throw;
        }
    }

    void CDbcParser::Clear()
    {
        m_vecVersions.clear();
        m_mapNodes.clear();
        m_mapValueTables.clear();
        m_mapMsgDefByName.clear();
        m_mapMsgDefById.clear();
    }

    const std::vector<CDbcSource>& CDbcParser::GetSources() const
    {
        return m_vecSources;
    }

    const std::vector<std::string>& CDbcParser::GetVersions() const
    {
        return m_vecVersions;
    }

    bool CDbcParser::HasNodeDef(const std::string& rssNodeDefName) const
    {
        return m_mapNodes.find(rssNodeDefName) != m_mapNodes.end();
    }

    const std::vector<std::string> CDbcParser::GetNodeDefNames() const
    {
        std::vector<std::string> vecNodes;
        for (auto prNode : m_mapNodes)
            vecNodes.push_back(prNode.first);
        return vecNodes;
    }

    std::pair<SNodeDef, bool> CDbcParser::GetNodeDef(const std::string& rssNodeDefName) const
    {
        auto itNode = m_mapNodes.find(rssNodeDefName);
        if (itNode == m_mapNodes.end()) return std::make_pair(SNodeDef(), false);
        return std::make_pair(itNode->second, true);
    }

    bool CDbcParser::HasValueTable(const std::string& rssName) const
    {
        return m_mapValueTables.find(rssName) != m_mapValueTables.end();
    }

    bool CDbcParser::HasValue(const std::string& rssTableName, uint32_t uiValue) const
    {
        auto itTable = m_mapValueTables.find(rssTableName);
        if (itTable == m_mapValueTables.end()) return false;
        return itTable->second.find(uiValue) != itTable->second.end();
    }

    std::string CDbcParser::GetValue(const std::string& rssTableName, uint32_t uiValue) const
    {
        auto itTable = m_mapValueTables.find(rssTableName);
        if (itTable == m_mapValueTables.end()) return std::string();
        auto itValue = itTable->second.find(uiValue);
        if (itValue == itTable->second.end()) return std::string();
        return itValue->second;
    }

    std::vector<std::string> CDbcParser::GetValueTableNames() const
    {
        std::vector<std::string> vecTables;
        for (auto prTable : m_mapValueTables)
            vecTables.push_back(prTable.first);
        return vecTables;
    }

    std::pair<std::vector<uint32_t>, bool> CDbcParser::GetValues(const std::string& rssTableName) const
    {
        // Find the table
        auto itTable = m_mapValueTables.find(rssTableName);
        std::vector<uint32_t> vecEntries;
        if (itTable == m_mapValueTables.end())
            return std::make_pair(vecEntries, false);

        for (auto prEntry : itTable->second)
            vecEntries.push_back(prEntry.first);

        return std::make_pair(std::move(vecEntries), true);
    }

    bool CDbcParser::HasMsgDef(const std::string& rssName) const
    {
        return m_mapMsgDefByName.find(rssName) != m_mapMsgDefByName.end();
    }

    bool CDbcParser::HasMsgDef(uint32_t uiRawId) const
    {
        return m_mapMsgDefById.find(uiRawId) != m_mapMsgDefById.end();
    }

    bool CDbcParser::HasMsgDefStdId(uint32_t uiStdId) const
    {
        if (uiStdId >= 1u << 11) return false;
        return m_mapMsgDefById.find(uiStdId) != m_mapMsgDefById.end();
    }

    bool CDbcParser::HasMsgDefExtId(uint32_t uiExtId) const
    {
        if (uiExtId >= 1u << 29) return false;
        return m_mapMsgDefById.find(uiExtId | 0x80000000u) != m_mapMsgDefById.end();
    }

    std::vector<uint32_t> CDbcParser::GetMessageIDs() const
    {
        std::vector<uint32_t> vecMsgIDs;
        for (auto prMsgDef : m_mapMsgDefById)
            vecMsgIDs.push_back(prMsgDef.first);
        return vecMsgIDs;
    }

    std::pair<uint32_t, bool> CDbcParser::ExtractMsgId(uint32_t uiRawId)
    {
        return std::make_pair(uiRawId & 0x7fffffffu, uiRawId & 0x80000000u ? true : false);
    }

    uint32_t CDbcParser::ComposeRawId(uint32_t uiMsgId, bool bExtended)
    {
        return uiMsgId | (bExtended ? 0x80000000u : 0x00000000u);
    }

    std::pair<SMessageDef, bool> CDbcParser::GetMsgDef(const std::string& rssName) const
    {
        auto itMsgDef = m_mapMsgDefByName.find(rssName);
        if (itMsgDef == m_mapMsgDefByName.end()) return std::make_pair(SMessageDef(), false);
        return std::make_pair(*itMsgDef->second, true);
    }

    std::pair<SMessageDef, bool> CDbcParser::GetMsgDef(uint32_t uiRawId) const
    {
        auto itMsgDef = m_mapMsgDefById.find(uiRawId);
        if (itMsgDef == m_mapMsgDefById.end()) return std::make_pair(SMessageDef(), false);
        return std::make_pair(*itMsgDef->second, true);
    }

    std::pair<SMessageDef, bool> CDbcParser::GetMsgDefStdId(uint32_t uiStdId) const
    {
        if (uiStdId >= 1u << 11) return std::make_pair(SMessageDef(), false);
        auto itMsgDef = m_mapMsgDefById.find(uiStdId);
        if (itMsgDef == m_mapMsgDefById.end()) return std::make_pair(SMessageDef(), false);
        return std::make_pair(*itMsgDef->second, true);
    }

    std::pair<SMessageDef, bool> CDbcParser::GetMsgDefExtId(uint32_t uiExtId) const
    {
        if (uiExtId >= 1u << 29) return std::make_pair(SMessageDef(), false);
        auto itMsgDef = m_mapMsgDefById.find(uiExtId | 0x80000000u);
        if (itMsgDef == m_mapMsgDefById.end()) return std::make_pair(SMessageDef(), false);
        return std::make_pair(*itMsgDef->second, true);
    }

    bool CDbcParser::HasSignalDef(const std::string& rssMsgName, const std::string& rssSignalName) const
    {
        auto prMsgDef = GetMsgDef(rssMsgName);
        if (!prMsgDef.second) return false;
        return std::find_if(prMsgDef.first.vecSignals.begin(), prMsgDef.first.vecSignals.end(),
            [&](const SSignalDef& rsSignalDef) -> bool
            {
                return rsSignalDef.ssName == rssSignalName;
            }) != prMsgDef.first.vecSignals.end();
    }

    bool CDbcParser::HasSignalDef(uint32_t uiRawMsgId, const std::string & rssSignalName) const
    {
        auto prMsgDef = GetMsgDef(uiRawMsgId);
        if (!prMsgDef.second) return false;
        return std::find_if(prMsgDef.first.vecSignals.begin(), prMsgDef.first.vecSignals.end(),
            [&](const SSignalDef& rsSignalDef) -> bool
            {
                return rsSignalDef.ssName == rssSignalName;
            }) != prMsgDef.first.vecSignals.end();
    }

    bool CDbcParser::HasSignalDefStdId(uint32_t uiStdMsgId, const std::string & rssSignalName) const
    {
        auto prMsgDef = GetMsgDefStdId(uiStdMsgId);
        if (!prMsgDef.second) return false;
        return std::find_if(prMsgDef.first.vecSignals.begin(), prMsgDef.first.vecSignals.end(),
            [&](const SSignalDef& rsSignalDef) -> bool
            {
                return rsSignalDef.ssName == rssSignalName;
            }) != prMsgDef.first.vecSignals.end();
    }

    bool CDbcParser::HasSignalDefExtId(uint32_t uiExtMsgId, const std::string & rssSignalName) const
    {
        auto prMsgDef = GetMsgDefExtId(uiExtMsgId);
        if (!prMsgDef.second) return false;
        return std::find_if(prMsgDef.first.vecSignals.begin(), prMsgDef.first.vecSignals.end(),
            [&](const SSignalDef& rsSignalDef) -> bool
            {
                return rsSignalDef.ssName == rssSignalName;
            }) != prMsgDef.first.vecSignals.end();
    }

    std::vector<std::string> CDbcParser::GetSignalNames(uint32_t uiRawMsgId) const
    {
        auto prMsgDef = GetMsgDef(uiRawMsgId);
        if (!prMsgDef.second) return std::vector<std::string>();
        std::vector<std::string> vecSigNames;
        for (const SSignalDef& rsSignalDef : prMsgDef.first.vecSignals)
            vecSigNames.push_back(rsSignalDef.ssName);
        return vecSigNames;
    }

    std::pair<SSignalDef, bool> CDbcParser::GetSignalDef(const std::string & rssMsgName, const std::string & rssSignalName) const
    {
        auto prMsgDef = GetMsgDef(rssMsgName);
        if (!prMsgDef.second) return std::make_pair(SSignalDef(), false);
        auto itSignalDef = std::find_if(prMsgDef.first.vecSignals.begin(), prMsgDef.first.vecSignals.end(),
            [&](const SSignalDef& rsSignalDef) -> bool
            {
                return rsSignalDef.ssName == rssSignalName;
            });
        if (itSignalDef == prMsgDef.first.vecSignals.end()) return std::make_pair(SSignalDef(), false);
        return std::make_pair(*itSignalDef, true);
    }

    std::pair<SSignalDef, bool> CDbcParser::GetSignalDef(uint32_t uiRawMsgId, const std::string & rssSignalName) const
    {
        auto prMsgDef = GetMsgDef(uiRawMsgId);
        if (!prMsgDef.second) return std::make_pair(SSignalDef(), false);
        auto itSignalDef = std::find_if(prMsgDef.first.vecSignals.begin(), prMsgDef.first.vecSignals.end(),
            [&](const SSignalDef& rsSignalDef) -> bool
            {
                return rsSignalDef.ssName == rssSignalName;
            });
        if (itSignalDef == prMsgDef.first.vecSignals.end()) return std::make_pair(SSignalDef(), false);
        return std::make_pair(*itSignalDef, true);
    }

    std::pair<SSignalDef, bool> CDbcParser::GetSignalDefStdId(uint32_t uiStdMsgId, const std::string & rssSignalName) const
    {
        auto prMsgDef = GetMsgDefStdId(uiStdMsgId);
        if (!prMsgDef.second) return std::make_pair(SSignalDef(), false);
        auto itSignalDef = std::find_if(prMsgDef.first.vecSignals.begin(), prMsgDef.first.vecSignals.end(),
            [&](const SSignalDef& rsSignalDef) -> bool
            {
                return rsSignalDef.ssName == rssSignalName;
            });
        if (itSignalDef == prMsgDef.first.vecSignals.end()) return std::make_pair(SSignalDef(), false);
        return std::make_pair(*itSignalDef, true);
    }

    std::pair<SSignalDef, bool> CDbcParser::GetSignalDefExtId(uint32_t uiExtMsgId, const std::string & rssSignalName) const
    {
        auto prMsgDef = GetMsgDefExtId(uiExtMsgId);
        if (!prMsgDef.second) return std::make_pair(SSignalDef(), false);
        auto itSignalDef = std::find_if(prMsgDef.first.vecSignals.begin(), prMsgDef.first.vecSignals.end(),
            [&](const SSignalDef& rsSignalDef) -> bool
            {
                return rsSignalDef.ssName == rssSignalName;
            });
        if (itSignalDef == prMsgDef.first.vecSignals.end()) return std::make_pair(SSignalDef(), false);
        return std::make_pair(*itSignalDef, true);
    }

    std::vector<std::string> CDbcParser::GetEnvVarNames() const
    {
        std::vector<std::string> vecEnvVars;
        for (auto prEnvVar : m_mapEnvVars)
            vecEnvVars.push_back(prEnvVar.first);
        return vecEnvVars;
    }

    std::pair<SEnvVarDef, bool> CDbcParser::GetEnvVarDef(const std::string& rssVarName) const
    {
        auto itVarDef = m_mapEnvVars.find(rssVarName);
        if (itVarDef == m_mapEnvVars.end()) return std::make_pair(SEnvVarDef(), false);
        return std::make_pair(itVarDef->second, true);
    }

    std::vector<std::string> CDbcParser::GetSignalTypeDefNames() const
    {
        std::vector<std::string> vecTypeDefs;
        for (auto prTypeDef : m_mapSigTypeDefs)
            vecTypeDefs.push_back(prTypeDef.first);
        return vecTypeDefs;
    }

    std::pair<SSignalTypeDef, bool> CDbcParser::GetSignalTypeDef(const std::string& rssSignalTypeDefName) const
    {
        auto itTypeDef = m_mapSigTypeDefs.find(rssSignalTypeDefName);
        if (itTypeDef == m_mapSigTypeDefs.end()) return std::make_pair(SSignalTypeDef(), false);
        return std::make_pair(itTypeDef->second, true);
    }

    std::vector<std::string> CDbcParser::GetSignalGroupDefNames(uint32_t uiRawMsgId) const
    {
        std::vector<std::string> vecGroupDefs;
        auto itMsgDef = m_mapMsgDefById.find(uiRawMsgId);
        if (itMsgDef != m_mapMsgDefById.end())
        {
            for (auto prGroupDef : itMsgDef->second->mapSigGroups)
                vecGroupDefs.push_back(prGroupDef.first);
        }
        return vecGroupDefs;
    }

    std::pair<SSignalGroupDef, bool> CDbcParser::GetSignalGroupDef(uint32_t uiRawMsgId, const std::string& rssSignalGroupDefName) const
    {
        auto itMsgDef = m_mapMsgDefById.find(uiRawMsgId);
        if (itMsgDef == m_mapMsgDefById.end()) return std::make_pair(SSignalGroupDef(), false);
        auto itSigGroupDef = itMsgDef->second->mapSigGroups.find(rssSignalGroupDefName);
        if (itSigGroupDef == itMsgDef->second->mapSigGroups.end()) return std::make_pair(SSignalGroupDef(), false);
        return std::make_pair(itSigGroupDef->second, true);
    }

    const std::vector<std::string>& CDbcParser::GetComments() const
    {
        return m_vecComments;
    }

    std::vector<std::string> CDbcParser::GetAttributeDefNames() const
    {
        std::vector<std::string> vecAttrDefs;
        for (auto prAttrDef : m_mapAttrDefs)
            vecAttrDefs.push_back(prAttrDef.first);
        return vecAttrDefs;
    }

    std::pair<SAttributeDef, bool> CDbcParser::GetAttributeDef(const std::string& rssAttributeDefName) const
    {
        auto itAttrDef = m_mapAttrDefs.find(rssAttributeDefName);
        if (itAttrDef == m_mapAttrDefs.end()) return std::make_pair(SAttributeDef(SAttributeDef::EType::integer), false);
        return std::make_pair(*itAttrDef->second, true);
    }

    const std::vector<SAttributeValue>& CDbcParser::GetAttributes() const
    {
        return m_vecAttributes;
    }

    void CDbcParser::SkipWhitespace(CDbcSource& rSource)
    {
        // NOTE: Comments are not part of the official standard.
        while (!rSource.IsEOF())
        {
            if (std::isspace(rSource.CurrentChar()))
            {
                rSource.Pos()++;
                continue;
            }
            if (rSource.CurrentChar() == '/')
            {
                auto sPosLock = rSource.CreatePosLock();
                rSource.Pos()++;
                if (rSource.CurrentChar() == '/')
                {
                    // Skip the rest of the line
                    sPosLock.Promote();
                    while (!rSource.IsEOF() && rSource.CurrentChar() != '\n')
                        rSource.Pos()++;
                    continue;
               }
            }
            break;
        }
    }

    std::pair<uint32_t, bool> CDbcParser::GetUInt(CDbcSource& rSource)
    {
        SkipWhitespace(rSource);
        if (rSource.IsEOF()) return std::make_pair<uint32_t, bool>(0u, false);
        const char* szBegin = rSource.ContentPtr();
        char* szEnd = nullptr;
        uint32_t uiVal = static_cast<uint32_t>(std::strtoul(szBegin, &szEnd, 10));
        if (errno == ERANGE) return std::make_pair<uint32_t, bool>(0u, false);
        if (!szEnd || szEnd == szBegin) return std::make_pair<uint32_t, bool>(0u, false);
        rSource.Pos() += (szEnd - szBegin);
        return std::make_pair(uiVal, true);
    }

    std::pair<int32_t, bool> CDbcParser::GetInt(CDbcSource& rSource)
    {
        SkipWhitespace(rSource);
        if (rSource.IsEOF()) return std::make_pair<int32_t, bool>(0, false);
        const char* szBegin = rSource.ContentPtr();
        char* szEnd = nullptr;
        int32_t iVal = static_cast<int32_t>(std::strtol(szBegin, &szEnd, 10));
        if (errno == ERANGE) return std::make_pair<int32_t, bool>(0, false);
        if (!szEnd || szEnd == szBegin) return std::make_pair<int32_t, bool>(0, false);
        rSource.Pos() += (szEnd - szBegin);
        return std::make_pair(iVal, true);
    }

    std::pair<double, bool> CDbcParser::GetDouble(CDbcSource& rSource)
    {
        SkipWhitespace(rSource);
        if (rSource.IsEOF()) return std::make_pair<double, bool>(0.0, false);
        const char* szBegin = rSource.ContentPtr();
        char* szEnd = nullptr;
        double dVal = std::strtod(szBegin, &szEnd);
        if (errno == ERANGE) return std::make_pair<double, bool>(0.0, false);
        if (!szEnd || szEnd == szBegin) return std::make_pair<double, bool>(0.0, false);
        rSource.Pos() += (szEnd - szBegin);
        return std::make_pair(dVal, true);
    }

    bool CDbcParser::ExpectChar(char c, CDbcSource& rSource)
    {
        SkipWhitespace(rSource);
        if (rSource.IsEOF()) return false;
        if (rSource.CurrentChar() != c) return false;
        rSource.Pos()++;
        return true;
    }

    std::pair<std::string, bool> CDbcParser::GetString(CDbcSource& rSource)
    {
        SkipWhitespace(rSource);
        if (rSource.IsEOF()) return std::pair<std::string, bool>{std::string(), false};

        // A string should start with a quote
        if (rSource.CurrentChar() != '\"') return std::pair<std::string, bool>{std::string(), false};

        // Search for the next quote - skip escaped quotes
        std::string ssResult;
        do
        {
            size_t nBegin = ++rSource.Pos();
            size_t nEnd = rSource.Content().find_first_of("\"\\", nBegin);
            if (nEnd == std::string::npos) return std::pair<std::string, bool>{std::string(), false};
            ssResult += rSource.Content().substr(nBegin, nEnd - nBegin);
            rSource.Pos() = nEnd + 1;
            if (rSource.Content()[nEnd] != '\\') break;

            // Escaped character. Add and continue.
            ssResult += rSource.Content()[nEnd + 1];
            rSource.Pos() = nEnd + 1;
        } while (true);

        return std::pair<std::string, bool>{ssResult, true};
    }

    std::string CDbcParser::GetIdentifier(CDbcSource& rSource)
    {
        SkipWhitespace(rSource);
        std::string ssIdentifier;
        while (!rSource.IsEOF())
        {
            char c = rSource.CurrentChar();
            // Allowed are '_' and alphanumerical characters. The identifier is not allowed to start with a number
            // Undocumented extension: '-', '<' and '>' are also allowed.
            if (c != '_' && c != '-' && c != '<' && c != '>' && !(ssIdentifier.empty() ? std::isalpha(c) : std::isalnum(c)))
                break;
            ssIdentifier += c;
            rSource.Pos()++;
        }
        return ssIdentifier;
    }

    bool CDbcParser::IsDbcIdentifier(const std::string& rssIdentifier)
    {
        const char* rgszDbcKeywords[] = { "VERSION", "NS_", "NS_DESC_", "CM_", "BA_DEF_", "BA_", "VAL_", "CAT_DEF_", "CAT_", "FILTER",
            "BA_DEF_DEF_", "EV_DATA_", "ENVVAR_DATA_", "SGTYPE_", "SGTYPE_VAL_", "BA_DEF_SGTYPE_", "BA_SGTYPE_", "SIG_TYPE_REF_",
            "VAL_TABLE_", "SIG_GROUP_", "SIG_VALTYPE_", "SIGTYPE_VALTYPE_", "BO_TX_BU_", "BA_DEF_REL_", "BA_REL_", "BA_DEF_DEF_REL_",
            "BU_SG_REL_", "BU_EV_REL_", "BU_BO_REL_", "SG_MUL_VAL_", "BS_", "BU_", "BO_", "SG_", "EV_", "VECTOR__XXX" };

        // Check whether the identifier does not correspond to one within the keyword list
        auto itDbcKeyword = std::find(std::begin(rgszDbcKeywords), std::end(rgszDbcKeywords), rssIdentifier);
        return itDbcKeyword != std::end(rgszDbcKeywords);
    }

    void CDbcParser::ReadVersion(CDbcSource& rSource)
    {
        auto prVersion = GetString(rSource);
        if (!prVersion.second)
            throw SDbcParserException("String expected following 'VERSION' keyword");

        m_vecVersions.push_back(prVersion.first);
    }

    void CDbcParser::ReadNewSymbols(CDbcSource& rSource)
    {
        const char* rgszNewSymbols[] = { "CM_", "NS_DESC_", "BA_DEF_", "BA_", "VAL_", "CAT_DEF_", "CAT_", "FILTER", "BA_DEF_DEF_",
            "EV_DATA_", "ENVVAR_DATA_", "SGTYPE_", "SGTYPE_VAL_", "BA_DEF_SGTYPE_", "BA_SGTYPE_", "SIG_TYPE_REF_", "VAL_TABLE_",
            "SIG_GROUP_", "SIG_VALTYPE_", "SIGTYPE_VALTYPE_", "BO_TX_BU_", "BA_DEF_REL_", "BA_REL_", "BA_DEF_DEF_REL_",
            "BU_SG_REL_", "BU_EV_REL_", "BU_BO_REL_", "SG_MUL_VAL_" };

        // Expect separator
        if (!ExpectChar(':', rSource))
            throw SDbcParserException("Colon ':' expected following 'NS_' keyword");

        // Zero or more symbols from the above list can be defined.
        do
        {
            // Get the next identifier
            auto sPosLock = rSource.CreatePosLock();
            std::string ssSymbol = GetIdentifier(rSource);
            if (ssSymbol.empty()) break;

            // Check whether the identifier corresponds to one of the above list
            auto itSymbol = std::find(std::begin(rgszNewSymbols), std::end(rgszNewSymbols), ssSymbol);
            if (itSymbol == std::end(rgszNewSymbols)) break;

            // Update the position and check for the next symbol.
            sPosLock.Promote();
        } while (!rSource.IsEOF());
    }

    void CDbcParser::ReadBitTiming(CDbcSource& rSource)
    {
        // Expect separator
        if (!ExpectChar(':', rSource))
            throw SDbcParserException("Colon ':' expected following 'BS_' keyword");

        // Optionally the timing could be specified (since this section has been defined obsolete, it is more likely that
        // nothing is specified).
        auto prBaudrate = GetUInt(rSource);
        if (prBaudrate.second)
        {
            if (!ExpectChar(':', rSource))
                throw SDbcParserException("Colon ':' expected following baudrate value.");
            auto prBTR1 = GetUInt(rSource);
            if (!prBTR1.second) throw SDbcParserException("BTR1 register value expected.");
            if (!ExpectChar(',', rSource))
                throw SDbcParserException("Comma ',' expected following BTR1 register value");
            auto prBTR2 = GetUInt(rSource);
            if (!prBTR2.second) throw SDbcParserException("BTR2 register value expected.");
        }
    }

    void CDbcParser::ReadNodeDef(CDbcSource& rSource)
    {
        // Expect separator
        if (!ExpectChar(':', rSource))
            throw SDbcParserException("Colon ':' expected following 'BU_' keyword.");

        // Zero or more nodes can be defined
        std::map<std::string, SNodeDef> mapNodesLocal;
        do
        {
            // Get the next identifier
            SNodeDef sNodeDef;
            auto sPosLock = rSource.CreatePosLock();
            sNodeDef.ssName = GetIdentifier(rSource);
            if (sNodeDef.ssName.empty()) break;

            // Check whether the identifier is a DBC identifier
            if (IsDbcIdentifier(sNodeDef.ssName)) break;

            // Add the node name to the local node name list.
            sPosLock.Promote();
            if (mapNodesLocal.find(sNodeDef.ssName) != mapNodesLocal.end())
                throw SDbcParserException("Duplicate nodes defined.");
            mapNodesLocal.insert(std::make_pair(sNodeDef.ssName, sNodeDef));
        } while (!rSource.IsEOF());

        // Add the local nodes to the global node set (if not already existing).
        m_mapNodes.insert(mapNodesLocal.begin(), mapNodesLocal.end());
    }

    void CDbcParser::ReadValTable(CDbcSource& rSource)
    {
        // Get the name of the table
        std::string ssTableName = GetIdentifier(rSource);
        if (ssTableName.empty() || IsDbcIdentifier(ssTableName))
            throw SDbcParserException("Expecting a name for the value table.");
        if (m_mapValueTables.find(ssTableName) != m_mapValueTables.end())
            throw SDbcParserException("Duplicate value table definition.");

        // Zero or more definitions can follow
        TValDescMap mapValues;
        do
        {
            auto prValue = GetUInt(rSource);
            if (!prValue.second) break; // Not a value
            auto prString = GetString(rSource);
            if (!prString.second)
                throw SDbcParserException("Expecting a string following the value.");

            // Duplicate values are not allowed
            if (mapValues.find(prValue.first) != mapValues.end())
                throw SDbcParserException("Duplicate value definition.");

            // Insert the value.
            mapValues.insert(std::make_pair(prValue.first, prString.first));
        } while (!rSource.IsEOF());

        // Expect terminator
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected finalizing the definition of the  'VAL_TABLE_'");

        // Insert the table
        m_mapValueTables.insert(std::make_pair(ssTableName, mapValues));
    }

    void CDbcParser::ReadMessageDef(CDbcSource& rSource)
    {
        // Get the message ID
        auto prId = GetUInt(rSource);
        if (!prId.second)
            throw SDbcParserException("Expecting an ID for the message.");
        uint32_t uiId = prId.first;

        // Get the name of the message
        std::string ssMessageName = GetIdentifier(rSource);
        if (ssMessageName.empty() || IsDbcIdentifier(ssMessageName))
            throw SDbcParserException("Expecting a name for the message.");

        // Correct the ID for the default message (enforce the same ID for this message).
        if (ssMessageName == "VECTOR__INDEPENDENT_SIG_MSG")
        {
            m_uiIndepMsgId = uiId;
            uiId = 0xffffffff;
        }
        else
        {
            if (uiId & 0x80000000)    // Differentiate between extended and standard ID
            {
                if ((uiId & 0x7fffffff) >= 1u<<29)
                    throw SDbcParserException("Specified extended message ID is larger than the 29 bits.");
            } else
            {
                if ((uiId & 0x7fffffff) >= 1u<<11)
                    throw SDbcParserException("Specified standard message ID is larger than the 11 bits.");
            }
        }

        // Expect separator
        if (!ExpectChar(':', rSource))
            throw SDbcParserException("Colon ':' expected following 'BO_' message definition");

        // Get the message size
        auto prSize = GetUInt(rSource);
        if (!prSize.second) throw SDbcParserException("Expecting a size for the message.");

        // Get the transmitter name
        std::string ssTransmitter = GetIdentifier(rSource);
        if (ssTransmitter.empty())
            throw SDbcParserException("Expecting a transmitter name for the message.");
        if (ssTransmitter != "Vector__XXX" && ssTransmitter != "Vector_XXX" && m_mapNodes.find(ssTransmitter) == m_mapNodes.end())
            throw SDbcParserException("Expecting a valid pre-defined transmitter name for the message.");

        // Duplicate definitions are not allowed.
        // Except when the message is named "VECTOR__INDEPENDENT_SIG_MSG", which is just a placeholder. In the latter case, the
        // message is extended with the defined signals.
        auto itMessage = m_mapMsgDefByName.find(ssMessageName);
        if (itMessage == m_mapMsgDefByName.end())
        {
            auto ptrMessage = std::make_shared<SMessageDef>();
            auto prResult1 = m_mapMsgDefByName.emplace(std::make_pair(ssMessageName, ptrMessage));
            if (!prResult1.second)
                throw SDbcParserException("Failed to construct message definition for '%1' and ID %2.", ssMessageName, uiId);
            itMessage = prResult1.first;
            auto prResult2 = m_mapMsgDefById.emplace(std::make_pair(uiId, ptrMessage));
            if (!prResult2.second)
            {
                m_mapMsgDefByName.erase(prResult1.first);
                throw SDbcParserException("Failed to construct message definition for '%1' and ID %2.", ssMessageName, uiId);
            }
        }
        else
        {
            if (ssMessageName != "VECTOR__INDEPENDENT_SIG_MSG")
                throw SDbcParserException("Duplicate message definition for ID '%1' and ID %2.", ssMessageName, uiId);
        }

        // Set the message information
        itMessage->second->ssName = ssMessageName;
        itMessage->second->uiId = uiId;
        itMessage->second->uiSize += prSize.first;
        itMessage->second->vecTransmitters.push_back(ssTransmitter);

        // Check for signals
        do
        {
            // Try peeking for the signal definition. Us a position lock to be able to rollback if the identifier doesn't represent a
            // signal definition.
            auto sPosLock = rSource.CreatePosLock();
            std::string ssIdentfier = GetIdentifier(rSource);
            if (ssIdentfier != "SG_") break;
            sPosLock.Promote();

            ReadSignalDef(rSource, *itMessage->second);
        } while (true);
    }

    void CDbcParser::ReadSignalTypeDefBase(CDbcSource& rSource, SSignalTypeBase& rsSignalTypeDefBase)
    {
        // Get size
        auto prSize = GetUInt(rSource);
        if (!prSize.second)
            throw SDbcParserException("Expecting the signal size.");
        rsSignalTypeDefBase.uiSize = prSize.first;

        // Expect separator
        if (!ExpectChar('@', rSource))
            throw SDbcParserException("At-sign '@' expected following size in the signal definition");

        // Get byte order
        auto prByteOrder = GetUInt(rSource);
        if (!prByteOrder.second)
            throw SDbcParserException("Expecting the byte order indicator.");
        if (prByteOrder.first > 1)
            throw SDbcParserException("Invalid byte order indicator for the signal (0=big endian or 1=little endian are allowed).");
        rsSignalTypeDefBase.eByteOrder = prByteOrder.first ? SSignalDef::EByteOrder::little_endian : SSignalDef::EByteOrder::big_endian;

        // Get the value type
        if (ExpectChar('+', rSource))
            rsSignalTypeDefBase.eValType = SSignalDef::EValueType::unsigned_integer;
        else if (ExpectChar('-', rSource))
            rsSignalTypeDefBase.eValType = SSignalDef::EValueType::signed_integer;
        else
            throw SDbcParserException("Invalid value type ('+'=unsigned or '-'=signed are allowed).");

        // Get the opening bracket
        if (!ExpectChar('(', rSource))
            throw SDbcParserException("Left bracket '(' expected following the value type in the signal definition");

        // Get the factor
        auto prFactor = GetDouble(rSource);
        if (!prFactor.second)
            throw SDbcParserException("Expecting the factor value.");
        if (prFactor.first == 0.0)
            throw SDbcParserException("A factor value of 0 is not valid.");
        rsSignalTypeDefBase.dFactor = prFactor.first;

        // Get the comma
        if (!ExpectChar(',', rSource))
            throw SDbcParserException("Comma '(' expected following the factor value in the signal definition");

        // Get the offset
        auto prOffset = GetDouble(rSource);
        if (!prOffset.second)
            throw SDbcParserException("Expecting the offset value.");
        rsSignalTypeDefBase.dOffset = prOffset.first;

        // Get the closing bracket
        if (!ExpectChar(')', rSource))
            throw SDbcParserException("Right bracket ')' expected following the offset value in the signal definition");

        // Get the opening square-bracket
        if (!ExpectChar('[', rSource))
            throw SDbcParserException("Left square-bracket '[' expected following the factor and offset in the signal definition");

        // Get the minimum
        auto prMinimum = GetDouble(rSource);
        if (!prMinimum.second)
            throw SDbcParserException("Expecting the minimum value.");
        rsSignalTypeDefBase.dMinimum = prMinimum.first;

        // Expect separator
        if (!ExpectChar('|', rSource))
            throw SDbcParserException("Pipe '|' expected following the minimum in the signal definition");

        // Get the maximum
        auto prMaximum = GetDouble(rSource);
        if (!prMaximum.second)
            throw SDbcParserException("Expecting the maximum value.");
        rsSignalTypeDefBase.dMaximum = prMaximum.first;

        // Get the closing square-bracket
        if (!ExpectChar(']', rSource))
            throw SDbcParserException("Right square-bracket ']' expected following the maximum in the signal definition");

        // Get the unit string
        auto prUnit = GetString(rSource);
        if (!prUnit.second)
            throw SDbcParserException("String expected following the minimum and maximum in the signal definition");
        rsSignalTypeDefBase.ssUnit = prUnit.first;
    }

    void CDbcParser::ReadSignalDef(CDbcSource& rSource, SMessageDef& rsMsgDef)
    {
        SSignalDef sSignalDef;

        // Set the message ID
        sSignalDef.uiMsgId = rsMsgDef.uiId;

        // Get the name of the signal
        sSignalDef.ssName = GetIdentifier(rSource);
        if (sSignalDef.ssName.empty() || IsDbcIdentifier(sSignalDef.ssName))
            throw SDbcParserException("Expecting a name for the signal.");
        if (std::find_if(rsMsgDef.vecSignals.begin(), rsMsgDef.vecSignals.end(),
            [&](const SSignalDef& rsSignal) -> bool
            {
                return rsSignal.ssName == sSignalDef.ssName;
            }) != rsMsgDef.vecSignals.end())
            throw SDbcParserException("Duplicate signal defined within the message.");

        // Check for the multiplexer indicator.
        if (ExpectChar('m', rSource))
        {
            sSignalDef.uiMultiplexBitmask |= static_cast<uint32_t>(SSignalDef::EMultiplexBitmask::mltplx_val);
            auto prCaseVal = GetInt(rSource);
            if (!prCaseVal.second)
                throw SDbcParserException("Expecting a multiplexed switch case value.");
            sSignalDef.iMltplxCase = prCaseVal.first;
        }
        if (ExpectChar('M', rSource))
            sSignalDef.uiMultiplexBitmask |= static_cast<uint32_t>(SSignalDef::EMultiplexBitmask::mltplx_switch);

        // Expect separator
        if (!ExpectChar(':', rSource))
            throw SDbcParserException("Colon ':' expected following 'SG_' signal definition");

        // Get start bit
        auto prStartBit = GetUInt(rSource);
        if (!prStartBit.second)
            throw SDbcParserException("Expecting the start bit.");
        if (rsMsgDef.uiId != 0xffffffff && prStartBit.first >= 8 * rsMsgDef.uiSize)
            throw SDbcParserException("Start bit has been defined beyond the size of the message.");
        sSignalDef.uiStartBit = prStartBit.first;

        // Expect separator
        if (!ExpectChar('|', rSource))
            throw SDbcParserException("Pipe '|' expected following start bit in the signal definition");

        // Get the signal type def - base part
        ReadSignalTypeDefBase(rSource, sSignalDef);

        // Check correct size
        if (sSignalDef.eByteOrder == SSignalDef::EByteOrder::little_endian)
        {
            // The signal occupies the area from start-bit until start-bit + size.
            if (rsMsgDef.uiId != 0xffffffff && sSignalDef.uiStartBit + sSignalDef.uiSize > rsMsgDef.uiSize * 8)
                throw SDbcParserException("The length of the signal positioned at the start bit exceeds the length of the message.");
        } else // big endian
        {
            // The signal occupies the byte where the start bit determines the MSB and any number of following bytes that start with
            // the most significant bit. For example:
            // Signal 1: start=7 length = 4
            // Signal 2: start=3 length = 8
            // Signal 3: start=11 length = 6
            // Signal 4: start=21 length = 6
            // Signal 5: start=31 length = 8
            // Signal 6: start=39 length = 32
            //      7   6   5   4   3   2   1   0
            //  0 | Signal 1      | Signal 2 MSB  |
            //  1 | signal 2 LSB  | Signal 3 MSB  |
            //  2 | Sig 3 | Signal 4              |
            //  3 | Signal 5                      |
            //  4 | Signal 6 MSB                  |
            //  5 |                               |
            //  6 |                               |
            //  7 |                  Signal 6 LSB |
            auto fnInverseBitPos = [](uint32_t uiPos) -> uint32_t
            {
                uint32_t uiInverseStartBit = (uiPos >> 3) << 3;
                uint32_t uiInverseStartBitInByte = (8 - ((uiPos + 1) & 7)) & 7;
                return uiInverseStartBit + uiInverseStartBitInByte;
            };
            if (rsMsgDef.uiId != 0xffffffff && fnInverseBitPos(sSignalDef.uiStartBit) + sSignalDef.uiSize > rsMsgDef.uiSize * 8)
                throw SDbcParserException("The length of the signal positioned at the start bit exceeds the length of the message.");
        }

        // Get one or more receivers
        do
        {
            // Get the receiver
            std::string ssReceiver = GetIdentifier(rSource);
            if (ssReceiver.empty() || IsDbcIdentifier(ssReceiver))
                throw SDbcParserException("Expecting a receiver for the signal.");

            // Check the receiver with the predefined nodes (except for "Vector__XXX" node)
            if (ssReceiver != "Vector__XXX" && ssReceiver != "Vector_XXX" && m_mapNodes.find(ssReceiver) == m_mapNodes.end())
                throw SDbcParserException("Expecting a valid pre-defined receiver name for the signal.");

            // Check for multiple definitions
            if (std::find(sSignalDef.vecReceivers.begin(), sSignalDef.vecReceivers.end(), ssReceiver) != sSignalDef.vecReceivers.end())
                throw SDbcParserException("Duplicate receiver name defined for the signal.");

            // Add to the receiver vector
            sSignalDef.vecReceivers.push_back(std::move(ssReceiver));

            // Check for additional receivers
            if (!ExpectChar(',', rSource)) break;
        } while(true);

        // Add the signal to the message
        rsMsgDef.vecSignals.push_back(std::move(sSignalDef));
    }

    void CDbcParser::ReadSignalValueTypeDef(CDbcSource& rSource)
    {
        // Get the message ID
        auto prMsgId = GetUInt(rSource);
        if (!prMsgId.second)
            throw SDbcParserException("Expecting the message ID.");
        if (prMsgId.first == m_uiIndepMsgId)
            prMsgId.first = 0xffffffff;
        auto itMsgDef = m_mapMsgDefById.find(prMsgId.first);
        if (itMsgDef == m_mapMsgDefById.end())
            throw SDbcParserException("Could not find message with supplied ID.");

        // Get the signal name
        std::string ssSignalName = GetIdentifier(rSource);
        auto itSignal = std::find_if(itMsgDef->second->vecSignals.begin(), itMsgDef->second->vecSignals.end(),
            [&](const SSignalDef& rsSignalDef)
            {
                return rsSignalDef.ssName == ssSignalName;
            });
        if (itSignal == itMsgDef->second->vecSignals.end())
            throw SDbcParserException("Could not find signal with supplied name.");

        // DEVIATION: The specification doesn't specifies a colon; CANdb++ inserts a colon
        ExpectChar(':', rSource);

        // Get the value type and if necessary update the value type enum of the signal.
        auto prValType = GetUInt(rSource);
        if (!prValType.second)
            throw SDbcParserException("Expecting the value type (0..2).");
        switch (prValType.first)
        {
        case 0: // Integer (signed/unsigned) is per default.
            break;
        case 1: // IEEE float
            itSignal->eValType = SSignalDef::EValueType::ieee_float;
            break;
        case 2: // IEEE double
            itSignal->eValType = SSignalDef::EValueType::ieee_double;
            break;
        default:
            throw SDbcParserException("Invalid value type (0..2).");
            break;
        }

        // Expecting the ending
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected.");
    }

    void CDbcParser::ReadMessageTransmitters(CDbcSource& rSource)
    {
        // Get the message ID
        auto prMsgId = GetUInt(rSource);
        if (!prMsgId.second)
            throw SDbcParserException("Expecting the message ID.");
        if (prMsgId.first == m_uiIndepMsgId)
            prMsgId.first = 0xffffffff;
        auto itMsgDef = m_mapMsgDefById.find(prMsgId.first);
        if (itMsgDef == m_mapMsgDefById.end())
            throw SDbcParserException("Could not find message with supplied ID.");

        // Expect separator
        if (!ExpectChar(':', rSource))
            throw SDbcParserException("Colon ':' expected following 'BO_TX_BU_' message transmitters definition");

        // Expect zero or more transmitters
        do
        {
            // Check for semi-colon indicating the end of the definition.
            if (ExpectChar(';', rSource)) break;

            // Get the transmitter name
            std::string ssTransmitter = GetIdentifier(rSource);
            if (ssTransmitter.empty())
                throw SDbcParserException("Expecting a transmitter name for the message.");
            if (ssTransmitter != "Vector__XXX" && ssTransmitter != "Vector_XXX" && m_mapNodes.find(ssTransmitter) == m_mapNodes.end())
                throw SDbcParserException("Expecting a valid pre-defined transmitter name for the message.");
            if (std::find(itMsgDef->second->vecTransmitters.begin(), itMsgDef->second->vecTransmitters.end(), ssTransmitter) !=
                itMsgDef->second->vecTransmitters.end())
                throw SDbcParserException("Duplicate transmitter defined for the message.");

            // Add the transmitter.
            itMsgDef->second->vecTransmitters.push_back(ssTransmitter);
        } while(true);
    }

    void CDbcParser::ReadValueDescriptions(CDbcSource& rSource)
    {
        TValDescMap* pmapValues = nullptr;

        // Get the message ID - if existing, this indicates a signal value descriptions.
        // Or a name - this indicates an environment variable
        auto prMsgId = GetUInt(rSource);
        if (prMsgId.second)
        {
            if (prMsgId.first == m_uiIndepMsgId)
                prMsgId.first = 0xffffffff;
            auto itMsgDef = m_mapMsgDefById.find(prMsgId.first);
            if (itMsgDef == m_mapMsgDefById.end())
                throw SDbcParserException("Could not find message with supplied ID.");

            // Get the signal name
            std::string ssSignalName = GetIdentifier(rSource);
            auto itSignal = std::find_if(itMsgDef->second->vecSignals.begin(), itMsgDef->second->vecSignals.end(),
                [&](const SSignalDef& rsSignalDef)
                {
                    return rsSignalDef.ssName == ssSignalName;
                });
            if (itSignal == itMsgDef->second->vecSignals.end())
                throw SDbcParserException("Could not find signal with supplied name.");
            if (!itSignal->mapValueDescriptions.empty())
                throw SDbcParserException("Duplicate value assignment.");

            pmapValues = &itSignal->mapValueDescriptions;
        }
        else
        {
            std::string ssEnvVarName = GetIdentifier(rSource);
            if (ssEnvVarName.empty())
                throw SDbcParserException("Could not find message or environment variable.");
            auto itVarDef = m_mapEnvVars.find(ssEnvVarName);
            if (itVarDef == m_mapEnvVars.end())
                throw SDbcParserException("Could not find environment variable with supplied name.");

            pmapValues = &itVarDef->second.mapValueDescriptions;
        }
        if (!pmapValues)
            throw SDbcParserException("Could not find message or environment variable.");

        // Zero or more definitions can follow
        TValDescMap mapValues;
        do
        {
            auto prValue = GetUInt(rSource);
            if (!prValue.second) break; // Not a value
            auto prString = GetString(rSource);
            if (!prString.second)
                throw SDbcParserException("Expecting a string following the value.");

            // Duplicate values are not allowed
            if (mapValues.find(prValue.first) != mapValues.end())
                throw SDbcParserException("Duplicate value definition.");

            // Insert the value.
            mapValues.insert(std::make_pair(prValue.first, prString.first));
        } while (!rSource.IsEOF());

        // Expect terminator
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected finalizing the definition of the  'VAL_'");

        // Assign the values
        *pmapValues = std::move(mapValues);
    }

    void CDbcParser::ReadEnvVarDef(CDbcSource& rSource)
    {
        SEnvVarDef sEnvVarDef;

        // Get the name of the variable
        sEnvVarDef.ssName = GetIdentifier(rSource);
        if (sEnvVarDef.ssName.empty() || IsDbcIdentifier(sEnvVarDef.ssName))
            throw SDbcParserException("Expecting a name for the environment variable.");
        if (m_mapEnvVars.find(sEnvVarDef.ssName) != m_mapEnvVars.end())
            throw SDbcParserException("Duplicate environment variable defined.");

        // Expect separator
        if (!ExpectChar(':', rSource))
            throw SDbcParserException("Colon ':' expected following 'EV_' environment variable definition");

        // Get the variable type
        auto prType = GetUInt(rSource);
        if (!prType.second)
            throw SDbcParserException("Expecting the byte order indicator.");
        switch (prType.first)
        {
        case 0:
            sEnvVarDef.eType = SEnvVarDef::EType::integer;
            break;
        case 1:
            sEnvVarDef.eType = SEnvVarDef::EType::floating_point;
            break;
        case 2:
            sEnvVarDef.eType = SEnvVarDef::EType::string;
            break;
        default:
            throw SDbcParserException("Invalid type defined for environment variable definition (0=integer, 1=float and 2=string are allowed).");
            break;
        }

        // Get the opening square-bracket
        if (!ExpectChar('[', rSource))
            throw SDbcParserException("Left square-bracket '[' expected following the variable type in the environment variable definition");

        // Get the minimum
        auto prMinimum = GetDouble(rSource);
        if (!prMinimum.second)
            throw SDbcParserException("Expecting the minimum value.");
        sEnvVarDef.dMinimum = prMinimum.first;

        // Expect separator
        if (!ExpectChar('|', rSource))
            throw SDbcParserException("Pipe '|' expected following the minimum in the environment variable definition");

        // Get the maximum
        auto prMaximum = GetDouble(rSource);
        if (!prMaximum.second)
            throw SDbcParserException("Expecting the maximum value.");
        sEnvVarDef.dMaximum = prMaximum.first;

        // Get the closing square-bracket
        if (!ExpectChar(']', rSource))
            throw SDbcParserException("Right square-bracket ']' expected following the maximum in the environment variable definition");

        // Get the unit string
        auto prUnit = GetString(rSource);
        if (!prUnit.second)
            throw SDbcParserException("String expected following the minimum and maximum in the environment variable definition");
        sEnvVarDef.ssUnit = prUnit.first;

        // Get the initial value
        auto prInitVal = GetDouble(rSource);
        if (!prInitVal.second)
            throw SDbcParserException("Expecting the initial value.");
        sEnvVarDef.dInitVal = prInitVal.first;

        // Get the variable ID
        auto prId = GetUInt(rSource);
        if (!prId.second)
            throw SDbcParserException("Expecting the environment variable ID.");
        sEnvVarDef.uiId = prId.first;

        // Get the access type
        std::string ssAccess = GetIdentifier(rSource);
        if (ssAccess == "DUMMY_NODE_VECTOR0")
            sEnvVarDef.eAccess = SEnvVarDef::EAccessType::unrestricted;
        else if (ssAccess == "DUMMY_NODE_VECTOR1")
            sEnvVarDef.eAccess = SEnvVarDef::EAccessType::read;
        else if (ssAccess == "DUMMY_NODE_VECTOR2")
            sEnvVarDef.eAccess = SEnvVarDef::EAccessType::write;
        else if (ssAccess == "DUMMY_NODE_VECTOR3")
            sEnvVarDef.eAccess = SEnvVarDef::EAccessType::readwrite;
        else if (ssAccess == "DUMMY_NODE_VECTOR8000")
        {
            if (sEnvVarDef.eType != SEnvVarDef::EType::string)
                throw SDbcParserException("The access type expects the environment variable to be a string.");
            sEnvVarDef.eAccess = SEnvVarDef::EAccessType::unrestricted;
        }
        else if (ssAccess == "DUMMY_NODE_VECTOR8001")
        {
            if (sEnvVarDef.eType != SEnvVarDef::EType::string)
                throw SDbcParserException("The access type expects the environment variable to be a string.");
            sEnvVarDef.eAccess = SEnvVarDef::EAccessType::read;
        }
        else if (ssAccess == "DUMMY_NODE_VECTOR8002")
        {
            if (sEnvVarDef.eType != SEnvVarDef::EType::string)
                throw SDbcParserException("The access type expects the environment variable to be a string.");
            sEnvVarDef.eAccess = SEnvVarDef::EAccessType::write;
        }
        else if (ssAccess == "DUMMY_NODE_VECTOR8003")
        {
            if (sEnvVarDef.eType != SEnvVarDef::EType::string)
                throw SDbcParserException("The access type expects the environment variable to be a string.");
            sEnvVarDef.eAccess = SEnvVarDef::EAccessType::readwrite;
        }
        else
            throw SDbcParserException("Invalid access type for the environment variable.");

        // Get one or more access nodes
        do
        {
            // Get the access node
            std::string ssNode = GetIdentifier(rSource);
            if (ssNode.empty() || IsDbcIdentifier(ssNode))
                throw SDbcParserException("Expecting an access node for the environment variable.");

            // Check the access node with the predefined nodes (except for "Vector__XXX" node)
            if (ssNode != "Vector__XXX" && ssNode != "Vector_XXX" && m_mapNodes.find(ssNode) == m_mapNodes.end())
                throw SDbcParserException("Expecting a valid pre-defined access node name for the environment variable.");

            // Check for multiple definitions
            if (std::find(sEnvVarDef.vecNodes.begin(), sEnvVarDef.vecNodes.end(), ssNode) != sEnvVarDef.vecNodes.end())
                throw SDbcParserException("Duplicate access node name defined for the environment variable.");

            // Add to the receiver vector
            sEnvVarDef.vecNodes.push_back(std::move(ssNode));

            // Check for additional receivers
            if (!ExpectChar(',', rSource)) break;
        } while(true);

        // Expecting the ending
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected.");

        // Add the variable to the message
        m_mapEnvVars.insert(std::make_pair(sEnvVarDef.ssName, sEnvVarDef));
    }

    void CDbcParser::ReadEnvVarData(CDbcSource& rSource)
    {
        std::string ssEnvVarName = GetIdentifier(rSource);
        if (ssEnvVarName.empty())
            throw SDbcParserException("Expected environment variable name.");
        auto itVarDef = m_mapEnvVars.find(ssEnvVarName);
        if (itVarDef == m_mapEnvVars.end())
            throw SDbcParserException("Could not find environment variable with supplied name.");

        // Expect separator
        if (!ExpectChar(':', rSource))
            throw SDbcParserException("Colon ':' expected following 'ENVVAR_DATA_' environment variable data definition");

        // Get the data size
        auto prSize = GetUInt(rSource);
        if (!prSize.second)
            throw SDbcParserException("Expecting the data size.");

        // Expecting the ending
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected.");

        // Promote the data type of the variable to "data" and set the data size
        itVarDef->second.eType = SEnvVarDef::EType::data;
        itVarDef->second.uiDataSize = prSize.first;
    }

    void CDbcParser::ReadSignalTypeDef(CDbcSource& rSource)
    {
        // Get the message ID - if existing, this indicates a signal type def assignment.
        // Or a name - this indicates a signal type definition.
        auto prMsgId = GetUInt(rSource);
        if (prMsgId.second)
        {
            if (prMsgId.first == m_uiIndepMsgId)
                prMsgId.first = 0xffffffff;
            auto itMsgDef = m_mapMsgDefById.find(prMsgId.first);
            if (itMsgDef == m_mapMsgDefById.end())
                throw SDbcParserException("Could not find message with supplied ID.");

            // Get the signal name
            std::string ssSignalName = GetIdentifier(rSource);
            auto itSignal = std::find_if(itMsgDef->second->vecSignals.begin(), itMsgDef->second->vecSignals.end(),
                [&](const SSignalDef& rsSignalDef)
                {
                    return rsSignalDef.ssName == ssSignalName;
                });
            if (itSignal == itMsgDef->second->vecSignals.end())
                throw SDbcParserException("Could not find signal with supplied name.");
            if (!itSignal->ssSignalTypeDef.empty())
                throw SDbcParserException("Duplicate signal type definition assignment.");

            // Expect separator
            if (!ExpectChar(':', rSource))
                throw SDbcParserException("Colon ':' expected following 'SGTYPE_' signal type definition");

            // Get the name of the signal type definition
            std::string ssSignalTypeDef = GetIdentifier(rSource);
            if (m_mapSigTypeDefs.find(ssSignalTypeDef) == m_mapSigTypeDefs.end())
                throw SDbcParserException("Could not find signal type definition with supplied name.");
            itSignal->ssSignalTypeDef = ssSignalTypeDef;
        }
        else
        {
            SSignalTypeDef sSignalTypeDef;
            sSignalTypeDef.ssName = GetIdentifier(rSource);
            if (sSignalTypeDef.ssName.empty())
                throw SDbcParserException("Could not find signal type definition name.");
            auto itTypeDef = m_mapSigTypeDefs.find(sSignalTypeDef.ssName);
            if (itTypeDef != m_mapSigTypeDefs.end())
                throw SDbcParserException("Duplicate signal type definition with supplied name.");

            // Expect separator
            if (!ExpectChar(':', rSource))
                throw SDbcParserException("Colon ':' expected following 'SGTYPE_' signal type definition");

            // Get the signal type def - base part
            ReadSignalTypeDefBase(rSource, sSignalTypeDef);

            // Get the default value
            auto prDefaultVal = GetDouble(rSource);
            if (!prDefaultVal.second)
                throw SDbcParserException("Expecting the default value.");
            sSignalTypeDef.dDefaultValue = prDefaultVal.first;

            // Expect separator
            if (!ExpectChar(',', rSource))
                throw SDbcParserException("Comma ',' expected following default value in signal type definition");

            // Value table name
            sSignalTypeDef.ssValueTable = GetIdentifier(rSource);
            if (sSignalTypeDef.ssValueTable != "Vector__XXX" && sSignalTypeDef.ssValueTable != "Vector_XXX")
            {
                if (sSignalTypeDef.ssValueTable.empty())
                    throw SDbcParserException("Expecting a value table name.");
                if (m_mapValueTables.find(sSignalTypeDef.ssValueTable) == m_mapValueTables.end())
                    throw SDbcParserException("Value table not found.");
            }
            // Add the signal type definition
            m_mapSigTypeDefs.insert(std::make_pair(sSignalTypeDef.ssName, sSignalTypeDef));
        }

        // Expecting the ending
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected.");
    }

    void CDbcParser::ReadSignalGroupDef(CDbcSource& rSource)
    {
        // Get the message ID and the belonging message
        auto prMsgId = GetUInt(rSource);
        if (!prMsgId.second)
            throw SDbcParserException("No message ID supplied.");
        if (prMsgId.first == m_uiIndepMsgId)
            prMsgId.first = 0xffffffff;
        auto itMsgDef = m_mapMsgDefById.find(prMsgId.first);
        if (itMsgDef == m_mapMsgDefById.end())
            throw SDbcParserException("Could not find message with supplied ID.");

        // Get the signal group name
        SSignalGroupDef sSigGrpDef;
        sSigGrpDef.ssName = GetIdentifier(rSource);
        if (sSigGrpDef.ssName.empty()) throw SDbcParserException("Missing signal group name.");
        if (IsDbcIdentifier(sSigGrpDef.ssName)) throw SDbcParserException("Invalid signal group name.");
        if (itMsgDef->second->mapSigGroups.find(sSigGrpDef.ssName) != itMsgDef->second->mapSigGroups.end())
            throw SDbcParserException("Duplicate signal group definition.");

        // Get repetition
        auto prRepetition = GetUInt(rSource);
        if (!prRepetition.second)
            throw SDbcParserException("Expected repetition factor following the signal group name.");
        sSigGrpDef.uiRepetitions = prRepetition.first;

        // Expect separator
        if (!ExpectChar(':', rSource))
            throw SDbcParserException("Colon ':' expected following 'SIG_GROUP_' signal group definition");

        // Expect zero or more signal definitions
        do
        {
            // Check for semi-colon indicating the end of the definition.
            if (ExpectChar(';', rSource)) break;

            // Get the signal name
            std::string ssSignalName = GetIdentifier(rSource);
            if (ssSignalName.empty())
                throw SDbcParserException("Expecting a signal name.");
            auto itSignal = std::find_if(itMsgDef->second->vecSignals.begin(), itMsgDef->second->vecSignals.end(),
                [&](const SSignalDef& rsSignalDef)
                {
                    return rsSignalDef.ssName == ssSignalName;
                });
            if (itSignal == itMsgDef->second->vecSignals.end())
                throw SDbcParserException("Could not find signal with supplied name.");

            // Add the signal name.
            sSigGrpDef.vecSignals.push_back(ssSignalName);

            // DEVIATION: The specification doesn't specifies a comma
            ExpectChar(',', rSource);

        } while(true);

        // Add the group to the message
        itMsgDef->second->mapSigGroups.insert(std::make_pair(sSigGrpDef.ssName, sSigGrpDef));
    }

    void CDbcParser::ReadCommentDef(CDbcSource& rSource)
    {
        // Determine the entity the comment is for
        std::vector<std::string>* pvecComments = nullptr;
        std::string ssEntity = GetIdentifier(rSource);
        if (ssEntity == "BU_")
        {
            // Find the node
            std::string ssNodeName = GetIdentifier(rSource);
            auto itNode = m_mapNodes.find(ssNodeName);
            if (itNode == m_mapNodes.end())
                throw SDbcParserException("Invalid node name.");
            pvecComments = &itNode->second.vecComments;
        }
        else if (ssEntity == "BO_")
        {
            // Find the message
            auto prMsgId = GetUInt(rSource);
            if (!prMsgId.second)
                throw SDbcParserException("Expecting the message ID.");
            if (prMsgId.first == m_uiIndepMsgId)
                prMsgId.first = 0xffffffff;
            auto itMsgDef = m_mapMsgDefById.find(prMsgId.first);
            if (itMsgDef == m_mapMsgDefById.end())
                throw SDbcParserException("Could not find message with supplied ID.");
            pvecComments = &itMsgDef->second->vecComments;
        }
        else if (ssEntity == "SG_")
        {
            // Find the message
            auto prMsgId = GetUInt(rSource);
            if (!prMsgId.second)
                throw SDbcParserException("Expecting the message ID.");
            if (prMsgId.first == m_uiIndepMsgId)
                prMsgId.first = 0xffffffff;
            auto itMsgDef = m_mapMsgDefById.find(prMsgId.first);
            if (itMsgDef == m_mapMsgDefById.end())
                throw SDbcParserException("Could not find message with supplied ID.");

            // Find the signal
            std::string ssSignalName = GetIdentifier(rSource);
            auto itSignal = std::find_if(itMsgDef->second->vecSignals.begin(), itMsgDef->second->vecSignals.end(),
                [&](const SSignalDef& rsSignalDef)
                {
                    return rsSignalDef.ssName == ssSignalName;
                });
            if (itSignal == itMsgDef->second->vecSignals.end())
                throw SDbcParserException("Could not find signal with supplied name.");
            pvecComments = &itSignal->vecComments;
        }
        else if (ssEntity == "EV_")
        {
            // Find the environment variable
            std::string ssEnvVarName = GetIdentifier(rSource);
            if (ssEnvVarName.empty())
                throw SDbcParserException("Could not find message or environment variable.");
            auto itVarDef = m_mapEnvVars.find(ssEnvVarName);
            if (itVarDef == m_mapEnvVars.end())
                throw SDbcParserException("Could not find environment variable with supplied name.");
            pvecComments = &itVarDef->second.vecComments;
        }
        else
            pvecComments = &m_vecComments;  // Global comments

        // There should be a valid comments vector.
        if (!pvecComments)
            throw SDbcParserException("Internal error with comments vector.");

        // Get the comment
        auto prComment = GetString(rSource);
        if (!prComment.second)
            throw SDbcParserException("Mising comment string.");

        // Add comment to definition
        pvecComments->push_back(prComment.first);

        // Expect terminator
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected finalizing the comment 'CM_'");
    }

    void CDbcParser::ReadAttrDef(CDbcSource& rSource)
    {
        std::string ssEntity = GetIdentifier(rSource);
        SAttributeDef::EObjectType eObjType = SAttributeDef::EObjectType::global;
        if (ssEntity == "BU_")
            eObjType = SAttributeDef::EObjectType::node;
        else if (ssEntity == "BO_")
            eObjType = SAttributeDef::EObjectType::message;
        else if (ssEntity == "SG_")
            eObjType = SAttributeDef::EObjectType::signal;
        else if (ssEntity == "EV_")
            eObjType = SAttributeDef::EObjectType::envvar;
        else if (!ssEntity.empty())
            throw SDbcParserException("Invalid object type.");

        // Expect opening quote
        if (!ExpectChar('\"', rSource))
            throw SDbcParserException("Quote '\"' expected preceding attribute definition name");

        // Get the name
        std::string ssName = GetIdentifier(rSource);
        if (ssName.empty() || IsDbcIdentifier(ssName))
            throw SDbcParserException("Expecting a name for the attribute definition.");

        // Expect closing quote
        if (!ExpectChar('\"', rSource))
            throw SDbcParserException("Quote '\"' expected following attribute definition name");

        // Get the value type
        std::string ssValueType = GetIdentifier(rSource);
        TAttributeDefPtr ptrAttrDef = std::make_shared<SAttributeDef>(SAttributeDef::EType::integer);
        if (ssValueType == "INT")
        {
            auto prMin = GetInt(rSource);
            if (!prMin.second) throw SDbcParserException("Expecting minimum value for integer attribute definition.");
            ptrAttrDef->sIntValues.iMinimum = prMin.first;
            auto prMax = GetInt(rSource);
            if (!prMax.second) throw SDbcParserException("Expecting maximum value for integer attribute definition.");
            ptrAttrDef->sIntValues.iMaximum = prMax.first;
        } else if (ssValueType == "HEX")
        {
            ptrAttrDef = std::move(std::make_shared<SAttributeDef>(SAttributeDef::EType::hex_integer));
            auto prMin = GetUInt(rSource);
            if (!prMin.second) throw SDbcParserException("Expecting minimum value for hexadecimal attribute definition.");
            ptrAttrDef->sHexValues.uiMinimum = prMin.first;
            auto prMax = GetUInt(rSource);
            if (!prMax.second) throw SDbcParserException("Expecting maximum value for hexadecimal attribute definition.");
            ptrAttrDef->sHexValues.uiMaximum = prMax.first;
        } else if (ssValueType == "FLOAT")
        {
            ptrAttrDef = std::move(std::make_shared<SAttributeDef>(SAttributeDef::EType::floating_point));
            auto prMin = GetDouble(rSource);
            if (!prMin.second) throw SDbcParserException("Expecting minimum value for floating point attribute definition.");
            ptrAttrDef->sFltValues.dMinimum = prMin.first;
            auto prMax = GetDouble(rSource);
            if (!prMax.second) throw SDbcParserException("Expecting maximum value for floating point attribute definition.");
            ptrAttrDef->sFltValues.dMaximum = prMax.first;
        } else if (ssValueType == "STRING")
        {
            ptrAttrDef = std::move(std::make_shared<SAttributeDef>(SAttributeDef::EType::string));
        } else if (ssValueType == "ENUM")
        {
            ptrAttrDef = std::move(std::make_shared<SAttributeDef>(SAttributeDef::EType::enumerator));

            // Zero or more strings could available
            bool bInitial = true;
            do
            {
                // Try getting the enum entry string
                auto prEnumEntry = GetString(rSource);
                if (!bInitial && !prEnumEntry.second)
                    throw SDbcParserException("Expecting an enumerator entry string.");
                bInitial = false;

                // Not used or not applicable entries are added, but can be redundant.
                // The use of "n/a" and "not-used" is not part of the official standard.
                if (prEnumEntry.first != "not-used" && prEnumEntry.first != "n/a")
                {
                    if (std::find(ptrAttrDef->sEnumValues.vecEnumValues.begin(), ptrAttrDef->sEnumValues.vecEnumValues.end(),
                        prEnumEntry.first) != ptrAttrDef->sEnumValues.vecEnumValues.end())
                        throw SDbcParserException("Duplicate enumerator entry string.");
                }

                // Add the value.
                ptrAttrDef->sEnumValues.vecEnumValues.push_back(prEnumEntry.first);

                // Another entry if a comma is provided
                if (!ExpectChar(',', rSource)) break;
            } while (true);
        } else
            throw SDbcParserException("Invalid attribute definition value type.");

        // Add the attribute definition
        ptrAttrDef->eObjType = eObjType;
        ptrAttrDef->ssName = ssName;
        m_mapAttrDefs.insert_or_assign(ssName, std::move(ptrAttrDef));

        // Expect terminator
        SkipWhitespace(rSource);
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected finalizing the attribute definition 'AB_DEF_'");
    }

    void CDbcParser::ReadAttrDefaultDef(CDbcSource& rSource)
    {
        // Expect opening quote
        if (!ExpectChar('\"', rSource))
            throw SDbcParserException("Quote '\"' expected preceding attribute definition name");

        // Get the name
        std::string ssName = GetIdentifier(rSource);
        auto itAttrDef = m_mapAttrDefs.find(ssName);
        if (itAttrDef == m_mapAttrDefs.end())
            throw SDbcParserException("Cannot find attribute definition with supplied name.");

        // Expect closing quote
        if (!ExpectChar('\"', rSource))
            throw SDbcParserException("Quote '\"' expected following attribute definition name");

        // Get the value
        switch (itAttrDef->second->eType)
        {
        case SAttributeDef::EType::integer:
        {
            // Some definitions use a float for defining an integer.
            auto prFloat = GetDouble(rSource);
            if (!prFloat.second)
                throw SDbcParserException("Expecting an integer value for the default value of the attribute definition");
            itAttrDef->second->sIntValues.iDefault = static_cast<int32_t>(prFloat.first);
            break;
        }
        case SAttributeDef::EType::hex_integer:
        {
            // Some definitions use a float for defining an integer.
            auto prFloat = GetDouble(rSource);
            if (!prFloat.second)
                throw SDbcParserException("Expecting an unsigned integer value for the default value of the attribute definition");
            if (prFloat.first < 0.0)
                throw SDbcParserException("Expecting an unsigned integer value for the default value of the attribute definition");
            itAttrDef->second->sHexValues.uiDefault = static_cast<uint32_t>(prFloat.first);
            break;
        }
        case SAttributeDef::EType::floating_point:
        {
            auto prFlt = GetDouble(rSource);
            if (!prFlt.second)
                throw SDbcParserException("Expecting a floating point value for the default value of the attribute definition");
            itAttrDef->second->sFltValues.dDefault = prFlt.first;
            break;
        }
        case SAttributeDef::EType::string:
        {
            auto prStr = GetString(rSource);
            if (!prStr.second)
                throw SDbcParserException("Expecting a string value for the default value of the attribute definition");
            itAttrDef->second->sStringValues.ssDefault = std::move(prStr.first);
            break;
        }
        case SAttributeDef::EType::enumerator:
        {
            auto prStr = GetString(rSource);
            if (!prStr.second)
            {
                auto prInt = GetUInt(rSource);
                if (!prInt.second)
                    throw SDbcParserException("Expecting a string value for the default value of the attribute definition");
                if (prInt.first >= itAttrDef->second->sEnumValues.vecEnumValues.size())
                    throw SDbcParserException("Default value out of range for the attribute definition");
                itAttrDef->second->sEnumValues.ssDefault = itAttrDef->second->sEnumValues.vecEnumValues[prInt.first];
            }
            else
            {
                // Non-documented extension: check the enum values without case consideration.
                if (std::find_if(itAttrDef->second->sEnumValues.vecEnumValues.begin(), itAttrDef->second->sEnumValues.vecEnumValues.end(),
                    [&](const std::string& rss) {return internal::EqualNC(rss, prStr.first);}) == itAttrDef->second->sEnumValues.vecEnumValues.end())
                    throw SDbcParserException("The enum value doesn't fit the list of the predefined values.");
                itAttrDef->second->sEnumValues.ssDefault = std::move(prStr.first);
            }
            break;
        }
        default:
            throw SDbcParserException("Internal error: invalid attribute definition value type.");
            break;
        }

        // Expect terminator
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected finalizing the attribute definition 'AB_DEF_DEF_'");
    }

    void CDbcParser::ReadAttributes(CDbcSource& rSource)
    {
        // Expect opening quote
        if (!ExpectChar('\"', rSource))
            throw SDbcParserException("Quote '\"' expected preceding attribute definition name");

        // Get the name
        std::string ssName = GetIdentifier(rSource);
        auto itAttrDef = m_mapAttrDefs.find(ssName);
        if (itAttrDef == m_mapAttrDefs.end())
            throw SDbcParserException("Cannot find attribute definition with supplied name.");

        // Expect closing quote
        if (!ExpectChar('\"', rSource))
            throw SDbcParserException("Quote '\"' expected following attribute definition name");

        std::string ssEntity = GetIdentifier(rSource);
        SAttributeDef::EObjectType eObjType = SAttributeDef::EObjectType::global;
        std::vector<SAttributeValue>* pAttrVals = nullptr;
        if (ssEntity == "BU_")
        {
            eObjType = SAttributeDef::EObjectType::node;

            // Get the node definition
            std::string ssNode = GetIdentifier(rSource);
            auto itNode = m_mapNodes.find(ssNode);
            if (itNode == m_mapNodes.end())
                throw SDbcParserException("Cannot find node with supplied name.");
            pAttrVals = &itNode->second.vecAttributes;
        }
        else if (ssEntity == "BO_")
        {
            eObjType = SAttributeDef::EObjectType::message;

            // Get the message definition
            auto prMsgId = GetUInt(rSource);
            if (!prMsgId.second)
                throw SDbcParserException("Expecting the message ID.");
            if (prMsgId.first == m_uiIndepMsgId)
                prMsgId.first = 0xffffffff;
            auto itMsgDef = m_mapMsgDefById.find(prMsgId.first);
            if (itMsgDef == m_mapMsgDefById.end())
                throw SDbcParserException("Could not find message with supplied ID.");
            pAttrVals = &itMsgDef->second->vecAttributes;
        }
        else if (ssEntity == "SG_")
        {
            eObjType = SAttributeDef::EObjectType::signal;

            // Get the message definition
            auto prMsgId = GetUInt(rSource);
            if (!prMsgId.second)
                throw SDbcParserException("Expecting the message ID.");
            if (prMsgId.first == m_uiIndepMsgId)
                prMsgId.first = 0xffffffff;
            auto itMsgDef = m_mapMsgDefById.find(prMsgId.first);
            if (itMsgDef == m_mapMsgDefById.end())
                throw SDbcParserException("Could not find message with supplied ID.");

            // Get the signal definition
            std::string ssSignalDef = GetIdentifier(rSource);
            auto itSignal = std::find_if(itMsgDef->second->vecSignals.begin(), itMsgDef->second->vecSignals.end(),
                [&](const SSignalDef& rsSignalDef)
                {
                    return rsSignalDef.ssName == ssSignalDef;
                });
            if (itSignal == itMsgDef->second->vecSignals.end())
                throw SDbcParserException("Could not find signal with supplied name.");
            pAttrVals = &itSignal->vecAttributes;
        }
        else if (ssEntity == "EV_")
        {
            eObjType = SAttributeDef::EObjectType::envvar;

            // Get the environment variable definition
            std::string ssEnvVarDef = GetIdentifier(rSource);
            auto itEnvVarDef = m_mapEnvVars.find(ssEnvVarDef);
            if (itEnvVarDef == m_mapEnvVars.end())
                throw SDbcParserException("Cannot find environment variable definition with supplied name.");
            pAttrVals = &itEnvVarDef->second.vecAttributes;
        }
        else if (ssEntity.empty())
        {
            eObjType = SAttributeDef::EObjectType::global;

            // Global entity
            pAttrVals = &m_vecAttributes;
        }

        // Check whether the object types match
        if (itAttrDef->second->eObjType != eObjType)
            throw SDbcParserException("Object type of attribute value is not fitting the object type of the attribute definition.");

        // Get the attribute value
        SAttributeValue sAttrVal(itAttrDef->second);
        switch (itAttrDef->second->eType)
        {
        case SAttributeDef::EType::integer:
        {
            // Some definitions use a float for defining an integer.
            auto prFloat = GetDouble(rSource);
            if (!prFloat.second)
                throw SDbcParserException("Expecting an integer value for the default value of the attribute definition");
            sAttrVal.iValue = static_cast<int32_t>(prFloat.first);
            break;
        }
        case SAttributeDef::EType::hex_integer:
        {
            // Some definitions use a float for defining an integer.
            auto prFloat = GetDouble(rSource);
            if (!prFloat.second)
                throw SDbcParserException("Expecting an unsigned integer value for the default value of the attribute definition");
            if (prFloat.first < 0.0)
                throw SDbcParserException("Expecting an unsigned integer value for the default value of the attribute definition");
            sAttrVal.uiValue = static_cast<uint32_t>(prFloat.first);
            break;
        }
        case SAttributeDef::EType::floating_point:
        {
            auto prFlt = GetDouble(rSource);
            if (!prFlt.second)
                throw SDbcParserException("Expecting a floating point value for the default value of the attribute definition");
            sAttrVal.dValue = prFlt.first;
            break;
        }
        case SAttributeDef::EType::string:
        {
            auto prStr = GetString(rSource);
            if (!prStr.second)
                throw SDbcParserException("Expecting a string value for the default value of the attribute definition");
            sAttrVal.ssValue = std::move(prStr.first);
            break;
        }
        case SAttributeDef::EType::enumerator:
        {
            auto prStr = GetString(rSource);
            if (!prStr.second)
            {
                auto prInt = GetUInt(rSource);
                if (!prInt.second)
                    throw SDbcParserException("Expecting a string value for the default value of the attribute definition");
                if (prInt.first >= itAttrDef->second->sEnumValues.vecEnumValues.size())
                    throw SDbcParserException("Default value out of range for the attribute definition");
                sAttrVal.ssValue = itAttrDef->second->sEnumValues.vecEnumValues[prInt.first];
            }
            else
            {
                if (std::find(itAttrDef->second->sEnumValues.vecEnumValues.begin(), itAttrDef->second->sEnumValues.vecEnumValues.end(),
                    prStr.first) == itAttrDef->second->sEnumValues.vecEnumValues.end())
                    throw SDbcParserException("The enum value doesn't fit the list of the predefined values.");
                sAttrVal.ssValue = std::move(prStr.first);
            }
            break;
        }
        default:
            throw SDbcParserException("Internal error: invalid attribute definition value type.");
            break;
        }

        // Add the value
        if (!pAttrVals) throw SDbcParserException("Internal error: attribute value vector not existing.");
        pAttrVals->push_back(std::move(sAttrVal));

        // Expect terminator
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected finalizing the attribute value 'AB_'");
    }

    void CDbcParser::ReadSignalMultiplexDef(CDbcSource& rSource)
    {
        // Get the message definition
        auto prMsgId = GetUInt(rSource);
        if (!prMsgId.second)
            throw SDbcParserException("Expecting the message ID.");
        if (prMsgId.first == m_uiIndepMsgId)
            prMsgId.first = 0xffffffff;
        auto itMsgDef = m_mapMsgDefById.find(prMsgId.first);
        if (itMsgDef == m_mapMsgDefById.end())
            throw SDbcParserException("Could not find message with supplied ID.");

        // Get the signal definition for the multiplexed signal
        std::string ssMultiplexedSignalDef = GetIdentifier(rSource);
        auto itMultiplexedSignal = std::find_if(itMsgDef->second->vecSignals.begin(), itMsgDef->second->vecSignals.end(),
            [&](const SSignalDef& rsSignalDef)
            {
                return rsSignalDef.ssName == ssMultiplexedSignalDef;
            });
        if (itMultiplexedSignal == itMsgDef->second->vecSignals.end())
            throw SDbcParserException("Could not find multiplexed signal with supplied name.");

        // Get the signal definition for the multiplexor signal
        std::string ssMultiplexorSignalDef = GetIdentifier(rSource);
        auto itMultiplexorSignal = std::find_if(itMsgDef->second->vecSignals.begin(), itMsgDef->second->vecSignals.end(),
            [&](const SSignalDef& rsSignalDef)
            {
                return rsSignalDef.ssName == ssMultiplexorSignalDef;
            });
        if (itMultiplexorSignal == itMsgDef->second->vecSignals.end())
            throw SDbcParserException("Could not find multiplexor signal with supplied name.");
        itMultiplexorSignal->uiMultiplexBitmask |= static_cast<uint32_t>(SSignalDef::EMultiplexBitmask::mltplx_switch);

        // Create the information structure
        SSignalDef::SExtendedMultiplex sExtMultiplex{ *itMultiplexorSignal, {} };

        // Read zero or more ranges
        do
        {
            auto itLow = GetUInt(rSource);
            if (!itLow.second) break;
            if (!ExpectChar('-', rSource))
                throw SDbcParserException("Expecting dash '-' between low and and high range value for a multiplexed signal.");
            auto itHigh = GetUInt(rSource);
            if (!itHigh.second) throw SDbcParserException("Missing high range value for a multiplexed signal.");
            sExtMultiplex.vecRanges.push_back(std::make_pair(itLow.first, itHigh.first));

            // DEVIATION: The specification doesn't specifies a comma; the example does
            ExpectChar(',', rSource);
        } while (true);

        // Add the multiplexed structure to the multiplexed signal
        itMultiplexedSignal->vecExtMultiplex.push_back(std::move(sExtMultiplex));

        // Expect terminator
        if (!ExpectChar(';', rSource))
            throw SDbcParserException("Semi-colon ';' expected finalizing the extended multiplexer definition 'SG_MUL_VAL_'");
    }
} // namespace dbc

#ifdef _MSC_VER
    #pragma warning(pop)
#endif
