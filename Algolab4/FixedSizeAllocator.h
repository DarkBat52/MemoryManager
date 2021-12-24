#pragma once

class FixedSizeAllocator{
public:
	FixedSizeAllocator();
	virtual ~FixedSizeAllocator();
	virtual void init();
	virtual void destroy();
	virtual void* alloc(size_t size);
	virtual void free(void* p);
	virtual void dumpStat() const;
	virtual void dumpBlocks() const;
};

