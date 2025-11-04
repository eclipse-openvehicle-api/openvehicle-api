#include "union_entity.h"
#include "../exception.h"
#include "../logger.h"
#include "../parser.h"
#include "typedef_entity.h"
#include "variable_entity.h"
#include <iostream>
#include <set>

CCaseEntry::CCaseEntry(const CContextPtr& rptrContext, CEntityPtr ptrParent, bool bDefault) :
    CVariableEntity(rptrContext, ptrParent, false, false), m_bDefault(bDefault)
{}

std::string CCaseEntry::GetDeclTypeStr(bool /*bResolveTypedef*/) const
{
    return GetScopedName();
}

sdv::interface_t CCaseEntry::GetInterface(sdv::interface_id idInterface)
{
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::ICaseEntity>())
        return static_cast<sdv::idl::ICaseEntity*>(this);
    return CVariableEntity::GetInterface(idInterface);
}

sdv::u8string CCaseEntry::GetLabel() const
{
    // The default case entry doesn#t have a label.
    if (m_bDefault) return sdv::u8string();

    // Get the label string from the value of the case label entity.
    CVariableEntity* pLabelVariableEntity = m_ptrLabel->Get<CVariableEntity>();
    if (!pLabelVariableEntity)
        throw CCompileException("Internal error: expected a variable entity as case label.");
    if (!pLabelVariableEntity->ValueRef())
        throw CCompileException("Internal error: expected a value for the variable entity case label.");

    // Enum value node?
    const CEnumValueNode* pEnumValueNode = pLabelVariableEntity->ValueRef()->Get<CEnumValueNode>();
    if (pEnumValueNode) return pEnumValueNode->String();

    // Simple value node?
    const CSimpleTypeValueNode* pSimpleTypeValueNode = pLabelVariableEntity->ValueRef()->Get<CSimpleTypeValueNode>();
    if (pSimpleTypeValueNode) return pSimpleTypeValueNode->Variant().GetAsString();

    // Otherwise error...
    throw CCompileException("Internal error: expected a value for the variable entity case label.");
}

const CToken& CCaseEntry::GetLabelToken() const
{
    return m_tokenLabel;
}

void CCaseEntry::Process()
{
    CLog log("Processing case label...");

    // Peek for the label token (used for reporting parsing errors).
    m_tokenLabel = PeekToken();

    // Processing of the case label value is done in post processing since the switch type might not be known yet.
    CToken token;
    std::string ssCaseLabel;
    while ((token = GetToken()) != ":")
    {
        if (!ssCaseLabel.empty())
            ssCaseLabel += ' ';
        ssCaseLabel += static_cast<std::string>(token);
        m_lstCaseValue.push_back(token);
    }
    log << "Case label name '" << ssCaseLabel << "'" << std::endl;

    // Only unnamed nested struct and union definitions are not allowed. Furthermore, anonymous structs and unions (without
    // explicit declaration cannot be supported (also not part of the C++ standard). Varable based switch type unions are
    // also not supported since the variable for the switch needs to be part of the parent switch case and thus needs a struct to
    // hold it.
    token = PeekToken();
    if ((token == "struct" && (PeekToken(1) == "{" || PeekToken(2) == "{")) ||
        (token == "union" && (PeekToken(1) == "switch" || PeekToken(2) == "switch")))
    {
        // Get the struct/union from the code
        token = GetToken();

        throw CCompileException(token, "Cannot make a definition inside an union.");
    }

    // Stop processing if the case doesn't have any declaration (is followed by another case or a closing curled bracket).
    if (token == "case" || token == "}")
    {
        // Determine whether the comments are preceding the token (either on the same line or the line before).
        CTokenList lstPreComments = GetPreCommentTokenList();
        if (!lstPreComments.empty()) SetCommentTokens(lstPreComments);

        // Assign any succeeding comments
        ProcessPostCommentTokenList(token.GetLine());

        // Insert a semi-colon to identify that the statement is finished.
        CToken tokenSep(";", ETokenType::token_separator);
        PrependToken(tokenSep);
        SetName(GetParserRef().GenerateAnonymousEntityName("case"));
        return;
    }

    // Now let the variable process the declaration
    CVariableEntity::Process();
}

