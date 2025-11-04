#include <algorithm>
#include "parser_node_toml.h"
#include "exception.h"
#include <sstream>

size_t FindFirst(const std::string& rss, const std::string& rssSeparator /*= "."*/)
{
    enum class EType {normal, single_quoted_string, double_quoted_string} eType = EType::normal;
    size_t nPos = 0;
    while (nPos < rss.size())
    {
        switch (rss[nPos])
        {
        case '\'':
            if (eType == EType::normal)
                eType = EType::single_quoted_string;
            else if (eType == EType::single_quoted_string)
                eType = EType::normal;
            break;
        case '\"':
            if (eType == EType::normal)
                eType = EType::double_quoted_string;
            else if (eType == EType::double_quoted_string)
                eType = EType::normal;
            break;
        case '\\':
            nPos++;
            break;
        default:
            if (eType == EType::normal && rssSeparator.find(rss[nPos]) != std::string::npos)
                return nPos;
            break;
        }
		nPos++;
    }
    return nPos >= rss.size() ? std::string::npos : nPos;
}

size_t FindLast(const std::string& rss, const std::string& rssSeparator /*= "."*/)
{
    enum class EType {normal, single_quoted_string, double_quoted_string} eType = EType::normal;
    size_t nPos = rss.size();
    while (nPos)
    {
        nPos--;
        bool bEscaped = nPos && rss[nPos - 1] == '\\';
        switch (rss[nPos])
        {
        case '\'':
            if (bEscaped)
                nPos--;
            else if (eType == EType::normal)
                eType = EType::single_quoted_string;
            else if (eType == EType::single_quoted_string)
                eType = EType::normal;
            break;
        case '\"':
            if (bEscaped)
                nPos--;
            else if (eType == EType::normal)
                eType = EType::double_quoted_string;
            else if (eType == EType::double_quoted_string)
                eType = EType::normal;
            break;
        default:
            if (eType == EType::normal && rssSeparator.find(rss[nPos]) != std::string::npos)
                return nPos;
            break;
        }
    }
    return std::string::npos;
}

bool CompareEqual(const std::string& rss1, const std::string& rss2)
{
    size_t nStart1 = 0, nStop1 = rss1.size();
    if (rss1.size() && rss1.find_first_of("\"\'") == 0 && rss1.find_last_of("\"\'") == (rss1.size() - 1))
    {
        nStart1++;
        nStop1--;
    }
    size_t nStart2 = 0, nStop2 = rss2.size();
    if (rss2.size() && rss2.find_first_of("\"\'") == 0 && rss2.find_last_of("\"\'") == (rss2.size() - 1))
    {
        nStart2++;
        nStop2--;
    }

    if (nStop1 - nStart1 != nStop2 - nStart2) return false;
    for (size_t n = 0; n < (nStop1 - nStart1); n++)
    {
        if (rss1[nStart1 + n] != rss2[nStart2 + n])
            return false;
    }
    return true;
}

