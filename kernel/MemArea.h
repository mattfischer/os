#ifndef MEM_AREA_H
#define MEM_AREA_H

#include "List.h"
#include "Page.h"
#include "PageTable.h"
#include "Slab.h"

class MemArea {
public:
	MemArea(int size);

	int size() { return mSize; }

	virtual void map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size) {}

private:
	int mSize;
};

class MemAreaPages : public MemArea {
public:
	MemAreaPages(int size);

	virtual void map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size);

	List2<Page, &Page::list>& pages() { return mPages; }

	void *operator new(size_t size) { return sSlab.allocate(); }

private:
	List2<Page, &Page::list> mPages;

	static SlabAllocator<MemAreaPages> sSlab;
};

class MemAreaPhys : public MemArea {
public:
	MemAreaPhys(int size, PAddr paddr);

	virtual void map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size);

	void *operator new(size_t size) { return sSlab.allocate(); }

private:
	PAddr mPAddr;

	static SlabAllocator<MemAreaPhys> sSlab;
};

#endif