void CCaseEntry::PostProcess()
{
    CLog log("Post processing case label...");

    // Get the base type of the enum entity and insert it in front of the declaration.
    CUnionEntity* pUnionEntity = GetParentEntity()->Get<CUnionEntity>();
    if (!pUnionEntity) throw CCompileException("Internal error: expected an union entity as parent.");

    // Separate between case statement and default
    if (m_bDefault)
    {
        if (!m_lstCaseValue.empty())
            throw CCompileException("Default case label cannot have a value.");


        log << "Default case label" << std::endl;

    } else
    {
        // Get the switch case type
        sdv::idl::EDeclType eSwitchCaseType = sdv::idl::EDeclType::decltype_unknown;
        CEntityPtr ptrSwitchCaseType;
        CValueNodePtr ptrSwitchCaseValue;
        pUnionEntity->GetSwitchCaseType(eSwitchCaseType, ptrSwitchCaseType, ptrSwitchCaseValue);

        // Insert the switch type specific assignment to the token list
        m_lstCaseValue.push_front(CToken("="));
        m_lstCaseValue.push_front(CToken("label"));
        if (!ptrSwitchCaseType)
        {
            switch (eSwitchCaseType)
            {
            case sdv::idl::EDeclType::decltype_short:               m_lstCaseValue.push_front(CToken("short"));              break;
            case sdv::idl::EDeclType::decltype_long:                m_lstCaseValue.push_front(CToken("long"));               break;
            case sdv::idl::EDeclType::decltype_long_long:           m_lstCaseValue.push_front(CToken("long long"));          break;
            case sdv::idl::EDeclType::decltype_unsigned_short:      m_lstCaseValue.push_front(CToken("unsigned short"));     break;
            case sdv::idl::EDeclType::decltype_unsigned_long:       m_lstCaseValue.push_front(CToken("unsigned long"));      break;
            case sdv::idl::EDeclType::decltype_unsigned_long_long:  m_lstCaseValue.push_front(CToken("unsigned long long")); break;
            case sdv::idl::EDeclType::decltype_octet:               m_lstCaseValue.push_front(CToken("octet"));              break;
            case sdv::idl::EDeclType::decltype_char:                m_lstCaseValue.push_front(CToken("char"));               break;
            case sdv::idl::EDeclType::decltype_char16:              m_lstCaseValue.push_front(CToken("char16"));             break;
            case sdv::idl::EDeclType::decltype_char32:              m_lstCaseValue.push_front(CToken("char32"));             break;
            case sdv::idl::EDeclType::decltype_wchar:               m_lstCaseValue.push_front(CToken("wchar"));              break;
            case sdv::idl::EDeclType::decltype_boolean:             m_lstCaseValue.push_front(CToken("boolean"));            break;
            case sdv::idl::EDeclType::decltype_native:              m_lstCaseValue.push_front(CToken("native"));             break;
            default: throw CCompileException(m_tokenLabel, "Internal error: invalid switch case data type.");
            }
        }
        else
            m_lstCaseValue.push_front(CToken(ptrSwitchCaseType->GetScopedName()));
        m_lstCaseValue.push_back(CToken(";", ETokenType::token_separator));

        // Create the label value.
        m_ptrLabel = std::make_shared<CVariableEntity>(GetContext(), shared_from_this(), m_lstCaseValue, true, false);

        // Process the label variable (this will resolve any assigned value and constant).
        m_ptrLabel->Process();
        CVariableEntity* pLabelVariableEntity = m_ptrLabel->Get<CVariableEntity>();
        if (!pLabelVariableEntity)
            throw CCompileException("Internal error: expected a variable entity as case label.");
        pLabelVariableEntity->PostProcess();

        log << "Case label is: " << GetLabel() << std::endl;
    }

    // Post process the assignment
    CVariableEntity::PostProcess();
}

