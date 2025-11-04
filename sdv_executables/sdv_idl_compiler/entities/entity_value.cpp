#include "entity_value.h"
#include "../exception.h"
#include "variable_entity.h"
#include "definition_entity.h"
#include "../constvariant.inl"
#include "../support.h"
#include "../parser.h"

CEntityValueNode::CEntityValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) :
    m_ptrEntity(ptrEntity), m_ptrParent(ptrParent)
{
    if (!ptrEntity) throw CCompileException("Internal error: expected a valid entity.");

    CLog log;
    if (!ptrParent)
        log << "The value node of '" << ptrEntity->GetName() << "' has no parent node..." << std::endl;
    else
        log << "The value node of '" << ptrEntity->GetName() << "' has a parent node..." << std::endl;
}

CEntityValueNode::CEntityValueNode(const CEntityValueNode& rValueNode, CEntityPtr ptrEntity, const CValueNodePtr ptrParent) :
    m_ptrEntity(ptrEntity), m_ptrParent(ptrParent), m_vecChildren(rValueNode.m_vecChildren)
{
    if (!ptrEntity) throw CCompileException("Internal error: expected a valid entity.");

    CLog log;
    if (!ptrParent)
        log << "The value node of '" << ptrEntity->GetName() << "' has no parent node..." << std::endl;
    else
        log << "The value node of '" << ptrEntity->GetName() << "' has a parent node..." << std::endl;
}

void CEntityValueNode::ProcessChildNodes(const CTokenList& rlstExpression)
{
    CLog log;
    log << "Processing " << m_vecChildren.size() << " child nodes..." << std::endl;
    bool bInitial = true;
    for (CValueNodePtr& rptrValue : m_vecChildren)
    {
        // Check for a comma separator.
        if (!bInitial)
        {
            if (rlstExpression.Current() != ",")
                throw CCompileException(rlstExpression.LastValid(), "Missing comma ',' for value separation.");
            ++rlstExpression;
        }
        bInitial = false;

        log << "Processing child node..." << std::endl;
        rptrValue->ProcessValueAssignment(rlstExpression);
    }
}

void CEntityValueNode::AddChild(CValueNodePtr ptrChild)
{
    CLog log;
    log << "Adding child node..." << std::endl;
    if (!ptrChild)
        throw CCompileException("Internal error: cannot add an empty child node.");
    m_vecChildren.push_back(ptrChild);
}

std::string CEntityValueNode::GetDeclTypeStr(bool bResolveTypedef) const
{
    return GetDeclEntity()->GetDeclTypeStr(bResolveTypedef);
}

bool CEntityValueNode::IsConst() const
{
    // Default implementation
    // - In case the entity is a const entity, return 'true'.
    // - Otherwise check whether the parent value is available and ask the parent if the declaration entity is a const entity.
    // - If no parent is available, this entity must be a declaration entity and is therefore not a const entity.
    if (m_ptrEntity->Get<CVariableEntity>() && m_ptrEntity->Get<CVariableEntity>()->IsReadOnly()) return true;
    if (m_ptrParent) return m_ptrParent->IsConst();
    return false;
}

bool CEntityValueNode::IsDeclaration() const
{
    // Default implementation
    return m_ptrEntity->Get<CVariableEntity>();
}

CSimpleTypeValueNode::CSimpleTypeValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) :
    CEntityValueNode(ptrEntity, ptrParent)
{}

CSimpleTypeValueNode::CSimpleTypeValueNode(const CSimpleTypeValueNode& rValueNode, CEntityPtr ptrEntity,
    const CValueNodePtr ptrParent) :
    CEntityValueNode(rValueNode, ptrEntity, ptrParent), m_varValue(rValueNode.m_varValue),
    m_lstExpression(rValueNode.m_lstExpression), m_eValueDef(rValueNode.m_eValueDef)
{}

CValueNodePtr CSimpleTypeValueNode::CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const
{
    return std::make_shared<std::decay_t<std::remove_pointer_t<decltype(this)>>>(*this, ptrEntity, ptrParent);
}

