#include "enum_entity.h"
#include "entity_value.h"
#include "typedef_entity.h"
#include "../exception.h"
#include <set>

CEnumEntry::CEnumEntry(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    CDeclarationEntity(rptrContext, ptrParent)
{}

std::string CEnumEntry::GetDeclTypeStr(bool /*bResolveTypedef*/) const
{
    return GetScopedName();
}

void CEnumEntry::Process()
{
    // Get the base type of the enum entity and insert it in front of the declaration.
    const CEnumEntity* pEnumEntity = GetParentEntity()->Get<CEnumEntity>();
    if (!pEnumEntity) throw CCompileException("Internal error: expected an enum entity as parent.");
	CToken token(DeclTypeToString(pEnumEntity->GetEnumType()));
    PrependToken(token);

    // Process as if normal declaration
    CDeclarationEntity::Process();
}

CEnumEntity::CEnumEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    CDefinitionEntity(rptrContext, ptrParent)
{
    m_typedecl.SetBaseType(sdv::idl::EDeclType::decltype_long);
}

sdv::interface_t CEnumEntity::GetInterface(sdv::interface_id idInterface)
{
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::IInterfaceAccess*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IEnumEntity>())
        return static_cast<sdv::idl::IEnumEntity*>(this);
    return CDefinitionEntity::GetInterface(idInterface);
}

std::string CEnumEntity::GetDeclTypeStr(bool /*bResolveTypedef*/) const
{
    return std::string("enum ") + GetScopedName();
}

void CEnumEntity::GetBaseType(sdv::idl::EDeclType& reType, sdv::IInterfaceAccess*& rpType) const
{
    reType = m_typedecl.GetBaseType();
    rpType = m_typedecl.GetTypeDefinition();
}

void CEnumEntity::Process()
{
    // The definition and declaration can be defined:
    // enum <enum_identifier>;                                          --> forward declaration
    // enum <enum_identifier> {...};                                    --> enum definition
    // <enum_identifier> <decl_identifier>;                             --> enum variable declaration
    // <enum_identifier> <decl_identifier> = <enum_value>;              --> enum variable declaration and assignment
    // enum <enum_identifier> <decl_identifier>;                        --> enum variable declaration
    // enum <enum_identifier> <decl_identifier> = <enum_value>;         --> enum variable declaration and assignment
    // enum <enum_identifier> {...} <decl_identifier>;                  --> enum definition and variable declaration
    // enum <enum_identifier> {...} <decl_identifier> = <enum_value>;   --> enum definition, variable declaration and assignment
    // enum {...} <decl_identifier>;                                    --> anonymous enum definition and variable declaration
    // enum {...} <decl_identifier> = <enum_value>;                     --> anonymous enum definition, variable declaration and assignment
    // const enum <enum_identifier> <decl_identifier> = <enum_value>;
    // const <enum_identifier> <decl_identifier> = <enum_value>;
    // const enum {...} <decl_identifier> = <enum_value>;
    // typedef <enum_identifier> <type_identifier>;
    // typedef enum <enum_identifier> {...} <type_identifier>;
    // typedef <enum_identifier> <type_identifier>;
    // typedef enum <enum_identifier> {...} <type_identifier>;
    // typedef enum {...} <type_identifier>;

    CDefinitionEntity::Process();

    // If supported create the value node for the definition (this allows assignments of values to this entity).
    CreateValueNode();
}

void CEnumEntity::ProcessDefinitionAddendum()
{
    // Check for inheritance
    CToken token = GetToken();
    if (token != ":")
    {
        PrependToken(token);
        return;
    }

    CLog log("Processing inheritance type...");

    // Process the type
    m_typedecl = ProcessType();

    // The base type must be an integral type and not an enum.
    if (!IsIntegralDeclType(m_typedecl.GetBaseType()) && m_typedecl.GetBaseType() != sdv::idl::EDeclType::decltype_enum)
        throw CCompileException("Expecting an integral type to inherit from.");
}

