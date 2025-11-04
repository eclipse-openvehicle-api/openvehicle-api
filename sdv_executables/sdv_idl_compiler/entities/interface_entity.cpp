#include "interface_entity.h"
#include "../exception.h"
#include "../logger.h"
#include "typedef_entity.h"
#include "variable_entity.h"
#include <iostream>

CInterfaceEntity::CInterfaceEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent, bool bIsLocal) :
    CDefinitionEntity(rptrContext, ptrParent), m_bIsLocal(bIsLocal)
{}

sdv::interface_t CInterfaceEntity::GetInterface(sdv::interface_id idInterface)
{
    // Expose interfaces
    if (idInterface == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
        return static_cast<sdv::idl::IInterfaceEntity*>(this);
    if (idInterface == sdv::GetInterfaceId<sdv::idl::IInterfaceEntity>())
        return static_cast<sdv::idl::IInterfaceEntity*>(this);
    return CDefinitionEntity::GetInterface(idInterface);
}

std::string CInterfaceEntity::GetDeclTypeStr(bool /*bResolveTypedef*/) const
{
    return std::string("interface ") + GetScopedName();
}

void CInterfaceEntity::Process()
{
    // The definition and declaration can be defined:
    // struct <struct_identifier>;                                  --> forward declaration
    // struct <struct_identifier> {...};                            --> struct definition
    // struct <struct_identifier> : <base_struct,...> {...};        --> struct definition with inheritance
    // <struct_identifier> <decl_identifier>;                       --> struct variable declaration
    // <struct_identifier> <decl_identifier> = {...};               --> struct variable declaration and assignment
    // struct <struct_identifier> <decl_identifier>;                --> struct variable declaration
    // struct <struct_identifier> <decl_identifier> = {...};        --> struct variable declaration and assignment
    // struct <struct_identifier> {...} <decl_identifier>;          --> struct definition and variable declaration
    // struct <struct_identifier> : <base_struct,...> {...} <decl_identifier>;  --> struct definition with inheritance and variable declaration
    // struct <struct_identifier> {...} <decl_identifier> = {...};  --> struct definition, variable declaration and assignment
    // struct <struct_identifier> : <base_struct,...> {...} <decl_identifier> = {...};  --> struct definition with inheritance, variable declaration and assignment
    // struct {...} <decl_identifier>;                              --> anonymous struct definition and variable declaration
    // struct : <base_struct,...> {...} <decl_identifier>;          --> anonymous struct definition with inheritance and variable declaration
    // struct {...} <decl_identifier> = {...};                      --> anonymous struct definition, variable declaration and assignment
    // struct : <base_struct,...> {...} <decl_identifier> = {...};  --> anonymous struct definition with inheritance, variable declaration and assignment

    // typedef <struct_identifier> <type_identifier>;
    // typedef struct <struct_identifier> { ... } <type_identifier>;
    // typedef <struct_identifier> <type_identifier>;
    // typedef struct <struct_identifier> { ... } <type_identifier>;
    // typedef struct { ... } <type_identifier>;
    // const struct <struct_identifier> <decl_identifier> = {...};
    // const <struct_identifier> <decl_identifier> = {...};
    // const struct {...} <decl_identifier> = {...};

    // The declaration is as follows:
    // <struct_identifier> <decl_identifier>;
    // struct <struct_identifier> <decl_identifier>;
    // struct <struct_identifier> { ... } <decl_identifier>;


    // TODO Enforce inheritance from sdv::IInterfaceAccess unless the interface is sdv::IInterfaceAccess.

    CDefinitionEntity::Process();
}

bool CInterfaceEntity::Supports(EDefinitionSupport eSupport) const
{
    switch (eSupport)
    {
        case EDefinitionSupport::support_const_variable: return true;
        case EDefinitionSupport::support_typedef:           return true;
        case EDefinitionSupport::support_struct:            return true;
        case EDefinitionSupport::support_union:             return true;
        case EDefinitionSupport::support_enum:              return true;
        case EDefinitionSupport::support_attribute:         return true;
        case EDefinitionSupport::support_operation:         return true;
        default:                                            return false;
    }
}

void CInterfaceEntity::CreateValueNode()
{
    // Create a compound type value node for this definition.
    ValueRef() = std::make_shared<CInterfaceValueNode>(shared_from_this(), nullptr);
}