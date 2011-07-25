#ifndef MAP_H
#define MAP_H

#include "Page.h"
#include "PageTable.h"

extern char __KernelStart[];
extern char __KernelEnd[];

#define KERNEL_START (unsigned int)__KernelStart

#define PADDR_TO_VADDR(paddr) ((char*)(paddr) + KERNEL_START)
#define VADDR_TO_PADDR(vaddr) ((char*)(vaddr) - KERNEL_START)

struct Map {
	void *start;
	int size;
	struct Page *pages;
	struct Map *next;
};

struct AddressSpace {
	struct Page *table;
	struct Page *L2Tables;
	struct Map *maps;
};

void MapPage(struct AddressSpace *space, void *vaddr, struct Page *page);
void MapPages(struct AddressSpace *space, void *vaddr, struct Page *pages);

void MapInit();

extern unsigned KernelMap[PAGE_TABLE_SIZE];

#endif