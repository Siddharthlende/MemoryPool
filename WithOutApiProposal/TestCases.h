#include <iostream>
#include <gtest/gtest.h>
#include "MemoryPool.h"
#include <thread>
//#include "TestCases.h"

class Testcases : public ::testing::Test
{
    protected:
    MemPool * pool;
    //subPoolManager * SubPoolManager;
    public:
    bool AllocateMoreMemoryThanLimit()
    {
        pool = new MemPool(5000);
        if(pool!=nullptr)
        {
            return false;
        }
        return true;
        /*SubPoolManager = createSubPool(5000,4);
        if(SubPoolManager!=nullptr)
        {
            return false;
        }*/
    }
     /* bool ToCheckThreadSafe()
    {
        std::thread t1(allocate);
        t1.detach();
 
        std::thread t2(allocate);
        t2.detach();
    }*/

    bool AllocatingMoreChunkThanPoolCanHold()
    {
        pool = new MemPool(40);
        int* p1 = (int *)pool->AllocateMemory("int");
    *p1 = 4;
    int* p2 = (int *)pool->AllocateMemory("int");
    *p2 = 4;

    int* p3 = (int *)pool->AllocateMemory("int");
    *p3 = 4;
    int* p4 = (int *)pool->AllocateMemory("int");
    *p4 = 4;
    int* p5 = (int *)pool->AllocateMemory("int");
    *p5 = 4;
    int* p6 = (int *)pool->AllocateMemory("int");
    *p6 = 4;
    int* p7 = (int *)pool->AllocateMemory("int");
    *p7 = 4;
    int* p8 = (int *)pool->AllocateMemory("int");
    *p8 = 4;
    int* p9 = (int *)pool->AllocateMemory("int");
    *p9 = 4;
    int* p10 = (int *)pool->AllocateMemory("int");
    *p10 = 4;
    int* p11 = (int *)pool->AllocateMemory("int");
    if(p11 !=nullptr)
    {
        return false;
    }
    return true;
    }

    bool DeallocateMemFromPoolWhoesptrNotFromOurPool()
    {
        pool = new MemPool(40);
        int * ptr = new int;
        if((pool->deallocateMemory(ptr, "int")))
        {
            return false;
        }
        return true;
        
    }

    void TestBody()
    {
        std::cout<<"TEST CASES"<<std::endl;
    }
    void setup()
    {

    }
    void TearDown()
    {

    }

};