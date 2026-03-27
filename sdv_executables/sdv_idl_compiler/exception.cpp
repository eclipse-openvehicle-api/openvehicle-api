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

#include "exception.h"
#include <cassert>

CCompileException::CCompileException(const sdv::idl::XCompileError& rxCompileError)
{
    static_cast<sdv::idl::XCompileError&>(*this) = rxCompileError;
}

std::string CCompileException::GetPath() const
{
    return ssFile;
}

std::string CCompileException::GetReason() const
{
    return ssReason;
}

void CCompileException::SetPath(const std::filesystem::path& rpath)
{
    if (ssFile.empty())
        ssFile = rpath.generic_u8string();
}

void CCompileException::SetLocation(const CToken& rtoken)
{
    uiLine = rtoken.GetLine();
    uiCol = rtoken.GetCol();
    ssToken = rtoken;
}

uint32_t CCompileException::GetLineNo() const
{
    return uiLine;
}

uint32_t CCompileException::GetColNo() const
{
    return uiCol;
}

std::string CCompileException::GetToken() const
{
    return ssToken;
}

std::string CCompileException::GetLine() const
{
    return ssLine;
}
