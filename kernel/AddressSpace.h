#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#include "List.h"
#include "PageTable.h"

struct Area {
	void *start;
	int size;
	LIST(struct Page) pages;
	struct ListEntry list;
};

struct AddressSpace {
	struct PageTable *pageTable;
	LIST(struct Area) areas;
};

struct AddressSpace *AddressSpace_Create();

void AddressSpace_Map(struct AddressSpace *space, void *vaddr, LIST(struct Page) pages);

void AddressSpace_CopyFrom(struct AddressSpace *space, void *dest, void *source, int size);
void AddressSpace_CopyTo(struct AddressSpace *space, void *dest, void *source, int size);

void AddressSpace_Init();
void AddressSpace_InitLow();

extern struct AddressSpace KernelSpace;

#endif