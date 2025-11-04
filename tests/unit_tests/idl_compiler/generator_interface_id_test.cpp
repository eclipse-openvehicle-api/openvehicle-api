#include "includes.h"
#include <interfaces/core.h>
#include "parser_test.h"
#include <fstream>

using CGeneratorInterfaceIdTest = CParserTest;

namespace id1
{
	namespace sdv
	{
		using interface_id = ::sdv::interface_id;
	}
#include "generated/test_ifc_id1.h"
}
namespace id2
{
	namespace sdv
	{
		using interface_id = ::sdv::interface_id;
	}
#include "generated/test_ifc_id2.h"
}

TEST_F(CGeneratorInterfaceIdTest, EmptyIdentical)
{
    EXPECT_EQ(sdv::GetInterfaceId<id1::IEmptyIdentical>(), sdv::GetInterfaceId<id2::IEmptyIdentical>());
}

TEST_F(CGeneratorInterfaceIdTest, Identical)
{
	EXPECT_EQ(sdv::GetInterfaceId<id1::IIdentical>(), sdv::GetInterfaceId<id2::IIdentical>());
}

TEST_F(CGeneratorInterfaceIdTest, EmptyDifferent)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IEmptyDifferent1>(), sdv::GetInterfaceId<id2::IEmptyDifferent2>());
}

TEST_F(CGeneratorInterfaceIdTest, Different)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferent1>(), sdv::GetInterfaceId<id2::IDifferent2>());
}

TEST_F(CGeneratorInterfaceIdTest, DiffentModule)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::mod1::IDiffentModule>(), sdv::GetInterfaceId<id2::mod2::IDiffentModule>());
}

TEST_F(CGeneratorInterfaceIdTest, DiffentParentModule)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::mod_parent1::mod::IEmptyIdentical>(), sdv::GetInterfaceId<id2::mod_parent2::mod::IEmptyIdentical>());
}

TEST_F(CGeneratorInterfaceIdTest, DiffentMemberFunc)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDiffentMemberFunc>(), sdv::GetInterfaceId<id2::IDiffentMemberFunc>());
}

TEST_F(CGeneratorInterfaceIdTest, DiffentMemberFuncOrder)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDiffentMemberFuncOrder>(), sdv::GetInterfaceId<id2::IDiffentMemberFuncOrder>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncVisibility)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncVisibility>(), sdv::GetInterfaceId<id2::IDifferentFuncVisibility>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAddFunc)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAddFunc>(), sdv::GetInterfaceId<id2::IDifferentAddFunc>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAddFuncParam)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAddFuncParam>(), sdv::GetInterfaceId<id2::IDifferentAddFuncParam>());
}

TEST_F(CGeneratorInterfaceIdTest, ComplexParamChange)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IComplexParamChange>(), sdv::GetInterfaceId<id2::IComplexParamChange>());
}

TEST_F(CGeneratorInterfaceIdTest, ComplexReturnValChange)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IComplexReturnValChange>(), sdv::GetInterfaceId<id2::IComplexReturnValChange>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncParamOrder)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncParamOrder>(), sdv::GetInterfaceId<id2::IDifferentFuncParamOrder>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncRetVal)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncRetVal>(), sdv::GetInterfaceId<id2::IDifferentFuncRetVal>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncParamName)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncParamName>(), sdv::GetInterfaceId<id2::IDifferentFuncParamName>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncParamType)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncParamType>(), sdv::GetInterfaceId<id2::IDifferentFuncParamType>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncParamDirection)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncParamDirection>(), sdv::GetInterfaceId<id2::IDifferentFuncParamDirection>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncAddException)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncAddException>(), sdv::GetInterfaceId<id2::IDifferentFuncAddException>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncChangeException)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncChangeException>(), sdv::GetInterfaceId<id2::IDifferentFuncChangeException>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncAdditionalException)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncAdditionalException>(), sdv::GetInterfaceId<id2::IDifferentFuncAdditionalException>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncExceptionOrder)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncExceptionOrder>(), sdv::GetInterfaceId<id2::IDifferentFuncExceptionOrder>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncExceptionName)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncExceptionName>(), sdv::GetInterfaceId<id2::IDifferentFuncExceptionName>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncExceptionType)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncExceptionType>(), sdv::GetInterfaceId<id2::IDifferentFuncExceptionType>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAttrName)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAttrName>(), sdv::GetInterfaceId<id2::IDifferentAttrName>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAttrType)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAttrType>(), sdv::GetInterfaceId<id2::IDifferentAttrType>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAddAttr)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAddAttr>(), sdv::GetInterfaceId<id2::IDifferentAddAttr>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAttrVisibility)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAttrVisibility>(), sdv::GetInterfaceId<id2::IDifferentAttrVisibility>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAttrAddException)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAttrAddException>(), sdv::GetInterfaceId<id2::IDifferentAttrAddException>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAttrChangeException)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAttrChangeException>(), sdv::GetInterfaceId<id2::IDifferentAttrChangeException>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAttrAdditionalException)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAttrAdditionalException>(), sdv::GetInterfaceId<id2::IDifferentAttrAdditionalException>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAttrExceptionDirection)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAttrExceptionDirection>(), sdv::GetInterfaceId<id2::IDifferentAttrExceptionDirection>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentFuncExceptionDirection2)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentFuncExceptionDirection2>(), sdv::GetInterfaceId<id2::IDifferentFuncExceptionDirection2>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAttrExceptionName)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAttrExceptionName>(), sdv::GetInterfaceId<id2::IDifferentAttrExceptionName>());
}

TEST_F(CGeneratorInterfaceIdTest, DifferentAttrExceptionType)
{
	EXPECT_NE(sdv::GetInterfaceId<id1::IDifferentAttrExceptionType>(), sdv::GetInterfaceId<id2::IDifferentAttrExceptionType>());
}

TEST_F(CGeneratorInterfaceIdTest, IdenticalAddConstDecl)
{
	EXPECT_EQ(sdv::GetInterfaceId<id1::IIdenticalAddConstDecl>(), sdv::GetInterfaceId<id2::IIdenticalAddConstDecl>());
}

TEST_F(CGeneratorInterfaceIdTest, IdenticalAddComments)
{
	EXPECT_EQ(sdv::GetInterfaceId<id1::IIdenticalAddComments>(), sdv::GetInterfaceId<id2::IIdenticalAddComments>());
}

TEST_F(CGeneratorInterfaceIdTest, IdenticalAddChildDef)
{
	EXPECT_EQ(sdv::GetInterfaceId<id1::IIdenticalAddChildDef>(), sdv::GetInterfaceId<id2::IIdenticalAddChildDef>());
}

