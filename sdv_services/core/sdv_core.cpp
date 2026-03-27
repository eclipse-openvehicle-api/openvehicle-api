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

#include "sdv_core.h"
#include "toml_parser_util.h"    // Insert to create the factory of this utility
#include <fstream>
#include <sstream>

extern sdv::IInterfaceAccess* SDVCore()
{
    static CSDVCore& rcore = CSDVCore::GetInstance();
    return &rcore;
}

CSDVCore::CSDVCore()
{}

CSDVCore::~CSDVCore()
{}

CSDVCore& CSDVCore::GetInstance()
{
    static CSDVCore core;
    return core;
}

