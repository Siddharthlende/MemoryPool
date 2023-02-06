//  main.cpp

#include <iostream>
#include <chrono>
#include <thread>
#include "MemoryPool.h"
using namespace std;

class Data
{
    char m_data[4096];
};

 MemPool* pool;

void NoPool()
{
    auto start = chrono::steady_clock::now();
    for(unsigned int i=0; i < 1000000; i++)
    {
        Data *p = new Data;
        delete p;
    }
    auto end = chrono::steady_clock::now();
    cout << "Elapsed Time NoPool  = " <<
        chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;
}

char buf[4096];  //Simple Memory Pool
class SharedData
{
public:
    void *operator new(size_t uiSize)
    {
        return (void *)buf;
    }
    void  operator delete(void *p)
    {
    }
};

void SimpleMemPool()
{
    auto start = chrono::steady_clock::now();
    for(unsigned int i=0; i < 1000000; i++)
    {
        SharedData *p = new SharedData;
        delete p;
    }
    auto end = chrono::steady_clock::now();
    cout << "Elapsed Time MemPool  = " <<
        chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;
}

void deallocate(int * ptr)
{
    pool->deallocateMemory(ptr, "int");
}

void allocate()
{
    int * ptr = (int *)pool->AllocateMemory("int");
    *ptr = 4;
    sleep(2);
    deallocate(ptr);
}



int main(int argc, const char * argv[]) 
{
    cout << "Performance Test" << endl;
    cout << "Without Pool" << endl;
    NoPool();
    cout << "With Pool" << endl;
    SimpleMemPool();
    
    pool = new MemPool(4096);

/*  Checking here if mempool is thread safe or not 
    std::thread t1(allocate);
    t1.detach();

    std::thread t2(allocate);
    t2.detach();

    sleep(5);*/

    /* To check if it gives assert or not for condition [if Pointer is from subPool or Not]
    int * ptr = new int;
    pool->deallocateMemory(ptr, "int");*/

/* Creating 11 pointers and allocating them a memory from a already created subpool 
   but actual subpool can give a memory to only 10 pointers*/
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
    pool->printMemoryUsage_p();
    int* p7 = (int *)pool->AllocateMemory("int");
    *p7 = 4;
    int* p8 = (int *)pool->AllocateMemory("int");
    *p8 = 4;
    int* p9 = (int *)pool->AllocateMemory("int");
    *p9 = 4;
    int* p10 = (int *)pool->AllocateMemory("int");
    *p10 = 4;
    sleep(1);

    //int* p11 = (int *)pool->AllocateMemory("int");
    //*p11 = 4;

    pool->printMemoryUsage_p(); // 9
    sleep(1);

    

    /*std::cout<<"int is : "<<*p1<<std::endl;
  
    

    char * c = (char *)pool->AllocateMemory("char");
    *c = 'c';
     std::cout<<"char is : "<<*c<<std::endl;
    pool->deallocateMemory(c, "char");*/
    
    delete pool; 

    return 0;
}
