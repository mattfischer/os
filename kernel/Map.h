#ifndef MAP_H
#define MAP_H

#include "Page.h"
#include "PageTable.h"
#include "List.h"

extern char __KernelStart[];
extern char __KernelEnd[];

typedef unsigned int PAddr;

#define KERNEL_START (unsigned int)__KernelStart

#define PADDR_TO_VADDR(paddr) ((char*)(paddr) + KERNEL_START)
#define VADDR_TO_PADDR(vaddr) ((PAddr)(vaddr) - KERNEL_START)

struct Map {
	void *start;
	int size;
	struct List pages;
	struct ListEntry list;
};

struct AddressSpace {
	struct List table;
	PAddr tablePAddr;
	struct List L2Tables;
	struct List maps;
};

void MapPage(struct AddressSpace *space, void *vaddr, PAddr paddr);
void MapSectionLow(struct AddressSpace *space, void *vaddr, PAddr paddr);

void MapCreate(struct AddressSpace *space, void *vaddr, struct List pages);

void MapInit();
void MapInitLow();

struct AddressSpace KernelSpace;
#endif