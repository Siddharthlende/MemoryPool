//  MemPool.h
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <cassert>
#include <pthread.h>
#include <unistd.h>
#include <map>
#ifndef MemPool_h
#define MemPool_h

#define MAXLIMITOFPOOLSIZE 4096
class MemPool
{
private:
  typedef struct chunk
  {
    struct chunk * pPrev, *pNext;
  }chunk;
  typedef struct subPoolManager
  {
    void * memoryBlock;
    chunk *freeMemoryBlocks, *allocatedMemoryBlock;
    int sizeOfSubPool;
    int totalChunks, usedChunks, unusedChunks, totalMemory, unusedMemory, usedMemory, maxUsedChunks, sizeOfChunk;
  }subPoolManager;
  std::map<std::string, subPoolManager *>SubPool;
  pthread_mutex_t PoolLock;

public:
    MemPool(int totalSizeOfPool)
    {  
      if (pthread_mutex_init(&PoolLock, NULL) != 0) 
      {
        printf("\n mutex init has failed\n");
      }
      std::cout<<"Creating a Memory Pool..."<<std::endl;
      SubPool.insert(std::pair<std::string, subPoolManager *>("int", createSubPool(totalSizeOfPool, sizeof(int))));
      SubPool.insert(std::pair<std::string, subPoolManager *>("char", createSubPool(totalSizeOfPool, sizeof(char))));
      std::cout<<"Memory Pool is Created.."<<std::endl;
    }

    void printMemoryUsage(subPoolManager * ptr)
    {
      std::cout<<"########################################################"<<std::endl;
      std::cout<<"Total Memory Created : "<<ptr->totalMemory<<std::endl;
      std::cout<<"Total Used Memory : "<<ptr->usedMemory<<std::endl;
      std::cout<<"Total UnUsed Memory : "<<ptr->unusedMemory<<std::endl;
      std::cout<<"=========================================================="<<std::endl;
      std::cout<<"Total number of Chunks : "<<ptr->totalChunks<<std::endl;
      std::cout<<"Total Used Chunks : "<<ptr->usedChunks<<std::endl;
      std::cout<<"Total UnUsed Chunks : "<<ptr->unusedChunks<<std::endl;
      std::cout<<"Peak Usage of Chunks : "<<ptr->maxUsedChunks<<std::endl;
      std::cout<<"########################################################"<<std::endl;
    }

    void printMemoryUsage_p()
    {
      for(auto it : SubPool)
      {
        if(!it.first.compare("int"))
        {
        std::cout<<std::endl;
        std::cout<<std::endl;
        std::cout<<"Data Type is : "<<it.first<<std::endl;
        printMemoryUsage(it.second);
        std::cout<<std::endl;
        }
      }
    }

    ~MemPool()
    {
      for(auto it : SubPool)
      {
        std::cout<<std::endl;
        std::cout<<std::endl;
        std::cout<<"Data Type is : "<<it.first<<std::endl;
        printMemoryUsage(it.second);
        std::cout<<std::endl;
        free(it.second->memoryBlock);
      }
      std::cout<<"Memory Pool is destroyed."<<std::endl;
    }
    
    subPoolManager * createSubPool(int& sizeOfPool, int sizeOfChunk)
    {
      void * memBlock = nullptr;
      //assert(sizeOfPool > MAXLIMITOFPOOLSIZE); 

      if(sizeOfPool > MAXLIMITOFPOOLSIZE)
      {
        std::cerr <<"Give Memory size is More than pre defined Limit given : "<<MAXLIMITOFPOOLSIZE<<std::endl;
        return nullptr;
      }      
      int totalUnits = sizeOfPool/sizeOfChunk;
      subPoolManager * head = (subPoolManager *)malloc(sizeof(subPoolManager)+2);
      head->allocatedMemoryBlock = nullptr;
      head->freeMemoryBlocks = nullptr;
      head->memoryBlock = nullptr;
      head->sizeOfSubPool = sizeOfPool+(totalUnits*sizeof(chunk));
      head->totalChunks = totalUnits;
      head->totalMemory = sizeOfPool;
      head->unusedChunks = totalUnits;
      head->usedChunks = 0;
      head->usedMemory = 0;
      head->maxUsedChunks = 0;
      head->unusedMemory = sizeOfPool;
      head->sizeOfChunk = sizeOfChunk;
      memBlock = malloc(head->sizeOfSubPool);
      if (memBlock == nullptr)
      {
        std::cout<<"Memory Not Allocated..!"<<std::endl;
        exit(0);
      }
      head->memoryBlock = memBlock;
      for(int i=0; i<totalUnits; i++)
      {
        chunk * currentChunk_p = (chunk *)( (char *)memBlock + i*(sizeOfChunk+sizeof(chunk)));          
        currentChunk_p->pPrev = nullptr;
        currentChunk_p->pNext = head->freeMemoryBlocks;    //Insert the new unit at head.
        
        if(nullptr != head->freeMemoryBlocks)
        {
            head->freeMemoryBlocks->pPrev = currentChunk_p;
        }
        head->freeMemoryBlocks = currentChunk_p;
      }
      return head;
    }

