#include "includes.h"
#include "parser_test.h"
#include <fstream>
#include <interfaces/core_idl.h>

using CIdlInterfaceIdCompatibilityTest = CParserTest;

// Backup namespace to differentiate between generated and defined in backup file.
namespace bck
{
	using u8string = ::sdv::u8string;
	template <class T, size_t nFixedSize = 0>
	using sequence = ::sdv::sequence<T, nFixedSize>;
	template <typename T /*= uint8_t*/>
	using pointer = ::sdv::pointer<T>;
	namespace internal
	{
		using IInternalMemAlloc = ::sdv::internal::IInternalMemAlloc;
		template <typename T>
		inline bck::pointer<T> make_ptr(internal::IInternalMemAlloc* /*pAllocator*/, size_t /*nSize*/)
		{
            return {};
		}
	}
}

// Reload the backup file, but change the namespace to "bck".
#define sdv bck
#undef IDL_PARSER_INTERFACE_H
#undef SDV_INTERFACE_H
#undef SDV_EXCEPT_H
#include "../../../sdv_executables/sdv_idl_compiler/core_idl_backup.h"
#undef sdv

/**
 * @brief Use inline non-constexpr function to get the ID.
 * @details The standard GetInterfaceId and GetExceptionId functions are constexpr and cause to return the same ID for the sdv and
 * the bck object, thus not detecting any changes. This is due to the use of "constexpr", implementing the return value of the
 * function at compile time. By using an inline function, the access of the ID occurs at runtime allowing to differentiate between
 * the bck and sdv IDs.
 * @tparam TObj The object to retrieve the ID for.
 * @return The ID of the object.
*/
template <typename TObj>
inline uint64_t GetId()
{
	return TObj::_id;
}

TEST_F(CIdlInterfaceIdCompatibilityTest, InterfaceAccess)
{
	EXPECT_EQ(GetId<sdv::IInterfaceAccess>(), GetId<bck::IInterfaceAccess>());
}

TEST_F(CIdlInterfaceIdCompatibilityTest, SdvExceptions)
{
	EXPECT_EQ(GetId<sdv::XNoInterface>(), GetId<bck::XNoInterface>());
	EXPECT_EQ(GetId<sdv::XIndexOutOfRange>(), GetId<bck::XIndexOutOfRange>());
	EXPECT_EQ(GetId<sdv::XInvalidIterator>(), GetId<bck::XInvalidIterator>());
	EXPECT_EQ(GetId<sdv::XNullPointer>(), GetId<bck::XNullPointer>());
	EXPECT_EQ(GetId<sdv::XInvalidRefCount>(), GetId<bck::XInvalidRefCount>());
	EXPECT_EQ(GetId<sdv::XBufferTooSmall>(), GetId<bck::XBufferTooSmall>());
	EXPECT_EQ(GetId<sdv::XHashNotMatching>(), GetId<bck::XHashNotMatching>());
	EXPECT_EQ(GetId<sdv::XOffsetPastBufferSize>(), GetId<bck::XOffsetPastBufferSize>());
	EXPECT_EQ(GetId<sdv::XUnhandledException>(), GetId<bck::XUnhandledException>());
}

TEST_F(CIdlInterfaceIdCompatibilityTest, CoreExceptions)
{
	EXPECT_EQ(GetId<sdv::core::XNoMemMgr>(), GetId<bck::core::XNoMemMgr>());
	EXPECT_EQ(GetId<sdv::core::XAllocFailed>(), GetId<bck::core::XAllocFailed>());
}

TEST_F(CIdlInterfaceIdCompatibilityTest, CompilerInterfaces)
{
	EXPECT_EQ(GetId<sdv::idl::ICompilerOption>(), GetId<bck::idl::ICompilerOption>());
	EXPECT_EQ(GetId<sdv::idl::ICompilerInfo>(), GetId<bck::idl::ICompilerInfo>());
}

TEST_F(CIdlInterfaceIdCompatibilityTest, CompilerExceptions)
{
	EXPECT_EQ(GetId<sdv::idl::XCompileError>(), GetId<bck::idl::XCompileError>());
}

TEST_F(CIdlInterfaceIdCompatibilityTest, EntityInterfaces)
{
	EXPECT_EQ(GetId<sdv::idl::IEntityInfo>(), GetId<bck::idl::IEntityInfo>());
	EXPECT_EQ(GetId<sdv::idl::IEntityIterator>(), GetId<bck::idl::IEntityIterator>());
	EXPECT_EQ(GetId<sdv::idl::IEntityContext>(), GetId<bck::idl::IEntityContext>());
	EXPECT_EQ(GetId<sdv::idl::IMetaEntity>(), GetId<bck::idl::IMetaEntity>());
	EXPECT_EQ(GetId<sdv::idl::IEntityComments>(), GetId<bck::idl::IEntityComments>());
	EXPECT_EQ(GetId<sdv::idl::IForwardDeclarationEntity>(), GetId<bck::idl::IForwardDeclarationEntity>());
	EXPECT_EQ(GetId<sdv::idl::IDeclarationType>(), GetId<bck::idl::IDeclarationType>());
	EXPECT_EQ(GetId<sdv::idl::IDeclarationEntity>(), GetId<bck::idl::IDeclarationEntity>());
	EXPECT_EQ(GetId<sdv::idl::IInterfaceEntity>(), GetId<bck::idl::IInterfaceEntity>());
	EXPECT_EQ(GetId<sdv::idl::IOperationEntity>(), GetId<bck::idl::IOperationEntity>());
	EXPECT_EQ(GetId<sdv::idl::IAttributeEntity>(), GetId<bck::idl::IAttributeEntity>());
	EXPECT_EQ(GetId<sdv::idl::IParameterEntity>(), GetId<bck::idl::IParameterEntity>());
	EXPECT_EQ(GetId<sdv::idl::IEnumEntity>(), GetId<bck::idl::IEnumEntity>());
	EXPECT_EQ(GetId<sdv::idl::IUnionEntity>(), GetId<bck::idl::IUnionEntity>());
	EXPECT_EQ(GetId<sdv::idl::ICaseEntity>(), GetId<bck::idl::ICaseEntity>());
}