std::string EscapeString(const std::string& rssString, const char cQuoteType /*= '\"'*/)
{
    // Iterate through the string
    std::stringstream sstream;
    size_t nPos = 0;
    uint32_t uiUTFChar = 0;
    while (nPos < rssString.size())
    {
        uint8_t uiChar = static_cast<uint8_t>(rssString[nPos]);
        switch (uiChar)
        {
        case '\a':          sstream << "\\a";         break;
        case '\b':          sstream << "\\b";         break;
        case '\f':          sstream << "\\f";         break;
        case '\n':          sstream << "\\n";         break;
        case '\r':          sstream << "\\r";         break;
        case '\t':          sstream << "\\t";         break;
        case '\v':          sstream << "\\v";         break;
        case '\\':          sstream << "\\\\";         break;
        case '\'':          if (static_cast<uint8_t>(cQuoteType) == uiChar) sstream << "\\"; sstream << "\'";  break;
        case '\"':          if (static_cast<uint8_t>(cQuoteType) == uiChar) sstream << "\\"; sstream << "\"";  break;
        default:
            if (uiChar >= 0x20 && uiChar < 0x7f)
            {
                // Standard ASCII
                sstream << static_cast<char>(uiChar);
                break;
            }
            else if (uiChar <= 0x80)  // One byte UTF-8
                uiUTFChar = static_cast<uint32_t>(uiChar);
            else if (uiChar <= 0xDF) // Two bytes UTF-8
            {
                uiUTFChar = static_cast<size_t>(uiChar & 0b00011111) << 6;

                // Expecting the next character to be between 0x80 and 0xBF
                nPos++;
                if (nPos >= rssString.size()) break;
                uiUTFChar |= static_cast<size_t>(rssString[nPos] & 0b00111111);
            }
            else if (uiChar <= 0xEF) // Three bytes UTF-8
            {
                uiUTFChar = static_cast<size_t>(uiChar & 0b00001111) << 6;

                // Expecting the next character to be between 0x80 and 0xBF
                nPos++;
                if (nPos >= rssString.size()) break;
                uiUTFChar |= static_cast<size_t>(rssString[nPos] & 0b00111111);
                uiUTFChar <<= 6;

                // Expecting the next character to be between 0x80 and 0xBF
                nPos++;
                if (nPos >= rssString.size()) break;
                uiUTFChar |= static_cast<size_t>(rssString[nPos] & 0b00111111);
            }
            else if (uiChar <= 0xF7) // Four bytes UTF-8
            {
                uiUTFChar = static_cast<size_t>(uiChar & 0b00000111) << 6;

                // Expecting the next character to be between 0x80 and 0xBF
                nPos++;
                if (nPos >= rssString.size()) break;
                uiUTFChar |= static_cast<size_t>(rssString[nPos] & 0b00111111);
                uiUTFChar <<= 6;

                // Expecting the next character to be between 0x80 and 0xBF
                nPos++;
                if (nPos >= rssString.size()) break;
                uiUTFChar |= static_cast<size_t>(rssString[nPos] & 0b00111111);
                uiUTFChar <<= 6;

                // Expecting the next character to be between 0x80 and 0xBF
                nPos++;
                if (nPos >= rssString.size()) break;
                uiUTFChar |= static_cast<size_t>(rssString[nPos] & 0b00111111);
            }

            // Stream the UTF character
            if (uiUTFChar <= 0xFFFF)
                sstream << "\\u" << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << uiUTFChar;
            else
                sstream << "\\U" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << uiUTFChar;
            break;
        }
        nPos++;
    }
    return sstream.str();
}


CNode::CNode(const std::string& rssName) : m_ssName(rssName)
{}

CNode::~CNode()
{}

sdv::u8string CNode::GetName() const
{
    return m_ssName;
}

sdv::any_t CNode::GetValue() const
{
    return sdv::any_t();
}

sdv::u8string CNode::GetTOML() const
{
    std::string ssLastTable;
    std::string ssParent;
    return CreateTOMLText(ssParent, ssLastTable, true, false, true, true);
}

std::shared_ptr<const CArray> CNode::GetArray() const
{
    if (!dynamic_cast<const CArray*>(this)) return {};
    return std::static_pointer_cast<const CArray>(shared_from_this());
}

std::shared_ptr<CArray> CNode::GetArray()
{
    if (!dynamic_cast<CArray*>(this)) return {};
    return std::static_pointer_cast<CArray>(shared_from_this());
}

std::shared_ptr<const CTable> CNode::GetTable() const
{
    if (!dynamic_cast<const CTable*>(this)) return {};
    return std::static_pointer_cast<const CTable>(shared_from_this());
}

std::shared_ptr<CTable> CNode::GetTable()
{
    if (!dynamic_cast<const CTable*>(this)) return {};
    return std::static_pointer_cast<CTable>(shared_from_this());
}

std::weak_ptr<const CNode> CNode::GetParent() const
{
    return m_ptrParent;
}

void CNode::SetParent(const std::shared_ptr<CNode>& rptrParent)
{
    m_ptrParent = rptrParent;
}

std::shared_ptr<CNode> CNode::Find(const std::string& /*rssPath*/) const
{
    return std::shared_ptr<CNode>();
}

std::shared_ptr<CNode> CNode::GetDirect(const std::string& /*rssPath*/) const
{
    // The CNode implementation doesn't have any children. Therefore there is nothing to get.
    return std::shared_ptr<CNode>();
}