void CSimpleTypeValueNode::ProcessValueAssignment(const CTokenList& rlstExpression)
{
    CLog log("Processing declaration value assignment...");

    if (!m_ptrEntity) throw CCompileException("Internal error: no entity assigned to the value node.");

    const CDeclarationEntity* pSystemTypeEntity = m_ptrEntity->Get<CDeclarationEntity>();
    if (!pSystemTypeEntity)
        throw CCompileException("Internal error: type mismatch between value node and entity type.");

    // Process the assignment expression and convert the result in the target type.
    std::pair<CConstVariant, bool> prValue{0, false};
    switch (pSystemTypeEntity->GetBaseType())
    {
    case  sdv::idl::EDeclType::decltype_short:
        log << "Processing system type value node of type 'int16/short'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<int16_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<int64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_long:
        log << "Processing system type value node of type 'int32/long'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<int32_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<int64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_long_long:
        log << "Processing system type value node of type 'int64/long long'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<int64_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<int64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_unsigned_short:
        log << "Processing system type value node of type 'uint16/unsigned short'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<uint16_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<uint64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_unsigned_long:
        log << "Processing system type value node of type 'uint32/unsigned long'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<uint32_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<uint64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_unsigned_long_long:
        log << "Processing system type value node of type 'uint64/unsigned long long'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<uint64_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<uint64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_char:
        log << "Processing system type value node of type 'int8/UTF-8 or ASCII char'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<int8_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<int64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_char16:
        log << "Processing system type value node of type 'UTF-16 char'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<uint16_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<uint64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_char32:
        log << "Processing system type value node of type 'UTF-32 char'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<uint32_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<uint64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_wchar:
        log << "Processing system type value node of type 'wchar'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        if (sizeof(wchar_t) == 2)
            prValue.first.Convert<uint16_t>();
        else
            prValue.first.Convert<uint32_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<uint64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_float:
        log << "Processing system type value node of type 'float'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<float>();
        log << "Simple type value node has calculated value " << prValue.first.Get<long double>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_double:
        log << "Processing system type value node of type 'double'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<double>();
        log << "Simple type value node has calculated value " << prValue.first.Get<long double>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_long_double:
        log << "Processing system type value node of type 'long double'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<long double>();
        log << "Simple type value node has calculated value " << prValue.first.Get<long double>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_fixed:
        log << "Processing system type value node of type 'fixed'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<fixed>();
        log << "Simple type value node has calculated value " << prValue.first.Get<long double>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_boolean:
        log << "Processing system type value node of type 'boolean'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<bool>();
        log << "Simple type value node has calculated value " << (prValue.first.Get<bool>() ? "'true'" : "'false'") << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_native:
        log << "Processing system type value node of type 'native'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<size_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<size_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_octet:
        log << "Processing system type value node of type 'uint8/octet'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessNumericExpression(rlstExpression);
        prValue.first.Convert<uint8_t>();
        log << "Simple type value node has calculated value " << prValue.first.Get<uint64_t>() << "..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_string:
        log << "Processing system type value node of type 'ASCII string'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessStringExpression(rlstExpression);
        prValue.first.Convert<std::string>();
        log << "Simple type value node has constructed string \"" << prValue.first.Get<std::string>() << "\"..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_u8string:
        log << "Processing system type value node of type 'UTF-8 string'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessStringExpression(rlstExpression);
        prValue.first.Convert<std::string>();
        log << "Simple type value node has constructed string \"" << prValue.first.Get<std::string>() << "\"..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_u16string:
        log << "Processing system type value node of type 'UTF-16 string'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessStringExpression(rlstExpression);
        prValue.first.Convert<std::u16string>();
        log << "Simple type value node has constructed string..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_u32string:
        log << "Processing system type value node of type 'UTF-32 string'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessStringExpression(rlstExpression);
        prValue.first.Convert<std::u32string>();
        log << "Simple type value node has constructed string..." << std::endl;
        break;
    case  sdv::idl::EDeclType::decltype_wstring:
        log << "Processing system type value node of type 'wstring'..." << std::endl;
        prValue = pSystemTypeEntity->ProcessStringExpression(rlstExpression);
        prValue.first.Convert<std::wstring>();
        log << "Simple type value node has constructed string..." << std::endl;
        break;
    default:
        log << "Processing system type value node of unknown type..." << std::endl;
        throw CCompileException("Internal error: expression build error.");
    }

    // Check for a dynamic result
    if (prValue.second)
        SetDynamicValue(rlstExpression);
    else
        SetFixedValue(prValue.first, rlstExpression);
}