CUnionEntity::CUnionEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    CStructEntity(rptrContext, ptrParent)
{}

sdv::interface_t CUnionEntity::GetInterface(sdv::interface_id idInterface)
{
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IUnionEntity>())
        return static_cast<sdv::idl::IUnionEntity*>(this);
    return CStructEntity::GetInterface(idInterface);
}

sdv::idl::IUnionEntity::ESwitchInterpret CUnionEntity::GetSwitchInterpretation() const
{
    return m_eSwitchInterpret;
}

void CUnionEntity::GetSwitchType(/*out*/ sdv::idl::EDeclType& reType, /*out*/ sdv::IInterfaceAccess*& rpType) const
{
    reType = m_typedeclSwitch.GetBaseType();
    rpType = m_typedeclSwitch.GetTypeDefinition();
}

void CUnionEntity::GetSwitchVar(/*out*/ sdv::u8string& rssVarStr, /*out*/ sdv::IInterfaceAccess*& rpVarEntity,
    /*out*/ sdv::IInterfaceAccess*& rpVarContainer) const
{
    rssVarStr.clear();
    rpVarEntity = nullptr;
    rpVarContainer = nullptr;
    if (m_eSwitchInterpret == sdv::idl::IUnionEntity::ESwitchInterpret::switch_type) return;
    rssVarStr = m_ssValueNode;
    rpVarEntity = m_ptrSwitchValueNode ? m_ptrSwitchValueNode->GetDeclEntity()->Get<sdv::IInterfaceAccess>() : nullptr;
    rpVarContainer = m_ptrContainer ? m_ptrContainer->Get<sdv::IInterfaceAccess>() : nullptr;
}

std::string CUnionEntity::GetDeclTypeStr(bool /*bResolveTypedef*/) const
{
    return std::string("union ") + GetScopedName();
}

void CUnionEntity::Process()
{
    // The definition and declaration can be defined:
    // union <union_identifier>;                                                --> forward declaration
    // union <union_identifier> switch(<type>) {...};                           --> union definition with type
    // struct { <type> <var>; union <union_identifier> switch(<var>) {...};};   --> union definition with varaiable
    // <union_identifier> <decl_identifier>;                       --> union variable declaration
    // <union_identifier> <decl_identifier> = {...};               --> union variable declaration and assignment
    // union <union_identifier> <decl_identifier>;                --> union variable declaration
    // union <union_identifier> <decl_identifier> = {...};        --> union variable declaration and assignment
    // union <union_identifier> {...} <decl_identifier>;          --> union definition and variable declaration
    // union <union_identifier> {...} <decl_identifier> = {...};  --> union definition, variable declaration and assignment
    // union {...} <decl_identifier>;                              --> anonymous union definition and variable declaration
    // union {...} <decl_identifier> = {...};                      --> anonymous union definition, variable declaration and assignment

    // typedef <union_identifier> <type_identifier>;
    // typedef union <union_identifier> { ... } <type_identifier>;
    // typedef <union_identifier> <type_identifier>;
    // typedef union <union_identifier> { ... } <type_identifier>;
    // typedef union { ... } <type_identifier>;
    // const union <union_identifier> <decl_identifier> = {...};
    // const <union_identifier> <decl_identifier> = {...};
    // const union {...} <decl_identifier> = {...};

    // The declaration is as follows:
    // <union_identifier> <decl_identifier>;
    // union <union_identifier> <decl_identifier>;
    // union <union_identifier> { ... } <decl_identifier>;

    CStructEntity::Process();
}