std::string CNode::CreateTOMLText(const std::string& rssParent /*= std::string()*/) const
{
    std::string ssLastTable;
    return CreateTOMLText(rssParent, ssLastTable);
}

void CNode::Add(const std::string& rssPath, const std::shared_ptr<CNode>& /*rptrNode*/, bool /*bDefinedExplicitly = true*/)
{
    throw XTOMLParseException(("Not allowed to add '" + rssPath + "'; parent node is final").c_str());
}

CBooleanNode::CBooleanNode(const std::string& rssName, bool bVal) : CNode(rssName), m_bVal(bVal)
{}

sdv::toml::ENodeType CBooleanNode::GetType() const
{
    return sdv::toml::ENodeType::node_boolean;
}

sdv::any_t CBooleanNode::GetValue() const
{
    return sdv::any_t(m_bVal);
}

std::string CBooleanNode::CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
    bool bAssignment, bool /*bRoot*/) const
{
    std::stringstream sstreamEntry;

    // Do we need to start a table?
    if (!bEmbedded && bFirst && bAssignment && rssParent != rssLastPrintedTable)
    {
        sstreamEntry << std::endl << "[" << rssParent << "]" << std::endl;
        rssLastPrintedTable = rssParent;
    }

    if (bEmbedded && !bFirst)     // 2nd or higher array entry
        sstreamEntry << ", ";
    if (!bEmbedded || bAssignment)     // Not an array entry
        sstreamEntry << GetName() << " = ";
    sstreamEntry << (m_bVal ? "true" : "false");
    if (!bEmbedded)     // Not an array entry
        sstreamEntry << std::endl;

    return sstreamEntry.str();
}

CIntegerNode::CIntegerNode(const std::string& rssName, int64_t iVal) : CNode(rssName), m_iVal(iVal)
{}

sdv::toml::ENodeType CIntegerNode::GetType() const
{
    return sdv::toml::ENodeType::node_integer;
}

sdv::any_t CIntegerNode::GetValue() const
{
    return sdv::any_t(m_iVal);
}

std::string CIntegerNode::CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
    bool bAssignment, bool /*bRoot*/) const
{
    std::stringstream sstreamEntry;

    // Do we need to start a table?
    if (!bEmbedded && bFirst && bAssignment && rssParent != rssLastPrintedTable)
    {
        sstreamEntry << std::endl << "[" << rssParent << "]" << std::endl;
        rssLastPrintedTable = rssParent;
    }

    if (bEmbedded && !bFirst)     // 2nd or higher array entry
        sstreamEntry << ", ";
    if (!bEmbedded || bAssignment)     // Not an array entry
        sstreamEntry << GetName() << " = ";
    sstreamEntry << m_iVal;
    if (!bEmbedded)     // Not an array entry
        sstreamEntry << std::endl;

    return sstreamEntry.str();
}

CFloatingPointNode::CFloatingPointNode(const std::string& rssName, double dVal) : CNode(rssName), m_dVal(dVal)
{}

sdv::toml::ENodeType CFloatingPointNode::GetType() const
{
    return sdv::toml::ENodeType::node_floating_point;
}

sdv::any_t CFloatingPointNode::GetValue() const
{
    return sdv::any_t(m_dVal);
}

std::string CFloatingPointNode::CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst,
    bool bEmbedded, bool bAssignment, bool /*bRoot*/) const
{
    std::stringstream sstreamEntry;

    // Do we need to start a table?
    if (!bEmbedded && bFirst && bAssignment && rssParent != rssLastPrintedTable)
    {
        sstreamEntry << std::endl << "[" << rssParent << "]" << std::endl;
        rssLastPrintedTable = rssParent;
    }

    if (bEmbedded && !bFirst)     // 2nd or higher array entry
        sstreamEntry << ", ";
    if (!bEmbedded || bAssignment)     // Not an array entry
        sstreamEntry << GetName() << " = ";
    sstreamEntry << std::setprecision(15) << std::defaultfloat << m_dVal;
    if (!bEmbedded)     // Not an array entry
        sstreamEntry << std::endl;

    return sstreamEntry.str();
}

