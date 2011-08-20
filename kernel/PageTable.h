#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "Slab.h"
#include "Page.h"

typedef unsigned int PAddr;

#define PADDR_TO_VADDR(paddr) ((char*)(paddr) + KERNEL_START)
#define VADDR_TO_PADDR(vaddr) ((PAddr)(vaddr) - KERNEL_START)

extern char __KernelStart[];
extern char __KernelEnd[];

#define KERNEL_START (unsigned int)__KernelStart

class PageTable {
public:
	enum Permission {
		PermissionRO,
		PermissionRW,
		PermissionRWPriv
	};

	PageTable();
	PageTable(struct Page *pages);

	static void init();
	static void initLow();

	PAddr tablePAddr() { return mTablePAddr; }

	void mapPage(void *vaddr, PAddr paddr, Permission permission);
	void mapSectionLow(void *vaddr, PAddr paddr, Permission permission);

	PAddr translateVAddr(void *vaddr);

	void *operator new(size_t size) { return sSlab.allocate(); }

	static PageTable *Kernel;

private:
	struct Page *mPages;
	PAddr mTablePAddr;
	LIST(struct Page) mL2Tables;

	static SlabAllocator<PageTable> sSlab;

	void allocL2Table(void *vaddr);
};

#endif