void CUnionEntity::ProcessDefinitionAddendum()
{
    //unions are defined as :
    //union <union_identifier> switch (<type>)-- > type is a integral or enum type or a typedef of this
    //{
    //case <value>: <type> <var_identifier>       -- > case values need to be unique
    //    ...
    //};
    //struct
    //{
    //    <type> <var>;
    //    union <union_identifier> switch (<var>)-- > var is an integral or enum value, which is part of the struct
    //    {
    //    case <value>: <type> <var_identifier>   -- > case values need to be unique
    //        ...
    //    };
    //};

    if (PeekToken() == ";") return; // Forward declaration
    CToken token = GetToken();
    if (token != "switch") throw CCompileException(token, "Expecting a switch statement following the union identifier.");
    token = GetToken();
    if (token != "(") throw CCompileException(token, "Expecting a left bracket '('.");
    size_t nIndex = 0;
    while (true)
    {
        token = PeekToken(nIndex++);
        if (!token || token == ")") break;
        m_ssSwitchVar += static_cast<std::string>(token);
    }
    m_typedeclSwitch = ProcessType(true);
    token = GetToken();
    while (token && token != ")")
    {
        m_ssValueNode += static_cast<std::string>(token);
        token = GetToken();
    }
    if (token != ")") throw CCompileException(token, "Expecting a right bracket ')'.");

    // Was it possible to resolve the type for the switch case. If not, the switch case is a variable?
    if (m_typedeclSwitch.GetBaseType() == sdv::idl::EDeclType::decltype_unknown)
        m_eSwitchInterpret = sdv::idl::IUnionEntity::ESwitchInterpret::switch_variable;
}