    void * AllocateMemory(std::string dataType)
    {
      pthread_mutex_trylock(&PoolLock);
      for(auto it : SubPool)
      {
        if(!dataType.compare(it.first))
         {
            return AllocateMemoryToChunk(it.second);
         }
      }
      pthread_mutex_unlock(&PoolLock);
      return nullptr;
    }
    
    void * AllocateMemoryToChunk(subPoolManager * ptr)
    {
      //To verify that mempool should not allocate memory more than it can hold.
      //assert(ptr->totalChunks > ptr->usedChunks);
      if(ptr->totalChunks <= ptr->usedChunks)
      {
        std::cout<<"Memory Pool is Busy..!"<<std::endl;
        return nullptr;
      }
      chunk * currentUnit = ptr->freeMemoryBlocks;
      ptr->freeMemoryBlocks = currentUnit->pNext;
      if (nullptr != ptr->freeMemoryBlocks)
      {
        ptr->freeMemoryBlocks->pPrev = nullptr;
      }

      currentUnit->pNext = ptr->allocatedMemoryBlock;
      if (nullptr != ptr->allocatedMemoryBlock)
      {
        ptr->allocatedMemoryBlock->pPrev = currentUnit;
      }
      std::cout<<"Memory Is Allocaed"<<std::endl;
      ptr->usedChunks++;
      ptr->unusedChunks--;
      ptr->usedMemory = ptr->usedMemory + ptr->sizeOfChunk;
      ptr->unusedMemory = ptr->unusedMemory - ptr->sizeOfChunk;
      ptr->maxUsedChunks++;
      return (void *)((char *)currentUnit + sizeof(chunk));
    }

    bool deallocateMemory(void *ptr, std::string dataType)
    {
      // This section is critical section so it should be thread safe.
      pthread_mutex_trylock(&PoolLock);
      for(auto it : SubPool)
      {
        if (!dataType.compare(it.first))
        {
          return FreeMemory(it.second, ptr);
        }
      }
      pthread_mutex_unlock(&PoolLock);
    }

    bool FreeMemory(subPoolManager * head, void *ptr)
    {
      //assert(head->memoryBlock<ptr && ptr<(void *)((char *)head->memoryBlock + head->sizeOfSubPool));
      if(head->memoryBlock<ptr && ptr<(void *)((char *)head->memoryBlock + head->sizeOfSubPool))
      {
        chunk *pCurUnit = (chunk *)((char *)ptr - sizeof(chunk));  
        chunk* pPrev = pCurUnit->pPrev;
        chunk* pNext = pCurUnit->pNext;
        if ( pPrev == nullptr)
        {
          head->allocatedMemoryBlock = pCurUnit->pNext;
        }
        else
        {
          pPrev->pNext = pNext;
        }
        if (nullptr != pNext)
        {
          pNext->pPrev = pPrev;
        }
        pCurUnit->pNext = head->freeMemoryBlocks;
        if (nullptr != head->freeMemoryBlocks)
        {
          head->freeMemoryBlocks->pPrev = pCurUnit;
        }
        head->freeMemoryBlocks = pCurUnit;
        head->usedChunks--;
        head->unusedChunks++;
        head->usedMemory = head->usedMemory - head->sizeOfChunk;
        head->unusedMemory = head->unusedMemory + head->sizeOfChunk;
        std::cout<<"Memory is Freed"<<std::endl;
        return true;
      }
      else
      {
        std::cout<<"This Memory does not belongs to our Created Memory Pool"<<std::endl;
        return false;
      }
    }
};

#endif /* MemPool_h */
