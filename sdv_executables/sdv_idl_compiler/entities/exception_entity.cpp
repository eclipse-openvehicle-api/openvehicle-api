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

#include "exception_entity.h"
#include "../exception.h"
#include "../logger.h"
#include "typedef_entity.h"
#include "variable_entity.h"
#include <iostream>

CExceptionEntity::CExceptionEntity(const CContextPtr& rptrContext, CEntityPtr ptrParent) :
    CStructEntity(rptrContext, ptrParent)
{}

std::string CExceptionEntity::GetDeclTypeStr(bool /*bResolveTypedef*/) const
{
    return std::string("exception ") + GetScopedName();
}

void CExceptionEntity::Process()
{
    CStructEntity::Process();
}

bool CExceptionEntity::Supports(EDefinitionSupport eSupport) const
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

void CExceptionEntity::CreateValueNode()
{
    // Create a compound type value node for this definition.
    ValueRef() = std::make_shared<CCompoundTypeValueNode>(shared_from_this(), nullptr);
}