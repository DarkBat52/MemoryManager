#include "MemoryAllocator.h"
#include <crtdbg.h>
#include <iostream>
constexpr static size_t kRegionSize = 10 * (1 << 20);

MemoryAllocator::~MemoryAllocator()
{
	destroy();
}

void MemoryAllocator::init()
{
	coalesceAllocator.init();
	fsa.init();
}

void MemoryAllocator::destroy()
{
	coalesceAllocator.~CoalesceAllocator();
	fsa.~FixedSizeAllocator();
}

void* MemoryAllocator::alloc(size_t size)
{	
	void* p = nullptr;
	AllocatorType* at;
	if (size <= 512 - sizeof(AllocatorType)) {
		p = fsa.alloc(size + sizeof(AllocatorType));
		at = reinterpret_cast<AllocatorType*>(p);
		*at = AllocatorType::FSA;
	} else if (size <= kRegionSize - sizeof(AllocatorType)) {
		p = coalesceAllocator.alloc(size + sizeof(AllocatorType));
		at = reinterpret_cast<AllocatorType*>(p);
		*at = AllocatorType::CA;
	} else {
		// heap alloc
		p = HeapAlloc(GetProcessHeap(), 0, size + sizeof(AllocatorType));
		_ASSERT(p && "HeapAlloc returned nullptr.\n");
		at = reinterpret_cast<AllocatorType*>(p);
		*at = AllocatorType::HA;
#ifdef _DEBUG
		heap_allocated.emplace(reinterpret_cast<size_t>(at), size + sizeof(AllocatorType));
#endif // _DEBUG
	}
	p = reinterpret_cast<void*>(at + 1);
	
	return p;
}

void MemoryAllocator::free(void* p)
{	
	AllocatorType* at = reinterpret_cast<AllocatorType*>(p) - 1;
	if (*at == AllocatorType::CA) {
		coalesceAllocator.free(at);
	}
	else if (*at == AllocatorType::FSA) {
		fsa.free(at);
	}
	else if (*at == AllocatorType::HA) {
		HeapFree(GetProcessHeap(), 0, at);
#ifdef _DEBUG
		heap_allocated.erase(reinterpret_cast<size_t>(at));
#endif // _DEBUG
	}
}

#ifdef _DEBUG

void MemoryAllocator::dumpStat() const
{
	coalesceAllocator.dumpStat();
	std::cout << std::endl;
	fsa.dumpStat();
	std::cout << std::endl;

	//Heap stats
	std::cout << "Heap DumpStats" << std::endl;
	size_t total_size = 0;
	size_t count = 0;
	for (auto pair : heap_allocated) {
		total_size += pair.first;
		++count;
	}
	std::cout << "Allocated blocks: " << count << std::endl;
	std::cout << "Total bytes allocated: " << total_size << std::endl << std::endl;
}

void MemoryAllocator::dumpBlocks() const
{
	coalesceAllocator.dumpBlocks();
	std::cout << std::endl;
	fsa.dumpBlocks();
	std::cout << std::endl;

	//Heap stats
	std::cout << "Heap DumpBlocks" << std::endl;
	for (auto pair : heap_allocated) {
		std::cout << '\t' << pair.first << '\t' << pair.second << std::endl;
	}
	std::cout << '\t' << "Total count" << heap_allocated.size() << std::endl <<std::endl << std::endl;
}

#endif // _DEBUG