CStringNode::CStringNode(const std::string& rssName, const std::string& rssVal) : CNode(rssName), m_ssVal(rssVal)
{}

sdv::toml::ENodeType CStringNode::GetType() const
{
    return sdv::toml::ENodeType::node_string;
}

sdv::any_t CStringNode::GetValue() const
{
    return sdv::any_t(m_ssVal);
}

std::string CStringNode::CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
    bool bAssignment, bool /*bRoot*/) const
{
    std::stringstream sstreamEntry;

    // Do we need to start a table?
    if (!bEmbedded && bFirst && bAssignment && rssParent != rssLastPrintedTable)
    {
        sstreamEntry << std::endl << "[" << rssParent << "]" << std::endl;
        rssLastPrintedTable = rssParent;
    }

    if (bEmbedded && !bFirst)     // 2nd or higher array entry
        sstreamEntry << ", ";
    if (!bEmbedded || bAssignment)     // Not an array entry
        sstreamEntry << GetName() << " = ";
    sstreamEntry << "\"" << EscapeString(m_ssVal) << "\"";
    if (!bEmbedded)     // Not an array entry
        sstreamEntry << std::endl;

    return sstreamEntry.str();
}

CNodeCollection::CNodeCollection(const std::string& rssName) : CNode(rssName)
{}

uint32_t CNodeCollection::GetCount() const
{
    return static_cast<uint32_t>(m_vecContent.size());
}

sdv::IInterfaceAccess* CNodeCollection::GetNode(/*in*/ uint32_t uiIndex) const
{
    auto ptrNode = Get(uiIndex);
    return static_cast<sdv::IInterfaceAccess*>(ptrNode.get());
}

std::shared_ptr<CNode> CNodeCollection::Get(uint32_t uiIndex) const
{
    if (static_cast<size_t>(uiIndex) >= m_vecContent.size()) return nullptr;

    return m_vecContent[uiIndex];
}

sdv::IInterfaceAccess* CNodeCollection::GetNodeDirect(/*in*/ const sdv::u8string& ssPath) const
{
    auto ptrNode = GetDirect(ssPath);
    return static_cast<sdv::IInterfaceAccess*>(ptrNode.get());
}

bool CNodeCollection::AddElement(const std::shared_ptr<CNode>& rptrNode, bool bUnique /*= false*/)
{
    if (!rptrNode) return false;
    if (bUnique && std::find_if(m_vecContent.begin(), m_vecContent.end(), [&](const std::shared_ptr<CNode>& rptrNodeEntry)
        {
            return CompareEqual(rptrNodeEntry->GetName(), rptrNode->GetName());
        }) != m_vecContent.end()) return false;
    m_vecContent.push_back(rptrNode);
    return true;
}

CTable::CTable(const std::string& rssName) : CNodeCollection(rssName)
{}

sdv::toml::ENodeType CTable::GetType() const
{
    return sdv::toml::ENodeType::node_table;
}

std::shared_ptr<CNode> CTable::GetDirect(const std::string& rssPath) const
{
    size_t nSeparator = FindFirst(rssPath, ".[");
    std::string ssKey = rssPath.substr(0, nSeparator);
    std::shared_ptr<CNode> ptrNode;
    for (uint32_t uiIndex = 0; !ptrNode && uiIndex < GetCount(); uiIndex++)
    {
        std::shared_ptr<CNode> ptrNodeEntry = Get(uiIndex);
        if (!ptrNodeEntry) continue;
        if (CompareEqual(ptrNodeEntry->GetName(), ssKey)) ptrNode = ptrNodeEntry;
    }
    if (!ptrNode) return ptrNode;   // Not found

    // Done?
    if (nSeparator == std::string::npos) return ptrNode;

    // There is more...
    if (rssPath[nSeparator] == '.') nSeparator++;   // Skip dot
    return ptrNode->GetDirect(rssPath.substr(nSeparator));
}

