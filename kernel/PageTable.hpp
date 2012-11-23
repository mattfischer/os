#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "Slab.hpp"
#include "Page.hpp"
#include "List.hpp"

#include <stddef.h>

/*!
 * \brief Represents a page table.
 */
class PageTable {
public:
	enum Permission {
		PermissionNone,
		PermissionRO,
		PermissionRW,
		PermissionRWPriv
	};

	PageTable(PageTable *copy);
	PageTable(Page *pages);
	~PageTable();

	static void init();

	/*!
	 * \brief Returns physical address of page table
	 * \return Physical address
	 */
	PAddr tablePAddr() { return mTablePAddr; }

	void mapPage(void *vaddr, PAddr paddr, Permission permission);
	void mapSection(void *vaddr, PAddr paddr, Permission permission);

	PAddr translateVAddr(void *vaddr);

	//! Allocator
	void *operator new(size_t size) { return sSlab.allocate(); }
	void operator delete(void *p) { return sSlab.free((PageTable*)p); }

	static const int SectionSize = (1024 * 1024);

private:
	Page *mPages;
	PAddr mTablePAddr;
	List<Page> mL2Tables;

	static Slab<PageTable> sSlab;

	void allocL2Table(void *vaddr);
};

#endif
