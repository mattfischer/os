#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "Slab.h"
#include "Page.h"
#include "List.h"

#define PAGE_TABLE_SECTION_SIZE (1024 * 1024)

class PageTable {
public:
	enum Permission {
		PermissionRO,
		PermissionRW,
		PermissionRWPriv
	};

	PageTable(PageTable *copy);
	PageTable(Page *pages);

	static void init();

	PAddr tablePAddr() { return mTablePAddr; }

	void mapPage(void *vaddr, PAddr paddr, Permission permission);

	PAddr translateVAddr(void *vaddr);

	void *operator new(size_t size) { return sSlab.allocate(); }

	static void initLow();
	static void mapSectionLow(Page *pageTable, void *vaddr, PAddr paddr, Permission permission);

private:
	struct Page *mPages;
	PAddr mTablePAddr;
	List<Page> mL2Tables;

	static SlabAllocator<PageTable> sSlab;

	void allocL2Table(void *vaddr);
};

#endif