std::string CTable::CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
    bool bAssignment, bool bRoot) const
{
    // Create the full name
    std::string ssFullName = rssParent;
    if (!GetName().empty() && !bRoot)
    {
        if (!ssFullName.empty()) ssFullName += ".";
        ssFullName += GetName();
    }

    // Stream the table
    std::stringstream sstreamEntry;
    if (bEmbedded && !bFirst)     // 2nd or higher array entry
        sstreamEntry << ", ";
    if (bEmbedded)    // Embedded table in an array
        sstreamEntry << "{";
    for (uint32_t uiIndex = 0; uiIndex < GetCount(); uiIndex++)
    {
        std::shared_ptr<CNode> ptrNode = Get(uiIndex);
        if (!ptrNode) continue;
        sstreamEntry << ptrNode->CreateTOMLText(ssFullName, rssLastPrintedTable, uiIndex == 0, bEmbedded);
    }
    if (bEmbedded)    // Embedded table in an array
        sstreamEntry << "}";
    if (!bEmbedded && !bAssignment && !GetName().empty())
        sstreamEntry << std::endl;

    return sstreamEntry.str();
}

void CTable::Add(const std::string& rssPath, const std::shared_ptr<CNode>& rptrNode, bool bDefinedExplicitly)
{
    if (!m_bOpenToAddChildren)
        throw XTOMLParseException(("Not allowed to add '" + rssPath + "'; parent node is final").c_str());

    size_t nDotPos = FindFirst(rssPath);
    std::string ssFirst = rssPath.substr(0, nDotPos);
    std::string ssSecond = nDotPos == std::string::npos ? "" : rssPath.substr(nDotPos + 1);
    auto ptrParent = Find(ssFirst);

    // Element does not already exist at given path
    if (!ptrParent)
    {
        // Add the new element as a direct child
        if (nDotPos == std::string::npos)
        {
            rptrNode->SetParent(shared_from_this());
            GetTable()->AddElement(rptrNode);
            return;
        }
        // Add the new element as a descendant further down
        auto ptrIntermediateTable = std::make_shared<CNormalTable>(ssFirst);
        ptrIntermediateTable->SetParent(shared_from_this());
        ptrIntermediateTable->m_bDefinedExplicitly = bDefinedExplicitly;
        GetTable()->AddElement(ptrIntermediateTable);
        static_cast<CNode*>(ptrIntermediateTable.get())->Add(ssSecond, rptrNode, bDefinedExplicitly);
        return;
    }
    if (dynamic_cast<CTableArray*>(ptrParent.get()) && nDotPos == std::string::npos)
    {
        ptrParent->Add(ssFirst, rptrNode, bDefinedExplicitly);
        return;
    }
    // Element already exists but would be inserted as a direct child
    if (nDotPos == std::string::npos)
    {
        // Make an already implicitly defined table explicitly defined
        if (ptrParent->GetType() == rptrNode->GetType() && dynamic_cast<CNodeCollection*>(ptrParent.get()) &&
            !static_cast<CNodeCollection*>(ptrParent.get())->m_bDefinedExplicitly)
        {
            static_cast<CNodeCollection*>(ptrParent.get())->m_bDefinedExplicitly = true;
            return;
        }
        throw XTOMLParseException(("Name '" + ssFirst + "' already exists").c_str());
    }
    // Element already exists and new element would be added as descendant
    ptrParent->Add(ssSecond, rptrNode, bDefinedExplicitly);
}

std::shared_ptr<CNode> CTable::Find(const std::string& rssPath) const
{
    size_t nDotPos = FindFirst(rssPath);
    std::string ssFirst  = rssPath.substr(0, nDotPos);
    std::string ssSecond = nDotPos == std::string::npos ? "" : rssPath.substr(nDotPos + 1);

    for (uint32_t uiIndex = 0; uiIndex < GetCount(); uiIndex++)
    {
        std::shared_ptr<CNode> ptrNode;
        ptrNode = Get(uiIndex);
        if (!ptrNode) continue;
        if (CompareEqual(ptrNode->GetName(), ssFirst))
        {
            if (nDotPos == std::string::npos)
                return ptrNode;
            return ptrNode->Find(ssSecond);
        }
    }

    // No node found...
    return std::shared_ptr<CNode>();
}

CArray::CArray(const std::string& rssName) : CNodeCollection(rssName)
{}

