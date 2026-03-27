/********************************************************************************
 * Copyright (c) 2025-2026 ZF Friedrichshafen AG
 *
 * This program and the accompanying materials are made available under the 
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contributors: 
 *   Erik Verhoeven - initial API and implementation 
 ********************************************************************************/

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
