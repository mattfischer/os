#include "MemArea.hpp"

#include "PageTable.hpp"

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
	mSize = PAGE_SIZE_ROUND_UP(size);
}

/*!
 * \brief Expand a memory area
 * \param newSize New size
 */
void MemArea::expand(int newSize)
{
	int newSizeRounded = PAGE_SIZE_ROUND_UP(newSize);

	doExpand(newSizeRounded);

	mSize = newSize;
}

/*!
 * \brief Perform an expand operation
 * \param newSize New size
 */
void MemArea::doExpand(int newSize)
{
	// Placeholder for subclasses
}

void MemArea::onLastRef()
{
	free();
}

/*!
 * \brief Constructor
 * \param size Size of area
 */
MemAreaPages::MemAreaPages(int size)
 : MemArea(size)
{
	// Allocate the appropriate number of pages
	mPages = Page::allocMulti(MemArea::size() >> PAGE_SHIFT);
}

MemAreaPages::~MemAreaPages()
{
	for(Page *page = mPages.head(); page != 0; page = mPages.next(page)) {
		page->free();
	}
}

void MemAreaPages::map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size)
{
	unsigned v = (unsigned)vaddr;
	unsigned int skipPages = offset >> PAGE_SHIFT;
	for(Page *page = mPages.head(); page != 0; page = mPages.next(page)) {
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

void MemAreaPages::doExpand(int newSize)
{
	if(newSize <= size()) {
		return;
	}

	unsigned int newPages = (newSize - size()) >> PAGE_SHIFT;

	for(int i=0; i<newPages; i++) {
		Page *page = Page::alloc();
		mPages.addTail(page);
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
