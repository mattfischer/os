#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#include "List.h"
#include "PageTable.h"
#include "MemArea.h"
#include "Slab.h"

struct Mapping : public ListEntry {
	void *vaddr;
	unsigned int offset;
	unsigned int size;
	MemArea *area;
};

class AddressSpace {
public:
	AddressSpace(PageTable *pageTable = NULL);

	static void init();

	struct PageTable *pageTable() { return mPageTable; }

	void map(MemArea *area, void *vaddr, unsigned int offset, unsigned int size);
	static void memcpy(AddressSpace *destSpace, void *dest, AddressSpace *srcSpace, void *src, int size);

	void *operator new(size_t size) { return sSlab.allocate(); }

	static AddressSpace *Kernel;

private:
	PageTable *mPageTable;
	List<struct Mapping> mMappings;

	static SlabAllocator<AddressSpace> sSlab;
};
#endif