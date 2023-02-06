#include "MemoryPool.h"
#include <string.h>

int main()
{
    MempoolConfig config;
    SubPoolDescriptor var;
       var.amount = 1024;
       var.size = 4;
       config.push_back(var);

    Mempool * pool = new Mempool(config, 4);

    int * array = (int *)pool->alloc(4);
    *array = 40;
    pool->free(array);
    delete pool;
}