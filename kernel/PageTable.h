#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "Page.h"

typedef unsigned int PAddr;

#define PADDR_TO_VADDR(paddr) ((char*)(paddr) + KERNEL_START)
#define VADDR_TO_PADDR(vaddr) ((PAddr)(vaddr) - KERNEL_START)

extern char __KernelStart[];
extern char __KernelEnd[];

#define KERNEL_START (unsigned int)__KernelStart

struct PageTable {
	struct Page *table;
	PAddr tablePAddr;
	LIST(struct Page) L2Tables;
};

struct PageTable *PageTable_Create();

enum PageTablePermission {
	PageTablePermissionRO,
	PageTablePermissionRW,
	PageTablePermissionRWPriv
};

void PageTable_MapPage(struct PageTable *pageTable, void *vaddr, PAddr paddr, enum PageTablePermission permission);
void PageTable_MapSectionLow(struct PageTable *pageTable, void *vaddr, PAddr paddr, enum PageTablePermission permission);

PAddr PageTable_TranslateVAddr(struct PageTable *pageTable, void *vaddr);

void PageTable_Init();
void PageTable_InitLow();

extern struct PageTable KernelPageTable;
#endif