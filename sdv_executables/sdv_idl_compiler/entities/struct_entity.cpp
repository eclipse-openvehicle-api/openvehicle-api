#include "struct_entity.h"
#include "../exception.h"
#include "../logger.h"
#include "typedef_entity.h"
#include "variable_entity.h"
#include <iostream>

CStructEntity::CStructEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    CDefinitionEntity(rptrContext, ptrParent)
{}

std::string CStructEntity::GetDeclTypeStr(bool /*bResolveTypedef*/) const
{
    return std::string("struct ") + GetScopedName();
}

void CStructEntity::Process()
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

    CDefinitionEntity::Process();
}

bool CStructEntity::Supports(EDefinitionSupport eSupport) const
{
    switch (eSupport)
    {
        case EDefinitionSupport::support_variable:       return true;
        case EDefinitionSupport::support_const_variable: return true;
        case EDefinitionSupport::support_typedef:        return true;
        case EDefinitionSupport::support_struct:         return true;
        case EDefinitionSupport::support_union:          return true;
        case EDefinitionSupport::support_enum:           return true;
        default:                                         return false;
    }
}

void CStructEntity::CreateValueNode()
{
    // Create a compound type value node for this definition.
    ValueRef() = std::make_shared<CCompoundTypeValueNode>(shared_from_this(), nullptr);
}