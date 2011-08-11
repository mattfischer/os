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

void AddressSpace_CopyFrom(struct AddressSpace *space, void *dest, void *source, int size);
void AddressSpace_CopyTo(struct AddressSpace *space, void *dest, void *source, int size);

void AddressSpace_Init();
void AddressSpace_InitLow();

extern struct AddressSpace KernelSpace;

#endif