sdv::toml::ENodeType CArray::GetType() const
{
    return sdv::toml::ENodeType::node_array;
}

std::shared_ptr<CNode> CArray::GetDirect(const std::string& rssPath) const
{
    size_t nIndexBegin = FindFirst(rssPath, "[");
    if (nIndexBegin == std::string::npos) return std::shared_ptr<CNode>();    // Unexpected

    size_t nIndexEnd = rssPath.find_first_not_of("0123456789", nIndexBegin + 1);
    if (nIndexEnd == std::string::npos) return std::shared_ptr<CNode>();    // Unexpected
    if (rssPath[nIndexEnd] != ']') return std::shared_ptr<CNode>();    // Unexpected
    std::string ssIndex = rssPath.substr(nIndexBegin + 1, nIndexEnd - nIndexBegin - 1);
    if (ssIndex.empty()) return std::shared_ptr<CNode>();    // Unexpected
    uint32_t uiIndex = std::atol(ssIndex.c_str());
    nIndexEnd++;

    // Get the node
    if (uiIndex >= GetCount()) return std::shared_ptr<CNode>(); // Not found
    std::shared_ptr<CNode> ptrNode = Get(uiIndex);

    // Done?
    if (nIndexEnd == rssPath.size()) return ptrNode;

    // Expecting a dot?
    size_t nSeparator = nIndexEnd;
    if (rssPath[nSeparator] == '.') nSeparator++;   // Skip dot
    return ptrNode->GetDirect(rssPath.substr(nSeparator));
}

std::string CArray::CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool bFirst, bool bEmbedded,
    bool bAssignment, bool /*bRoot*/) const
{
    std::stringstream sstreamEntry;

    // Do we need to start a table?
    if (!bEmbedded && bFirst && bAssignment && rssParent != rssLastPrintedTable)
    {
        sstreamEntry << std::endl << "[" << rssParent << "]" << std::endl;
        rssLastPrintedTable = rssParent;
    }

    // Stream the array
    if (bEmbedded && !bFirst)     // 2nd or higher array entry
        sstreamEntry << ", ";
    if (!bEmbedded || bAssignment)     // Not an array entry
        sstreamEntry << GetName() << " = ";
    sstreamEntry << "[";
    for (uint32_t ui = 0; ui < GetCount(); ui++)
        sstreamEntry << Get(ui)->CreateTOMLText(rssParent, rssLastPrintedTable, ui == 0, true, false);
    sstreamEntry << "]";
     if (!bEmbedded)     // Not an array entry
        sstreamEntry << std::endl;

    return sstreamEntry.str();
}

void CArray::Add(const std::string& rssPath, const std::shared_ptr<CNode>& rptrNode, bool bDefinedExplicitly)
{
    size_t		nDotPos = FindFirst(rssPath);
    std::string ssFirst  = rssPath.substr(0, nDotPos);
    std::string ssSecond = nDotPos == std::string::npos ? "" : rssPath.substr(nDotPos + 1);

    // Add new element to array
    if (nDotPos == std::string::npos)
    {
        GetArray()->AddElement(rptrNode);
        return;
    }
    // Add new element to subelement of array
    if (std::any_of(ssFirst.begin(), ssFirst.end(), [](char digit) { return (digit < '0') || (digit > '9'); }))
    {
        throw XTOMLParseException(("Invalid array access subscript '" + ssFirst + "'").c_str());
    }
    uint32_t uiIndex = std::stoi(ssFirst);
    if (uiIndex >= GetArray()->GetCount())
    {
        // This indicates an array within an arrays. Add the intermediate array
        auto ptrIntermediateArray = std::make_shared<CNormalArray>(ssFirst);
        ptrIntermediateArray->SetParent(shared_from_this());
        ptrIntermediateArray->m_bDefinedExplicitly = bDefinedExplicitly;
        GetArray()->AddElement(ptrIntermediateArray);
        static_cast<CNode*>(ptrIntermediateArray.get())->Add(ssSecond, rptrNode, bDefinedExplicitly);
        //throw XTOMLParseException(("Invalid array access index '" + ssFirst + "'; out of bounds").c_str());
    }
    GetArray()->Get(uiIndex)->Add(ssSecond, rptrNode, bDefinedExplicitly);
}

