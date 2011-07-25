#ifndef MAP_H
#define MAP_H

#include "Page.h"
#include "PageTable.h"

extern char __KernelStart[];
extern char __KernelEnd[];

#define KERNEL_START (unsigned int)__KernelStart

#define PADDR_TO_VADDR(paddr) ((char*)(paddr) + KERNEL_START)
#define VADDR_TO_PADDR(vaddr) ((char*)(vaddr) - KERNEL_START)

struct AddressSpace {
	struct Page *pageTable;
	struct Page *L2Tables;
};

void MapPage(struct AddressSpace *space, void *vaddr, struct Page *page);

void MapInit();

extern unsigned KernelMap[PAGE_TABLE_SIZE];

#endif