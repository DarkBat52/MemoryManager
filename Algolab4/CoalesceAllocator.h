#pragma once

class CoalesceAllocator {
private:
	constexpr static size_t kRegionSize = 10* (1 << 20);

public:
	CoalesceAllocator() = default;
	virtual ~CoalesceAllocator();
	virtual void init();
	virtual void destroy();
	virtual void* alloc(size_t size);
	virtual void free(void* p);

#ifdef _DEBUG
	virtual void dumpStat() const;
	virtual void dumpBlocks() const;
#endif // _DEBUG

	bool ptrIsAllocated(void* p);


	//allocate region
private:  
	struct Region;

	//struct block
	struct Block {
		//header 
		// size that was/can be allocated (doesn't include header of the block)
		size_t size;
		Block* next = nullptr;
		Block* previous = nullptr;
		Block* next_free = nullptr;
		Block* previous_free = nullptr;
		Region* region = nullptr;
		bool free = true;
	};

	struct Region {
		Block* head = nullptr;
		size_t available_size;
		Region* next = nullptr;
		
	};

	Region* allocateRegion();

	bool trySplitBlock(Block* block, int size);


	// the very end
	Region* RegionHead = nullptr;
	Block* free_list = nullptr;
	
#ifdef _DEBUG
	bool is_destroyed = false;
#endif // _DEBUG


};