std::shared_ptr<CNode> CArray::Find(const std::string& rssPath) const
{
    size_t nDotPos = FindFirst(rssPath);
    std::string ssFirst  = rssPath.substr(0, nDotPos);
    std::string ssSecond = nDotPos == std::string::npos ? "" : rssPath.substr(nDotPos + 1);

    if (ssFirst.empty())
        throw XTOMLParseException("Missing array subscript");

    if (std::any_of(ssFirst.begin(), ssFirst.end(), [](char digit) { return (digit < '0') || (digit > '9'); }))
        throw XTOMLParseException(("Invalid array access subscript '" + ssFirst + "'").c_str());
    uint32_t uiIndex = std::stoi(ssFirst);
    if (GetArray()->GetCount() <= uiIndex)
        throw XTOMLParseException(
            ("Invalid array access index '" + ssFirst + "'; out of bounds").c_str());
    if (nDotPos == std::string::npos)
        return GetArray()->Get(uiIndex);
    return GetArray()->Get(uiIndex)->Find(ssSecond);
}

std::string CTableArray::CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool /*bFirst*/, bool /*bEmbedded*/,
    bool /*bAssignment*/, bool bRoot) const
{
    // Create the full name
    std::string ssFullName = rssParent;
    if (!ssFullName.empty() && !bRoot) ssFullName += ".";
    ssFullName += GetName();

    // Stream the array
    std::stringstream sstreamEntry;
    for (uint32_t ui = 0; ui < GetCount(); ui++)
    {
        sstreamEntry << std::endl << "[[" << ssFullName << "]]" << std::endl;
        rssLastPrintedTable = ssFullName;
        sstreamEntry << Get(ui)->CreateTOMLText(ssFullName, rssLastPrintedTable, ui == 0, false, false);
    }
    //sstreamEntry << std::endl;

    return sstreamEntry.str();
}

void CTableArray::Add(const std::string& rssPath, const std::shared_ptr<CNode>& rptrNode, bool bDefinedExplicitly)
{
    //size_t nDotPos = FindFirst(rssPath);
    //std::string ssFirst  = rssPath.substr(0, nDotPos);
    //std::string ssSecond = nDotPos == std::string::npos ? "" : rssPath.substr(nDotPos + 1);

    uint32_t uiSize = GetArray()->GetCount();
    if (uiSize == 0)
    {
        throw XTOMLParseException("Trying to access table in an empty array of tables");
    }
    GetArray()->Get(uiSize - 1)->Add(rssPath, rptrNode, bDefinedExplicitly);
}

std::shared_ptr<CNode> CTableArray::Find(const std::string& rssPath) const
{
    //size_t nDotPos = FindFirst(rssPath);
    //std::string ssFirst  = rssPath.substr(0, nDotPos);
    //std::string ssSecond = nDotPos == std::string::npos ? "" : rssPath.substr(nDotPos + 1);

    if (!GetArray()->GetCount())
        throw XTOMLParseException(("Trying to access empty table array; " + rssPath).c_str());
    return GetArray()->Get(GetArray()->GetCount() - 1)->Find(rssPath);
}

std::string CRootTable::CreateTOMLText(const std::string& rssParent, std::string& rssLastPrintedTable, bool /*bFirst*/,
    bool /*bEmbedded*/, bool /*bAssignment*/, bool /*bRoot*/) const
{
    // Create the full name
    std::string ssFullName = rssParent;

    // Stream the table
    std::stringstream sstreamEntry;
    for (uint32_t uiIndex = 0; uiIndex < GetCount(); uiIndex++)
    {
        std::shared_ptr<CNode> ptrNode = Get(uiIndex);
        if (!ptrNode) continue;
        sstreamEntry << ptrNode->CreateTOMLText(ssFullName, rssLastPrintedTable);
    }

    sstreamEntry << std::endl;

    // Skip whitespace at the beginning
    std::string ssRet = sstreamEntry.str();
    size_t nStart = ssRet.find_first_not_of(" \t\f\r\n\v");
    if (nStart == std::string::npos) return std::string();
    return ssRet.substr(nStart);
}