void CSimpleTypeValueNode::SetFixedValue(const CConstVariant& rvarValue, const CTokenList& rlstExpression)
{
    CLog log;
    switch (m_eValueDef)
    {
    case EValueDef::dynamic:
        log << "The value was re-assigned; the previous value was a dynamic value..." << std::endl;
        break;
    case EValueDef::fixed:
        log << "The value was re-assigned; the previous value was a fixed value of " <<
            m_varValue.GetAsString() << "..." << std::endl;
        break;
    default:
        break;
    }
    m_eValueDef = EValueDef::fixed;

    log << "The system type value node was set to a fixed value of " << rvarValue.GetAsString() << "..." << std::endl;

    m_varValue = rvarValue;
    m_lstExpression = rlstExpression;
}

void CSimpleTypeValueNode::SetDynamicValue(const CTokenList& rlstExpression)
{
    CLog log;
    switch (m_eValueDef)
    {
    case EValueDef::dynamic:
        log << "The value was re-assigned; the previous value was a dynamic value..." << std::endl;
        break;
    case EValueDef::fixed:
        log << "The value was re-assigned; the previous value was a fixed value of " <<
            m_varValue.GetAsString() << "..." << std::endl;
        break;
    default:
        break;
    }
    m_eValueDef = EValueDef::dynamic;

    log << "The system type value node was set to a dynamic value..." << std::endl;

    m_lstExpression = rlstExpression;
}

CArrayValueNode::CArrayValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) :
    CEntityValueNode(ptrEntity, ptrParent)
{}

CArrayValueNode::CArrayValueNode(const CArrayValueNode& rValueNode, CEntityPtr ptrEntity, const CValueNodePtr ptrParent) :
    CEntityValueNode(rValueNode, ptrEntity, ptrParent), m_lstArraySizeExpression(rValueNode.m_lstArraySizeExpression),
    m_eSizeDef(rValueNode.m_eSizeDef)
{
    m_vecChildren = rValueNode.m_vecChildren;
}

CValueNodePtr CArrayValueNode::CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const
{
    return std::make_shared<std::decay_t<std::remove_pointer_t<decltype(this)>>>(*this, ptrEntity, ptrParent);
}

void CArrayValueNode::ProcessValueAssignment(const CTokenList& rlstExpression)
{
    CLog log("Process array value node...");

    if (rlstExpression.End())
        throw CCompileException(rlstExpression.LastValid(), "Expecting left curly bracket '{'");

    // The array assignment is placed between brackets.
    // In case the type is a character, the array might also be defined as a string literal
    // Or identified as a value sequence.

    if (rlstExpression.Current() == "{")
    {
        // Skip the bracket
        ++rlstExpression;

        // Process the child nodes
        ProcessChildNodes(rlstExpression);

        // Expecting '}'
        if (rlstExpression.End() || rlstExpression.Current() != "}")
            throw CCompileException(rlstExpression.LastValid(), "Expecting right curly bracket '}'");
        ++rlstExpression;
    }
    else if (rlstExpression.Current().GetType() == ETokenType::token_literal && rlstExpression.Current().IsString())
    {
        // Process the string node
        ProcessStringNode(rlstExpression.Current());
        ++rlstExpression;
    }
    else
        throw CCompileException(rlstExpression.LastValid(), "Expecting left curly bracket '{'");
}

void CArrayValueNode::ProcessChildNodes(const CTokenList& rlstExpression)
{
    CLog log;
    CValueNodePtr ptrValueTemplateUnbound;
    if (IsUnbound())
    {
        // The array should have one node. This is a template for all the nodes.
        if (m_vecChildren.size() != 1)
            throw CCompileException(rlstExpression.LastValid(), "Internal error: the size of the array value nodes is not as expected for the"
                " unbound array.");
        ptrValueTemplateUnbound = std::move(m_vecChildren[0]);

        log << "Processing ubound element nodes..." << std::endl;
    }
    else
        log << "Processing " << m_vecChildren.size() << " element nodes..." << std::endl;

    size_t nIndex = 0;
    while (!rlstExpression.End() && rlstExpression.Current() != "}")
    {
        // Check for a comma separator.
        if (nIndex)
        {
            if (rlstExpression.Current() != ",")
                throw CCompileException(rlstExpression.LastValid(), "Missing comma ',' for value separation.");
            ++rlstExpression;
        }

        // Extend the array for unbound
        if (IsUnbound())
        {
            m_vecChildren.resize(nIndex + 1);
            m_vecChildren[nIndex] = ptrValueTemplateUnbound->CreateCopy(m_ptrEntity, shared_from_this());
        }

        // Does the size fit?
        if (nIndex >= m_vecChildren.size())
            throw CCompileException(rlstExpression.LastValid(), "The assignment doesn't fit the amount of elements.");

        log << "Processing element node #" << nIndex << std::endl;
        m_vecChildren[nIndex]->ProcessValueAssignment(rlstExpression);

        // Next node
        nIndex++;
    }

    // Check whether the index corresponds to the size of the aray.
    if (nIndex != m_vecChildren.size())
        throw CCompileException(rlstExpression.LastValid(), "Missing elements for array assignment.");
}