void CEnumEntity::ProcessContent()
{
    CLog log("Processing definition content...");

    // An enum definition consists of one declaration statement with entries separated by a comma.
    CToken token = PeekToken();
    if (!token) return;
    if (token == "}") return;

    // Expecting an enum entry
    CEntityPtr ptrEntry = CreateChild<CEnumEntry>(token.GetContext(), this);
    if (!ptrEntry) throw CCompileException("Internal error: could not create enum entry.");
    ptrEntry->Process();

    // The entries should reside in the const members list
    // Check the values and create values where none is declared.
    // Separate between signed and unsigned.
    int64_t iSignedNext = 0;
    std::set<int64_t> setSigned;
    uint64_t uiUnsignedNext = 0;
    std::set<uint64_t> setUnsigned;
    for (CEntityPtr ptrMember : m_lstConstMembers)
    {
		CEnumEntry* pEntry = ptrMember->Get<CEnumEntry>();
        if (!pEntry) throw CCompileException("Internal error: expected only enum entries in an enumeration.");

        // Do post processing
        pEntry->PostProcess();

        // Get the value from the entity; if one was assigned.
        CSimpleTypeValueNode* pValue = pEntry->ValueRef() ? pEntry->ValueRef()->Get<CSimpleTypeValueNode>() : nullptr;
        if (!pValue)
            throw CCompileException("The value for '", ptrEntry->GetName(), "' must be a system type value.");

        // Deal with signed and unsigned values
        if (IsSignedDeclType(m_typedecl.GetBaseType()))
        {
            // Is a value assigned?
            if (pValue->IsDefined())
            {
                // Check whether the value is already in use.
                int64_t iValue = pValue->Variant().Get<int64_t>();
                if (setSigned.find(iValue) != setSigned.end())
                    throw CCompileException("The value for '", ptrEntry->GetName(), "' is already defined for another entry.");

                // Store the value
                setSigned.insert(iValue);
                iSignedNext = iValue + 1;
            }
            else
            {
                // Get the next available value
                while (setSigned.find(iSignedNext) != setSigned.end())
                    iSignedNext++;

                // Create a value assignment
                CTokenList lstValueTokens;
                lstValueTokens.push_back(CToken(std::to_string(iSignedNext), ETokenLiteralType::token_literal_dec_integer));
                pValue->SetFixedValue(iSignedNext, lstValueTokens);

                // Store the value
                setSigned.insert(iSignedNext);
                iSignedNext++;
            }
        }
        else
        {
            // Is a value assigned?
            if (pValue->IsDefined())
            {
                // Check whether the value is already in use.
                uint64_t uiValue = pValue->Variant().Get<uint64_t>();
                if (setUnsigned.find(uiValue) != setUnsigned.end())
                    throw CCompileException("The value for '", ptrEntry->GetName(), "' is already defined for another entry.");

                // Store the value
                setUnsigned.insert(uiValue);
                uiUnsignedNext = uiValue + 1;
            }
            else
            {
                // Get the next available value
                while (setUnsigned.find(uiUnsignedNext) != setUnsigned.end())
                    uiUnsignedNext++;

                // Create a value assignment
                CTokenList lstValueTokens;
                lstValueTokens.push_back(CToken(std::to_string(uiUnsignedNext), ETokenLiteralType::token_literal_dec_integer));
                pValue->SetFixedValue(uiUnsignedNext, lstValueTokens);

                // Store the value
                setUnsigned.insert(uiUnsignedNext);
                iSignedNext++;
            }
        }
    }
}

bool CEnumEntity::Supports(EDefinitionSupport eSupport) const
{
    switch (eSupport)
    {
        case EDefinitionSupport::support_enum_entry:        return true;
        default:                                            return false;
    }
}

void CEnumEntity::CreateValueNode()
{
    // Create a simple type value node for this definition.
    ValueRef() = std::make_shared<CEnumValueNode>(shared_from_this(), nullptr);
}
