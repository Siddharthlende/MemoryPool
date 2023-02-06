  #include <iostream>
  #include <string>
  #include <vector>
  #include <map>
  #include <stdlib.h>

  constexpr std::size_t total_limit = 2048*1024;
  typedef struct SubPoolDescriptor
  {
    int32_t amount;
    std::size_t size;
  }SubPoolDescriptor;

  typedef std::vector<SubPoolDescriptor> MempoolConfig;

  class Mempool
  {
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
    std::map<int, subPoolManager *>SubPool;
    pthread_mutex_t PoolLock;

    public:
    Mempool(MempoolConfig config, std::size_t alignment)
    {
      for(auto it : config)
      {
          SubPool.insert(std::pair<int, subPoolManager *>(alignment, createSubPool(it, alignment)));
      }
    }

    subPoolManager * createSubPool(SubPoolDescriptor MemConfig, std::size_t alignment)
    {
        void * memBlock = nullptr;
        //assert(MemConfig.amount > total_limit); 

        if(MemConfig.amount > total_limit)
        {
          std::cerr <<"Give Memory size is More than pre defined Limit given : "<<total_limit<<std::endl;
          return nullptr;
        }      
        int totalUnits = MemConfig.amount/alignment;
        subPoolManager * head = (subPoolManager *)malloc(sizeof(subPoolManager)+2);
        head->allocatedMemoryBlock = nullptr;
        head->freeMemoryBlocks = nullptr;
        head->memoryBlock = nullptr;
        head->sizeOfSubPool = MemConfig.amount+(totalUnits*sizeof(chunk));
        head->totalChunks = totalUnits;
        head->totalMemory = MemConfig.amount;
        head->unusedChunks = totalUnits;
        head->usedChunks = 0;
        head->usedMemory = 0;
        head->maxUsedChunks = 0;
        head->unusedMemory = MemConfig.amount;
        head->sizeOfChunk = MemConfig.size;
        memBlock = malloc(head->sizeOfSubPool);
        if (memBlock == nullptr)
        {
          std::cout<<"Memory Not Allocated..!"<<std::endl;
          exit(0);
        }
        head->memoryBlock = memBlock;
        for(int i=0; i<totalUnits; i++)
        {
          chunk * currentChunk_p = (chunk *)( (char *)memBlock + i*(MemConfig.size+sizeof(chunk)));          
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

    void * alloc(std::size_t size)
    {
      pthread_mutex_trylock(&PoolLock);
        for(auto it : SubPool)
        {
          return AllocateMemoryToChunk(it.second, size);
        }
        pthread_mutex_unlock(&PoolLock);
        return nullptr;
    }

    void * AllocateMemoryToChunk(subPoolManager * ptr, int size)
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

      bool free(void * ptr)
      {
          bool result=false;
          auto it = SubPool.begin();
        //for(auto it : SubPool)
        for (auto it = SubPool.begin(); it!=SubPool.end(); it++)
        {
            //if(it.second->memoryBlock<ptr && ptr<(void *)((char *)it.second->memoryBlock + it.second->sizeOfSubPool))
            {
              result = freeMemory(it->second, ptr);
            }
        }
        //std::cout<<"Memory does not belongs to our MemoryPool"<<std::endl;
        return result;
      }

      bool freeMemory(subPoolManager * head, void * ptr)
      {
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
        return false;
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

      void printMemoryUsage_p(std::size_t size)
      {
        for(auto it : SubPool)
        {
          if(it.first == size || it.first > size)
          {
          std::cout<<std::endl;
          std::cout<<std::endl;
          std::cout<<"Data Type is : "<<it.first<<std::endl;
          printMemoryUsage(it.second);
          std::cout<<std::endl;
          }
        }
      }

      ~Mempool()
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
    
  };