void CArrayValueNode::ProcessStringNode(const CToken& rToken)
{
    CLog log;

    if (!rToken.IsString())
        throw CCompileException(rToken, "Internal error: token is expected to be a string literal.");


    // Check for unbound arrays.
    CValueNodePtr ptrValueTemplateUnbound;
    if (IsUnbound())
    {
        // The array should have one node. This is a template for all the nodes.
        if (m_vecChildren.size() != 1)
            throw CCompileException(rToken, "Internal error: the size of the array value nodes is not as expected for the"
                " unbound array.");
        ptrValueTemplateUnbound = std::move(m_vecChildren[0]);

        log << "Processing ubound element nodes..." << std::endl;
    }
    else
        log << "Processing " << m_vecChildren.size() << " element nodes..." << std::endl;

    // Determine the start position of the string
    const std::string ssValue = static_cast<std::string>(rToken);
    size_t nStart = ssValue.find('"');
    if (nStart == std::string::npos)
        throw CCompileException(rToken, "Internal error: invalid string token.");
    nStart++;

    // For raw string, determine the delimiter
    std::string ssDelimiter = "\"";
    bool bRaw = false;
    if (rToken.IsRawString())
    {
        bRaw = true;
        size_t nStart2 = ssValue.find('(', nStart);
        if (nStart2 == std::string::npos)
            throw CCompileException(rToken, "Internal error: invalid raw string token.");
        ssDelimiter = ")" + ssValue.substr(nStart, nStart2 - nStart) + "\"";
        nStart = nStart2;
    }

    // Check whether the base type corresponds to the string and count the characters.
    bool bError = true;
    uint32_t uiByteCnt = 0;
    std::vector<CConstVariant> lstValues;
    switch (GetDeclEntity()->GetBaseType())
    {
    case sdv::idl::EDeclType::decltype_char:
    {
        if (!rToken.IsAscii() && !rToken.IsUtf8()) break;
        bError = false;
        std::string ssText;
        InterpretCText(ssValue.c_str() + nStart, ssDelimiter.c_str(), ssText, uiByteCnt, bRaw, rToken.IsAscii());
        for (char c : ssText)
            lstValues.push_back(c);
        break;
    }
    case sdv::idl::EDeclType::decltype_wchar:
    {
        if (!rToken.IsWide()) break;
        bError = false;
        std::wstring ssText;
        InterpretCText(ssValue.c_str() + nStart, ssDelimiter.c_str(), ssText, uiByteCnt, bRaw);
        for (wchar_t c : ssText)
            lstValues.push_back(c);
        break;
    }
    case sdv::idl::EDeclType::decltype_char16:
    {
        if (!rToken.IsUtf16()) break;
        bError = false;
        std::u16string ssText;
        InterpretCText(ssValue.c_str() + nStart, ssDelimiter.c_str(), ssText, uiByteCnt, bRaw);
        for (char16_t c : ssText)
            lstValues.push_back(c);
        break;
    }
    case sdv::idl::EDeclType::decltype_char32:
    {
        if (!rToken.IsUtf32()) break;
        bError = false;
        std::u32string ssText;
        InterpretCText(ssValue.c_str() + nStart, ssDelimiter.c_str(), ssText, uiByteCnt, bRaw);
        for (char32_t c : ssText)
            lstValues.push_back(c);
        break;
    }
    default:
        throw CCompileException(rToken, "Expecting left curly bracket '{'");
        break;
    }
    if (bError)
        throw CCompileException(rToken, "Invalid string type for the assignment");

    // Add the terminateing zero
    lstValues.push_back(0);

    // Extend the array for unbound
    if (IsUnbound())
    {
        m_vecChildren.resize(lstValues.size());
        for (size_t nIndex = 0; nIndex < lstValues.size(); nIndex++)
            m_vecChildren[nIndex] = ptrValueTemplateUnbound->CreateCopy(m_ptrEntity, shared_from_this());
    }

    // Does the size fit?
    if (lstValues.size() != m_vecChildren.size())
        throw CCompileException(rToken, "The assignment doesn't fit the amount of elements.");

    // Assign the values
    for (size_t nIndex = 0; nIndex < m_vecChildren.size(); nIndex++)
    {
        log << "Processing element node #" << nIndex << std::endl;
        CSimpleTypeValueNode* pValueNode = m_vecChildren[nIndex]->Get<CSimpleTypeValueNode>();
        if (!pValueNode)
            throw CCompileException(rToken, "Internal error: the array element is not of type simple value node.");

        CTokenList lstExpression;
        lstExpression.push_back(rToken);
        pValueNode->SetFixedValue(lstValues[0], lstExpression);
    }
}

