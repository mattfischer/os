#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#include "List.h"
#include "PageTable.h"
#include "MemArea.h"

struct Mapping {
	void *vaddr;
	unsigned int offset;
	unsigned int size;
	struct MemArea *area;
	struct ListEntry list;
};

struct AddressSpace {
	struct PageTable *pageTable;
	LIST(struct Mapping) mappings;
};

struct AddressSpace *AddressSpace_Create();

void AddressSpace_Map(struct AddressSpace *space, struct MemArea *area, void *vaddr, unsigned int offset, unsigned int size);
void AddressSpace_Memcpy(struct AddressSpace *destSpace, void *dest, struct AddressSpace *srcSpace, void *src, int size);

void AddressSpace_Init();

extern struct AddressSpace *KernelSpace;

#endif