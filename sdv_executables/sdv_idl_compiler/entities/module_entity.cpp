#include "module_entity.h"
#include "typedef_entity.h"
#include "struct_entity.h"
#include "union_entity.h"
#include "interface_entity.h"
#include "exception_entity.h"
#include "../exception.h"
#include "../token.h"

CModuleEntity::CModuleEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    CDefinitionEntity(rptrContext, ptrParent)
{}

CModuleEntity::CModuleEntity(CParser& rParser, const CContextPtr& rptrContext) :
    CDefinitionEntity(rParser, rptrContext)
{}

void CModuleEntity::Process()
{
    CDefinitionEntity::Process();
}

bool CModuleEntity::Supports(EDefinitionSupport eSupport) const
{
    switch (eSupport)
    {
        case EDefinitionSupport::support_module:            return true;
        case EDefinitionSupport::support_const_variable: return true;
        case EDefinitionSupport::support_typedef:           return true;
        case EDefinitionSupport::support_struct:            return true;
        case EDefinitionSupport::support_union:             return true;
        case EDefinitionSupport::support_enum:              return true;
        case EDefinitionSupport::support_interface:         return true;
        case EDefinitionSupport::support_exception:         return true;
        default:                                            return false;
    }
}

bool CModuleEntity::IsExtendable() const
{
    // Allow extending the module.
    return true;
}

bool CModuleEntity::SupportsChildren() const
{
    // Allow the support of children.
    return true;
}