void CArrayValueNode::SetFixedSize(size_t nSize, const CTokenList& rlstExpression)
{
    CLog log;
    if constexpr (std::is_signed_v<size_t>)
    {
        if (static_cast<int64_t>(nSize) < 0)
            throw CCompileException("Internal error: array index is negative.");
    }

    if (m_eSizeDef != ESizeDef::not_defined)
        throw CCompileException("Internal error: size of the value array was defined more than once.");
    m_eSizeDef = ESizeDef::fixed;

    // Allocate the array with empty value pointers.
    m_vecChildren.resize(nSize);
    log << "Array set to a fixed size of " << nSize << "..." << std::endl;

    // Copy the token list resulting in the size.
    m_lstArraySizeExpression = rlstExpression;
}

void CArrayValueNode::SetDynamicSize(size_t nSize, const CTokenList& rlstExpression)
{
    if (IsConst()) throw CCompileException("Cannot use a value of a variable declaration as size within a const declaration.");

    CLog log;
    if (m_eSizeDef != ESizeDef::not_defined)
        throw CCompileException("Internal error: size of the value array was defined more than once.");
    m_eSizeDef = ESizeDef::dynamic;

    // Allocate the array with one element.
    m_vecChildren.resize(nSize);
    log << "Array set to a dynamic size based on a variable of " << nSize << "..." << std::endl;

    // Copy the token list resulting in the size.
    m_lstArraySizeExpression = rlstExpression;
}

void CArrayValueNode::SetFixedSizeUnbound()
{
    CLog log;
    if (m_eSizeDef != ESizeDef::not_defined)
        throw CCompileException("Internal error: size of the value array was defined more than once.");

    m_eSizeDef = ESizeDef::fixed_unbound;

    // Allocate the array with one element.
    m_vecChildren.resize(1);
    log << "Array set to be unbound..." << std::endl;
}

const CValueNodePtr& CArrayValueNode::operator[](size_t nIndex) const
{
    if (m_eSizeDef == ESizeDef::not_defined)
        throw CCompileException("Internal error: accessing the array can only take place after the size has been defined.");

    if constexpr (std::is_signed_v<size_t>)
    {
        if (static_cast<int64_t>(nIndex) < 0)
            throw CCompileException("Internal error: array index is negative.");
    }

    return m_vecChildren[nIndex];
}

CValueNodePtr& CArrayValueNode::operator[](size_t nIndex)
{
    if (m_eSizeDef == ESizeDef::not_defined)
        throw CCompileException("Internal error: accessing the array can only take place after the size has been defined.");

    if constexpr (std::is_signed_v<size_t>)
    {
        if (static_cast<int64_t>(nIndex) < 0)
            throw CCompileException("Internal error: array index is negative.");
    }

    return m_vecChildren[nIndex];
}

size_t CArrayValueNode::GetSize() const
{
    return m_vecChildren.size();
}

bool CArrayValueNode::IsArray() const
{
    return true;
}

bool CArrayValueNode::IsUnbound() const
{
    return m_eSizeDef == ESizeDef::fixed_unbound;
}

std::string CArrayValueNode::GetSizeExpression() const
{
    if (IsUnbound()) return std::string();
    std::string ss;
    for (const CToken& rtoken : m_lstArraySizeExpression)
        ss += static_cast<std::string>(rtoken);
    return ss;
}

