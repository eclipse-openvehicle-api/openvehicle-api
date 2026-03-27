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

#include "includes.h"
#include "parser_test.h"
#include "../../../sdv_executables/sdv_idl_compiler/entities/module_entity.cpp"
#include "../../../sdv_executables/sdv_idl_compiler/parser.h"

using CParserModuleTest = CParserTest;

TEST_F(CParserModuleTest, ParsingNonExistentModule)
{
    EXPECT_FALSE(CParser("").Parse().Root()->Find("Test"));
    EXPECT_THROW(CParser("{};").Parse(), CCompileException);
    EXPECT_THROW(CParser("};").Parse(), CCompileException);
    EXPECT_THROW(CParser("}").Parse(), CCompileException);
    EXPECT_NO_THROW(CParser(";").Parse());
}

TEST_F(CParserModuleTest, ParsingModule)
{
    EXPECT_TRUE(CParser("module Test {};").Parse().Root()->Find("Test"));
}

TEST_F(CParserModuleTest, ParsingRootModule)
{
    EXPECT_TRUE(CParser("module Test {};").Parse().Root()->Find("::Test"));
}

TEST_F(CParserModuleTest, ParsingNestedModules)
{
    EXPECT_TRUE(CParser("module Test { module Test2 {}; };").Parse().Root()->Find("Test::Test2"));
}

TEST_F(CParserModuleTest, ParsingJoinModules)
{
    EXPECT_NO_THROW(CParser("module Test { module Test2 {}; };\nmodule Test { module Test3 {}; };").Parse().Root()->Find("Test::Test3"));
}

TEST_F(CParserModuleTest, ParsingJoinNestedModules)
{
    EXPECT_NO_THROW(CParser("module Test { module Test2 {}; };\nmodule Test { module Test2 { module Test2b {}; }; };").Parse().Root()->Find("Test::Test2::Test2b"));
}

