#pragma once
#include <Windows.h>

class MemoryAllocator {
public:
	MemoryAllocator();
	virtual ~MemoryAllocator();
	virtual void init();
	virtual void destroy();
	virtual void* alloc(size_t size);
	virtual void free(void* p);
	virtual void dumpStat() const;
	virtual void dumpBlocks() const;
};


