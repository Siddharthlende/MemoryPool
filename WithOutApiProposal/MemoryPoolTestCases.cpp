#include <gtest/gtest.h>
#include "MemoryPool.h"
#include "TestCases.h"

TEST(Testcases, Allocate)
{
    Testcases obj;
    //MemPool * Pool = new MemPool(5000);
    //ASSERT_EQ(true, obj.AllocateMoreMemoryThanLimit());
    ASSERT_EQ(true, obj.AllocatingMoreChunkThanPoolCanHold());
    ASSERT_EQ(true, obj.DeallocateMemFromPoolWhoesptrNotFromOurPool());
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}