std::string CArrayValueNode::GetDeclTypeStr(bool bResolveTypedef) const
{
    if (!GetSize()) return "[]";    // Not resolved yet...

    CValueNodePtr ptrValue = (*this)[0];
    if (!ptrValue) return std::string("[" + std::to_string(GetSize()) + "]"); // Not resolved yet...
    return ptrValue->GetDeclTypeStr(bResolveTypedef) + "[" + std::to_string(GetSize()) + "]";
}

CCompoundTypeValueNode::CCompoundTypeValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) :
    CEntityValueNode(ptrEntity, ptrParent)
{}

CCompoundTypeValueNode::CCompoundTypeValueNode(const CCompoundTypeValueNode& rValueNode, CEntityPtr ptrEntity,
    const CValueNodePtr ptrParent) : CEntityValueNode(rValueNode, ptrEntity, ptrParent)
{}

CValueNodePtr CCompoundTypeValueNode::CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const
{
    return std::make_shared<std::decay_t<std::remove_pointer_t<decltype(this)>>>(*this, ptrEntity, ptrParent);
}

void CCompoundTypeValueNode::ProcessValueAssignment(const CTokenList& rlstExpression)
{
    CLog log("Processing struct value node...");

    if (rlstExpression.End())
        throw CCompileException(rlstExpression.LastValid(), "Missing assignment");
    if (!m_ptrEntity)
        throw CCompileException(rlstExpression.LastValid(), "Internal error: a value node should be assigned to an entity.");

    // There are two options: either a scoped name representing a type (or typedef) being of the same type (identical name), or a
    // compound value (between curly brackets '{...}').
    if (rlstExpression.Current() != "{")    // Type value assignment
    {
        CValueNodePtr ptrValue = m_ptrEntity->FindValue(rlstExpression);

        // Check if the original type of the assginee and the assigned are identical. For this both entities must be declarations.
        if (!GetDeclEntity()->Get<CDeclarationEntity>())
            throw CCompileException(rlstExpression.LastValid(), "Internal error: the assignee entity must be a declaration entity.");

        // Assignment can only work if the types are identical. This is regardless of the typedefs that have been made from the
        // type.
        if (GetDeclTypeStr(true) != ptrValue->GetDeclTypeStr(true))
                throw CCompileException(rlstExpression.LastValid(), "For '", GetDeclEntity()->GetName(), "', cannot assign '",
                ptrValue->GetDeclTypeStr(true), "' to '", ptrValue->GetDeclTypeStr(true), "'; base types are different.");

        // Replace the value by the value chain of the assigned to the assignee.
        m_ptrEntity->ValueRef() = ptrValue->CreateCopy(m_ptrEntity, m_ptrParent);
    } else  // Compound value assignment
    {
        ++rlstExpression;   // Skip left bracket

        // Process the child nodes
        ProcessChildNodes(rlstExpression);

        // Expecting '}'
        if (rlstExpression.End() || rlstExpression.Current() != "}")
            throw CCompileException(rlstExpression.LastValid(), "Expecting right curly bracket '}'");
        ++rlstExpression;   // Skip right bracket
    }
}

CInterfaceValueNode::CInterfaceValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) :
    CEntityValueNode(ptrEntity, ptrParent)
{}

CInterfaceValueNode::CInterfaceValueNode(const CInterfaceValueNode& rValueNode, CEntityPtr ptrEntity,
    const CValueNodePtr ptrParent) :
    CEntityValueNode(rValueNode, ptrEntity, ptrParent)
{}

CValueNodePtr CInterfaceValueNode::CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const
{
    return std::make_shared<std::decay_t<std::remove_pointer_t<decltype(this)>>>(*this, ptrEntity, ptrParent);
}

void CInterfaceValueNode::ProcessValueAssignment(const CTokenList& rlstExpression)
{
    CLog log("Processing declaration value assignment...");

    if (!m_ptrEntity) throw CCompileException("Internal error: no entity assigned to the value node.");

    const CDeclarationEntity* pSystemTypeEntity = m_ptrEntity->Get<CDeclarationEntity>();
    if (!pSystemTypeEntity)
        throw CCompileException("Internal error: type mismatch between value node and entity type.");

    // Only "null" and "0" are allowed.
    if ((!m_ptrEntity->GetParserRef().GetEnvironment().InterfaceTypeExtension() || rlstExpression.Current() != "null") &&
        rlstExpression.Current() != "0")
        throw CCompileException(rlstExpression.Current(),
            "Error: assignment of interface values is only possible with the value \"null\".");

    // Skip the value.
    ++rlstExpression;
}


