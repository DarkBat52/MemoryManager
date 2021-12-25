// Algolab4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "CoalesceAllocator.h"
#include "MemoryAllocator.h"

int main()
{
    MemoryAllocator allocator;
    allocator.init();
    int* pi = (int*)allocator.alloc(sizeof(int));
    allocator.dumpStat();
    allocator.dumpBlocks();
    double* pd = (double*)allocator.alloc(sizeof(double));
    allocator.dumpStat();
    allocator.dumpBlocks();
    int* pa = (int*)allocator.alloc(10 * sizeof(int));
    allocator.dumpStat();
    allocator.dumpBlocks();
    _ASSERT(pa != nullptr);
    allocator.dumpStat();
    allocator.dumpBlocks();
    allocator.free(pi);
    int* pi2 = (int*)allocator.alloc(sizeof(int));
    allocator.dumpStat();
    allocator.dumpBlocks();
    allocator.free(pi2);
    allocator.free(pa);
    allocator.dumpBlocks();
    allocator.free(pd);
    allocator.dumpBlocks();
    allocator.destroy();
}

