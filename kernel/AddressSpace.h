#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#include "List.h"

extern char __KernelStart[];
extern char __KernelEnd[];

typedef unsigned int PAddr;

#define KERNEL_START (unsigned int)__KernelStart

#define PADDR_TO_VADDR(paddr) ((char*)(paddr) + KERNEL_START)
#define VADDR_TO_PADDR(vaddr) ((PAddr)(vaddr) - KERNEL_START)

struct Area {
	void *start;
	int size;
	struct List pages;
	struct ListEntry list;
};

struct AddressSpace {
	struct List table;
	PAddr tablePAddr;
	struct List L2Tables;
	struct List areas;
};

struct AddressSpace *AddressSpace_Create();

void AddressSpace_Map(struct AddressSpace *space, void *vaddr, struct List pages);

void AddressSpace_MapPage(struct AddressSpace *space, void *vaddr, PAddr paddr);
void AddressSpace_MapSectionLow(struct AddressSpace *space, void *vaddr, PAddr paddr);

void AddressSpace_Init();
void AddressSpace_InitLow();

extern struct AddressSpace KernelSpace;

#endif