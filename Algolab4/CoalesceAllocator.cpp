#include "CoalesceAllocator.h"
#include "Windows.h"
#include <crtdbg.h>
#include <iostream>

CoalesceAllocator::Region* CoalesceAllocator::allocateRegion()
{
	//allocate memory
	Region* r = static_cast<Region*>(VirtualAlloc(nullptr, kRegionSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	_ASSERT(r != nullptr);
	r->available_size = kRegionSize - sizeof(Region);

	//init free block
	Block* const block = reinterpret_cast<Block*> (r + 1);

	block->size = r->available_size - sizeof(Block);
	block->next = nullptr;
	block->previous = nullptr;
	r->head = block;
	block->region = r;
	r->available_size = block->size;
	block->next_free = nullptr;
	if (free_list == nullptr) {
		free_list = block;
		block->previous_free = nullptr;
	}
	else {
		Block* fl = free_list;
		while (fl->next_free) {
			fl++;
		}
		fl->next_free = block;
		block->previous_free = fl;
	}
	
	return r;
}

bool CoalesceAllocator::trySplitBlock(Block* block, int size)
{
	if (block->size >= sizeof(Block) + size /*+ 512*/) {
		Block* next_block = reinterpret_cast<Block*>(reinterpret_cast<INT8*>(block + 1) + size);
		next_block->next_free = block->next_free;
		block->next_free = next_block;
		block->free = false;
		next_block->size = block->size - size - sizeof(Block);
		block->size = size;
		block->next = next_block;
		next_block->previous = block;
		next_block->free = true;
		next_block->previous_free = block->previous_free;
		next_block->next = nullptr;
		if (block->previous_free == nullptr) {
			free_list = next_block;
		}
		if (block->previous_free != nullptr) {
			block->previous_free->next_free = next_block;
		}
		if (block->previous != nullptr && block->previous->free) {
			block->previous->next_free = next_block;
		}
		return true;
	}
	else {
		// just alloc this block
		block->free = false;
		if (block == free_list) {
			free_list = block->next_free;
		}
		else if (block->previous_free != nullptr) {
			block->previous_free->next_free = block->next_free;
		}
		if (block->next_free != nullptr)
		{
			block->next_free->previous_free = block->previous_free;
		}
	}
	return false;
}

CoalesceAllocator::~CoalesceAllocator()
{
	destroy();
	_ASSERT(is_destroyed);
}

void CoalesceAllocator::init()
{
	RegionHead = allocateRegion();
}

void CoalesceAllocator::destroy()
{
	_ASSERT(RegionHead != nullptr);
#ifdef _DEBUG
	bool no_leak = true;
	Region* region = RegionHead;
	for (Region* region = RegionHead; region;region = region->next) {
		for (Block* block = region->head; block; block = block->next) {
			if (!block->free) {
				no_leak = false;
			}
		}
	}
	_ASSERT(no_leak);
#endif // _DEBUG

	Region* next = RegionHead->next;
	while (next) {
		Region* next = RegionHead->next;
		VirtualFree(RegionHead, 0, MEM_RELEASE);
	}
	is_destroyed = true;
}

void* CoalesceAllocator::alloc(size_t size)
{	
	size = size % 8? size + 8 - size%8: size;
	
	Region* rh = RegionHead;
	Block* fb;
	bool found = false;

	fb = free_list;
	while (fb) {
		if (fb->size >= size) {
			found = true;
			break;
		}
		if (fb->next_free == nullptr) {
			break;
		}
		fb = fb->next_free;
	}

	//allocate new region if no space found
	if (!found) {
		rh->next = allocateRegion();
		fb->next_free = rh->next->head;
		fb->next_free->previous_free = fb;
	}

	//initialize block on located pointer
	trySplitBlock(fb, size);

	return fb + 1;
}

void CoalesceAllocator::free(void* p)
{
	_ASSERTE(p != nullptr && "p should not be null");

	Block* const block = reinterpret_cast<Block*>(p) - 1;

#ifdef _DEBUG
	bool found = false;
	;
	for (Region* r = RegionHead; r; r = r->next) {
		for (Block* b = RegionHead->head; b; b = b->next){
			if (block == b) {
				found = true;
			}
		}
	}
	
	_ASSERT(found);

#endif // _DEBUG

	block->free = true;
	Block* res = block;

	// coalesce with previous block
	if (block->previous != nullptr && block->previous->free) {
		res = block->previous;		
		res->next = block->next;
		res->size += block->size + sizeof(Block);
	} 

	if (block->next != nullptr && block->next->free) {
		res->next_free = block->next->next_free;	
		res->size += block->next->size + sizeof(Block);
		res->next = block->next->next;
		_ASSERT(block);
		if (block->previous != nullptr && !block->previous->free) {
			res->previous_free = block->previous_free;
		}	
	}

	if (res->previous_free == nullptr) {
		Block* nf = free_list;
		free_list = res;
		free_list->next_free = nf;
	}

}

#ifdef _DEBUG
void CoalesceAllocator::dumpStat() const
{
	size_t free_count = 0;
	size_t allocated_count = 0;
	size_t region_count = 0;

	for (Region* r = RegionHead; r; r = r->next) {
		for (Block* b = RegionHead->head; b; b = b->next) {
			if (b->free) {
				++free_count;
			}
			else {
				++allocated_count;
			}
		}
		++region_count;
	}

	std::cout << "Coalesce DumpStat:" << std::endl;
	std::cout << '\t' << "Free blocks: \t" << free_count << std::endl;
	std::cout << '\t' << "Allocated blocks: \t" << allocated_count << std::endl;

	std::cout << '\t' << "Allocated regions: \t" << region_count << std::endl << std::endl;
	for (Region* r = RegionHead; r; r = r->next) {
		std::cout  << '\t' << r << '\t' << kRegionSize << std::endl;
	}
}

void CoalesceAllocator::dumpBlocks() const
{
	size_t count = 0;

	std::cout << "Coalesce DumpBlocks: \t" << std::endl;

	for (Region* r = RegionHead; r; r = r->next) {
		for (Block* b = RegionHead->head; b; b = b->next) {
			//if (!b->free) {
				std::cout << "\t" << b << "\t" << b->size << "\t\t" << (b->free ? "free" : "allocated") << std::endl;
				++count;
			//}
			
		}
	}

	std::cout << "Total: \t" << count << " blocks." << std::endl;

}


#endif // _DEBUG