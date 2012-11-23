#include "PageTable.hpp"

#include "Pte.hpp"

#include <string.h>

//! Slab allocator for page tables
struct Slab<PageTable> PageTable::sSlab;

/*!
 * \brief Construct a page table by copying another table's contents
 * \param copy Page table to copy
 */
PageTable::PageTable(PageTable *copy)
{
	// Allocate contiguous pages to hold page table contents
	mPages = Page::allocContig(4, 4);
	mTablePAddr = mPages->paddr();

	unsigned *base = (unsigned*)PADDR_TO_VADDR(mTablePAddr);
	unsigned *copyBase = (unsigned*)PADDR_TO_VADDR(copy->mTablePAddr);

	// Copy contents into new page table
	memcpy(base, copyBase, PAGE_TABLE_SIZE * sizeof(unsigned));
}

/*!
 * \brief Wrap a page table structure around an existing set of pages
 * \param pages Pointer to first page of page table
 */
PageTable::PageTable(Page *pages)
{
	mPages = pages;
	mTablePAddr = mPages->paddr();
}

PageTable::~PageTable()
{
	for(int i=0; i<4; i++) {
		Page *page = Page::fromNumber(mPages->number() + i);
		page->free();
	}

	for(Page *page = mL2Tables.head(); page != 0; page = mL2Tables.next(page)) {
		page->free();
	}
}

// Allocate a second-level page table
void PageTable::allocL2Table(void *vaddr)
{
	unsigned *table = (unsigned*)PADDR_TO_VADDR(mTablePAddr);
	int idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;

	// Search through the list of L2 tables and attempt to find an unused one
	for(Page *L2Page = mL2Tables.head(); L2Page != 0; L2Page = mL2Tables.next(L2Page)) {
		unsigned *L2Table = (unsigned*)L2Page->vaddr();
		// Each 4kb page contains 4 1kb second-level tables
		for(int i=0; i<4; i++) {
			int l2idx = i*PAGE_L2_TABLE_SIZE;
			unsigned l2pte = L2Table[l2idx];

			// If the page table is marked as disabled, all other bits are unused, so this
			// code uses the top bit as a marker that the table is available for use
			if((l2pte & PTE_L2_TYPE_MASK) != PTE_L2_TYPE_DISABLED || (l2pte & 0x80000000) == 0) {
				continue;
			}

			// Found a page--zero it out
			for(int j=0; j<PAGE_L2_TABLE_SIZE; j++) {
				L2Table[l2idx + j] = 0;
			}

			// Link the new table into the main page table
			table[idx] = VADDR_TO_PADDR(L2Table + l2idx) | PTE_SECTION_AP_READ_WRITE | PTE_TYPE_COARSE;
			return;
		}
	}

	// No free tables found--allocate a new page and add it to the list
	Page *L2Page = Page::alloc();
	unsigned *L2Table = (unsigned*)L2Page->vaddr();
	mL2Tables.addTail(L2Page);

	// Zero out everything, and mark the other 3 pages as available
	memset(L2Table, 0, PAGE_L2_TABLE_SIZE * sizeof(unsigned));
	for(int i=1; i<4; i++) {
		L2Table[i*PAGE_L2_TABLE_SIZE] = 0x80000000;
	}

	// Link the new table into the main page table
	table[idx] = VADDR_TO_PADDR(L2Table) | PTE_TYPE_COARSE;
}

/*!
 * \brief Map a page of physical memory into the page table
 * \param vaddr Virtual address to map to
 * \param paddr Physical address to map
 * \param permission Permission flags
 */
void PageTable::mapPage(void *vaddr, PAddr paddr, Permission permission)
{
	unsigned *table = (unsigned*)PADDR_TO_VADDR(mTablePAddr);
	int idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	unsigned pte = table[idx];

	// If this section does not already have a second-level table,
	// then allocate one
	if((pte & PTE_TYPE_MASK) == PTE_TYPE_SECTION ||
	   (pte & PTE_TYPE_MASK) == PTE_TYPE_DISABLED) {
	   allocL2Table(vaddr);
	}

	pte = table[idx];

	// Set up permission bits
	unsigned int perm;
	switch(permission) {
		case PermissionNone: perm = PTE_L2_AP_ALL_NONE; break;
		case PermissionRO: perm = PTE_L2_AP_ALL_READ_ONLY; break;
		case PermissionRW: perm = PTE_L2_AP_ALL_READ_WRITE; break;
		case PermissionRWPriv: perm = PTE_L2_AP_ALL_READ_WRITE_PRIV; break;
	}

	if((pte & PTE_TYPE_MASK) == PTE_TYPE_COARSE) {
		// Set the appropriate entry of the second-level page table
		unsigned *L2Table = (unsigned*)PADDR_TO_VADDR(pte & PTE_COARSE_BASE_MASK);
		int l2idx = ((unsigned)vaddr & (~PAGE_TABLE_SECTION_MASK)) >> PAGE_SHIFT;
		L2Table[l2idx] = (paddr & PTE_COARSE_BASE_MASK) | perm | PTE_L2_TYPE_SMALL;
	}
}

/*!
 * \brief Map a section-sized range of virtual address space
 * \param vaddr Virtual address to map
 * \param paddr Physical address to map
 * \param permission Permission flags
 */
void PageTable::mapSection(void *vaddr, PAddr paddr, Permission permission)
{
	unsigned int perm;

	// Set up permission bits
	switch(permission) {
		case PermissionNone: perm = PTE_L2_AP_ALL_NONE; break;
		case PermissionRO: perm = PTE_L2_AP_ALL_READ_ONLY; break;
		case PermissionRW: perm = PTE_L2_AP_ALL_READ_WRITE; break;
		case PermissionRWPriv: perm = PTE_L2_AP_ALL_READ_WRITE_PRIV; break;
	}

	// Set the appropriate entry of the top-level page table.  Since this is
	// a section mapping, there is no second-level table
	unsigned *table = (unsigned*)PADDR_TO_VADDR(mTablePAddr);
	unsigned int idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	table[idx] = (paddr & PTE_SECTION_BASE_MASK) | perm | PTE_TYPE_SECTION;
}

/*!
 * \brief Translate a virtual address into the corresponding physical address
 *        mapped to by this page table
 * \param addr Virtual address
 * \return Physical address
 */
PAddr PageTable::translateVAddr(void *addr)
{
	unsigned *table = (unsigned*)PADDR_TO_VADDR(mTablePAddr);
	int idx = (unsigned)addr >> PAGE_TABLE_SECTION_SHIFT;
	unsigned pte = table[idx];

	if((pte & PTE_TYPE_MASK) == PTE_TYPE_SECTION) {
		// Section mapping--dereference the top-level page table entry
		return (pte & PTE_SECTION_BASE_MASK) | ((unsigned)addr & (~PTE_SECTION_BASE_MASK));
	}

	// Coarse page table entry--indirect through second-level table to compute address
	unsigned *L2Table = (unsigned*)PADDR_TO_VADDR(pte & PTE_COARSE_BASE_MASK);
	int l2idx = ((unsigned)addr & (~PAGE_TABLE_SECTION_MASK)) >> PAGE_SHIFT;
	return (L2Table[l2idx] & PTE_L2_BASE_MASK) | ((unsigned)addr & (~PTE_L2_BASE_MASK));
}
