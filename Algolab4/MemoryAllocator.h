#pragma once
#include <Windows.h>
#include "CoalesceAllocator.h"
#include "FixedSizeAllocator.h"
#ifdef _DEBUG
#include <map>
#endif // _DEBUG

class MemoryAllocator {
public:
	MemoryAllocator() = default;
	virtual ~MemoryAllocator();
	virtual void init();
	virtual void destroy();
	virtual void* alloc(size_t size);
	virtual void free(void* p);
#ifdef _DEBUG
	virtual void dumpStat() const;
	virtual void dumpBlocks() const;

	std::map<size_t, size_t> heap_allocated{};

#endif // _DEBUG

	enum class AllocatorType {
		CA, FSA, HA
	};

	struct Data {
		AllocatorType type;
		void* ptr;
	};

private:
	CoalesceAllocator coalesceAllocator;
	FixedSizeAllocator fsa;

#ifdef _DEBUG
	bool is_destroyed;
#endif // _DEBUG

};


