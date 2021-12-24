#include "CoalesceAllocator.h"
#include "Windows.h"

CoalesceAllocator::Region* CoalesceAllocator::allocateRegion()
{
	//allocate memory
	Region* r = static_cast<Region*>(VirtualAlloc(nullptr, kRegionSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	r->available_size = kRegionSize - sizeof(Region);

	//init free block
	Block* const block = reinterpret_cast<Block*> (r + 1);

	block->size = r->available_size - sizeof(Block);
	block->next = nullptr;
	r->head = block;
	r->available_size = block->size;

	return r;
}


