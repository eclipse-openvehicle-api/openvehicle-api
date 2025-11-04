#include <fstream>
#include <gtest/gtest.h>
#include "../../../global/process_watchdog.h"
#include <support/mem_access.h>

#if defined(_WIN32) && defined(_UNICODE)
extern "C" int wmain(int argc, wchar_t* argv[])
#else
extern "C" int main(int argc, char* argv[])
#endif
{
	CProcessWatchdog watchdog;

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(MemoryManagerTest, Instantiate)
{
	EXPECT_NE(sdv::core::GetMemMgr(), nullptr);
}

TEST(MemoryManagerTest, Allocate)
{
	sdv::core::IMemoryAlloc* pAlloc = sdv::core::GetMemMgr();
	ASSERT_NE(pAlloc, nullptr);
	sdv::pointer<uint8_t> ptr = pAlloc->Allocate(1024 * 1024);
	ASSERT_TRUE(ptr);
	EXPECT_EQ(ptr.size(), 1024 * 1024);
	ptr.reset();
	ASSERT_FALSE(ptr);
}

TEST(MemoryManagerTest, AllocateSupportFunc)
{
	sdv::pointer<uint32_t> ptr = sdv::core::AllocMem<uint32_t>(1024 * 1024);
	ASSERT_TRUE(ptr);
	EXPECT_EQ(ptr.size(), 1024 * 1024);
	ptr.reset();
	ASSERT_FALSE(ptr);
}

TEST(MemoryManagerTest, ReallocateSupportFunc)
{
	sdv::pointer<uint32_t> ptr = sdv::core::AllocMem<uint32_t>(1024 * 1024);
	ASSERT_TRUE(ptr);
	EXPECT_EQ(ptr.size(), 1024 * 1024);
	ptr.resize(512 * 1024);
	EXPECT_EQ(ptr.size(), 512 * 1024);
	ptr.resize(2048 * 1024);
	EXPECT_EQ(ptr.size(), 2048 * 1024);
	ptr.reset();
	ASSERT_FALSE(ptr);
}

