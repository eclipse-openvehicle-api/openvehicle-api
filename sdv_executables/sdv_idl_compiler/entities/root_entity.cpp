#include "root_entity.h"
#include "module_entity.h"
#include "../exception.h"
#include "../token.h"


CRootEntity::CRootEntity(CParser& rParser, const CContextPtr& rptrContext) :
    CModuleEntity(rParser, rptrContext)
{}

void CRootEntity::Process()
{
    // Skip the definition and process the content directly.
    ProcessContent();
}

bool CRootEntity::IsExtendable() const
{
    return false;
}

void CRootEntity::AddMeta(const CEntityPtr& ptrMeta)
{
    m_lstMetaEntities.push_back(ptrMeta);
}

const CEntityList& CRootEntity::GetMeta() const
{
    return m_lstMetaEntities;
}
