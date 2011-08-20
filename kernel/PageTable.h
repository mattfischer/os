#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "Slab.h"
#include "Page.h"
#include "List.h"

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
	List<Page> mL2Tables;

	static SlabAllocator<PageTable> sSlab;

	void allocL2Table(void *vaddr);
};

#endif