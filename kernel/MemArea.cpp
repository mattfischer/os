#include "MemArea.h"

//! Slab allocator
Slab<MemAreaPages> MemAreaPages::sSlab;

//! Slab allocator
Slab<MemAreaPhys> MemAreaPhys::sSlab;

/*!
 * \brief Constructor
 * \param size Size of area
 */
MemArea::MemArea(int size)
{
	mSize = size;
}

/*!
 * \brief Constructor
 * \param size Size of area
 */
MemAreaPages::MemAreaPages(int size)
 : MemArea(PAGE_SIZE_ROUND_UP(size))
{
	// Allocate the appropriate number of pages
	mPages = Page::allocMulti(MemArea::size() >> PAGE_SHIFT);
}

void MemAreaPages::map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size)
{
	unsigned v = (unsigned)vaddr;
	unsigned int skipPages = offset >> PAGE_SHIFT;
	for(Page *page = mPages.head(); page != NULL; page = mPages.next(page)) {
		// Skip pages until we get to the one containing the beginning of the mapping
		if(skipPages > 0) {
			skipPages--;
			continue;
		}

		// Map each page into the table
		table->mapPage((void*)v, page->paddr(), PageTable::PermissionRW);
		v += PAGE_SIZE;
	}
}

/*!
 * \brief Constructor
 * \param size Size of area
 * \param paddr Starting physical address of area
 */
MemAreaPhys::MemAreaPhys(int size, PAddr paddr)
 : MemArea(PAGE_SIZE_ROUND_UP(size))
{
	mPAddr = paddr;
}

void MemAreaPhys::map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size)
{
	unsigned v = (unsigned)vaddr;
	for(PAddr paddr = mPAddr; paddr < mPAddr + size; paddr += PAGE_SIZE, v += PAGE_SIZE) {
		// Map each page of physical address space into the page table
		table->mapPage((void*)v, paddr, PageTable::PermissionRW);
	}
}