CValueNodePtr CCompoundTypeValueNode::Member(const std::string& rssName) const
{
    // Check the entity names of the child value nodes.
    // ATTENTION: The child value nodes are copies of the original definition. They still contain the link to the declarations
    // that were part of the definition. The entity belonging to a declaration of a compound type does not have any children (the
    // entity itself is just the declaration and not the definition):
    for (const CValueNodePtr& rptrValue : m_vecChildren)
        if (rptrValue->GetDeclEntity()->GetName() == rssName) return rptrValue;
    return nullptr; // Value not found.
}

CEnumValueNode::CEnumValueNode(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) :
    CEntityValueNode(ptrEntity, ptrParent)
{}

CEnumValueNode::CEnumValueNode(const CEnumValueNode& rValueNode, CEntityPtr ptrEntity,
    const CValueNodePtr ptrParent) :
    CEntityValueNode(rValueNode, ptrEntity, ptrParent), m_ptrEntryVal(rValueNode.m_ptrEntryVal),
    m_lstExpression(rValueNode.m_lstExpression)
{}

CValueNodePtr CEnumValueNode::CreateCopy(CEntityPtr ptrEntity, const CValueNodePtr ptrParent) const
{
    return std::make_shared<std::decay_t<std::remove_pointer_t<decltype(this)>>>(*this, ptrEntity, ptrParent);
}

void CEnumValueNode::ProcessValueAssignment(const CTokenList& rlstExpression)
{
    CLog log("Processing enum value assignment...");
    // The assignment is done using the enum entry type. Find the scoped name. This could be the enum value. Check the type first.
    CTokenList rlstExpressionTemp = rlstExpression;
    std::pair<std::string, CEntityPtr> prEntity = GetDeclEntity()->GetTypeEntity()->ProcessScopedName(rlstExpression, true);
    // If not an enum value from the type, use as scope the parent of the current enum to search for other declarations.
    if (!prEntity.second) prEntity = GetDeclEntity()->GetParentEntity()->ProcessScopedName(rlstExpressionTemp, true);
    if (!prEntity.second) throw CCompileException(rlstExpression.LastValid(), "Expecting an enum value.");

    // This could either be an assignment of an enum to another enum or an enum entry to an enum.
    log << "The to be assigned type is '" << prEntity.second->GetDeclTypeStr(true) << "'..." << std::endl;
    log << "The assignee type is '" << GetDeclEntity()->GetDeclTypeStr(true) << "'..." << std::endl;
    if (prEntity.second->GetDeclTypeStr(true) == GetDeclEntity()->GetDeclTypeStr(true))
    {
        if (!prEntity.second->ValueRef())
            throw CCompileException(rlstExpression.LastValid(), "Internal error: the enum doesn't have an assigned value.");
        if (!prEntity.second->ValueRef()->Get<CEnumValueNode>())
            throw CCompileException(rlstExpression.LastValid(), "Internal error: the enum doesn't have an assigned enum value.");
        m_ptrEntryVal = prEntity.second->ValueRef()->Get<CEnumValueNode>()->m_ptrEntryVal;
    } else if (prEntity.second->GetParentEntity()->GetDeclTypeStr(true) == GetDeclEntity()->GetDeclTypeStr(true))
    {
        log << "Assigning enum entry..." << std::endl;
        m_ptrEntryVal = prEntity.second;
    } else
        throw CCompileException(rlstExpression.LastValid(), "Invalid enum value; enum types are not identical.");
    log << "Enum entity was assigned value '" << m_ptrEntryVal->GetParentEntity()->GetName() << "::" <<
        m_ptrEntryVal->GetName() << "'..." << std::endl;
}

const CConstVariant& CEnumValueNode::Variant() const
{
    static CConstVariant temp;
    if (!m_ptrEntryVal) return temp;
    if (!m_ptrEntryVal->ValueRef()) return temp;
    const CSimpleTypeValueNode* pValueNode = m_ptrEntryVal->ValueRef()->Get<CSimpleTypeValueNode>();
    if (!pValueNode) return temp;
    return pValueNode->Variant();
}

std::string CEnumValueNode::String() const
{
    if (!m_ptrEntryVal) return std::string();
    return m_ptrEntryVal->GetParentEntity()->GetScopedName() + "::" + m_ptrEntryVal->GetName();
}
