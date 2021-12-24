#pragma once

class CoalesceAllocator {
private:
	constexpr static size_t kRegionSize = 10* (1 << 20);

public:
	CoalesceAllocator();
	virtual ~CoalesceAllocator();
	virtual void init();
	virtual void destroy();
	virtual void* alloc(size_t size);
	virtual void free(void* p);
	virtual void dumpStat() const;
	virtual void dumpBlocks() const;


	//allocate region
private:  

	//struct block
	struct Block {
		//header 
		// size that was/can be allocated (doesn't include header of the block)
		size_t size;
		Block* next;
		Block* previous;
	};

	struct Region {
		Block* head;
		size_t available_size;
	};

	Region* allocateRegion();

	


};