void CUnionEntity::PostProcess()
{
    if (ForwardDeclaration()) return;

    // Was the switch case type/variable found before? If not, check for a variable.
    if (m_eSwitchInterpret == sdv::idl::IUnionEntity::ESwitchInterpret::switch_variable)
    {
        // Get the switch type object
        CEntityPtr ptrSwitchVarBase = Find(m_typedeclSwitch.GetTypeString());
        if (!ptrSwitchVarBase)
            throw CCompileException(m_ssSwitchVar,
                "The switch case must be determined by a predefined type or a member variable.");

        // Proper relative name (without global scope)
        std::string ssSwitchFullName = ptrSwitchVarBase->GetName() + m_ssValueNode;
        m_ssValueNode = ssSwitchFullName;

        // The type must be a variable type
        CVariableEntity* pVarEntityBase = ptrSwitchVarBase->Get<CVariableEntity>();
        if (!pVarEntityBase)
            throw CCompileException(m_ssSwitchVar,
                "The switch case is not determined by a member variable nor a predefined type.");

        // The parent of the base is the common parent of both the switch var and the union.
        m_ptrContainer = ptrSwitchVarBase->GetParentEntity();
        if (!m_ptrContainer)
            throw CCompileException(m_ssSwitchVar,
                "The switch case variable and the union do not share a common parent.");

        // Find the value node of the switch variable (in case it is a child some layers deep).
        CValueNodePtr ptrSwitchValueNode;
        if (!m_ssValueNode.empty())
            m_ptrSwitchValueNode = pVarEntityBase->FindValue(m_ssValueNode);
        else
            m_ptrSwitchValueNode = pVarEntityBase->ValueRef();
        if (!m_ptrSwitchValueNode)
            throw CCompileException(m_ssSwitchVar, "Could not find the switch variable.");

        // Change the variable type entity to a switch variable.
        m_ptrSwitchValueNode->GetDeclEntity()->Get<CVariableEntity>()->UseAsSwitchVariable();

        // Assign the type of the variable.
        m_typedeclSwitch.SetTypeDefinitionEntityPtr(m_ptrSwitchValueNode->GetDeclEntity()->GetTypeEntity());
        m_typedeclSwitch.SetBaseType(m_ptrSwitchValueNode->GetDeclEntity()->GetBaseType());
        if (!IsIntegralDeclType(m_typedeclSwitch.GetBaseType()) &&
            m_typedeclSwitch.GetBaseType() != sdv::idl::EDeclType::decltype_enum)
            throw CCompileException(m_ssSwitchVar, "Expecting an integral or enum identifier type or variable.");

        // Check whether one of the parents is a struct or a union and the variable is a struct member.
        CEntityPtr ptrParent = GetParentEntity();
        if (!ptrParent ||
            ((ptrParent->GetType() != sdv::idl::EEntityType::type_struct) &&
                (ptrParent->GetType() != sdv::idl::EEntityType::type_exception)))
            throw CCompileException(m_ssSwitchVar,
                "The union needs to be part of a struct or an exception when used with a variable based switch case.");
        while (ptrSwitchVarBase)
        {
            if (!ptrParent ||
                ((ptrParent->GetType() != sdv::idl::EEntityType::type_struct) &&
                    (ptrParent->GetType() != sdv::idl::EEntityType::type_exception)))
                throw CCompileException(m_ssSwitchVar,
                    "The variable used in the switch case must be a member of the same or parent struct or exception the union "
                    "is declared in.");
            if (ptrParent->GetType() == sdv::idl::EEntityType::type_struct &&
                ptrParent == ptrSwitchVarBase->GetParentEntity())
                break;

            ptrParent = ptrParent->GetParentEntity();
        }
    }

    // Post process all the case statements
    std::set<std::string> setValues;
    bool bDefaultFound = false;
    for (CEntityPtr ptrEntity : m_lstDeclMembers)
    {
        CCaseEntry* pCaseEntry = ptrEntity->Get<CCaseEntry>();
        if (!pCaseEntry) throw CCompileException("Internal error: unexpected entity stored at union.");
        pCaseEntry->PostProcess();

        // Differentiate between default and standard case label...
        if (pCaseEntry->IsDefault())
        {
            if (bDefaultFound)
				throw CCompileException(pCaseEntry->GetLabelToken(), "Duplicate default switch found.");
            bDefaultFound = true;
        } else
        {
            // Check for the existence of the label.
            std::string ssLabel = pCaseEntry->GetLabel();
            if (m_setValues.find(ssLabel) != m_setValues.end())
                throw CCompileException(pCaseEntry->GetLabelToken(), "Duplicate switch case label found.");
            m_setValues.insert(ssLabel);
        }
    }

    // If supported create the value node for the definition (this allows assignments of values to this entity).
    CreateValueNode();
}

bool CUnionEntity::Supports(EDefinitionSupport eSupport) const
{
    switch (eSupport)
    {
        case EDefinitionSupport::support_case_declaration:  return true;
        case EDefinitionSupport::support_variable:          return false;
        case EDefinitionSupport::support_const_variable:    return false;
        case EDefinitionSupport::support_typedef:           return false;
        case EDefinitionSupport::support_struct:            return false;
        case EDefinitionSupport::support_union:             return false;
        case EDefinitionSupport::support_enum:              return false;
        default:                                            return false;
    }
}

void CUnionEntity::CreateValueNode()
{
    // Create a compound type value node for this definition.
    ValueRef() = std::make_shared<CCompoundTypeValueNode>(shared_from_this(), nullptr);
}

bool CUnionEntity::RequireDeclaration() const
{
    // Require a declaration when unnamed union.
    return !ForwardDeclaration() && IsUnnamed();
}

bool CUnionEntity::AllowAutoTransparentDeclaration() const
{
    return !ForwardDeclaration() && IsUnnamed();
}

void CUnionEntity::GetSwitchCaseType(sdv::idl::EDeclType& reType, CEntityPtr& rptrType, CValueNodePtr& rptrValue)
{
    reType = m_typedeclSwitch.GetBaseType();
    rptrType = m_typedeclSwitch.GetTypeDefinitionEntityPtr();
    if (m_ptrSwitchValueNode) rptrValue = m_ptrSwitchValueNode;
}
