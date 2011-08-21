#include "MemArea.h"

SlabAllocator<MemAreaPages> MemAreaPages::sSlab;
SlabAllocator<MemAreaPhys> MemAreaPhys::sSlab;

MemArea::MemArea(int size)
{
	mSize = size;
}

MemAreaPages::MemAreaPages(int size)
 : MemArea(PAGE_SIZE_ROUND_UP(size))
{
	mPages = Page::allocMulti(MemArea::size() >> PAGE_SHIFT);
}

void MemAreaPages::map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size)
{
	unsigned v = (unsigned)vaddr;
	unsigned int skipPages = offset >> PAGE_SHIFT;
	for(Page *page = mPages.head(); page != NULL; page = mPages.next(page)) {
		if(skipPages > 0) {
			skipPages--;
			continue;
		}

		table->mapPage((void*)v, page->paddr(), PageTable::PermissionRW);
		v += PAGE_SIZE;
	}
}

MemAreaPhys::MemAreaPhys(int size, PAddr paddr)
 : MemArea(PAGE_SIZE_ROUND_UP(size))
{
	mPAddr = paddr;
}

void MemAreaPhys::map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size)
{
	unsigned v = (unsigned)vaddr;
	for(PAddr paddr = mPAddr; paddr < mPAddr + size; paddr += PAGE_SIZE, v += PAGE_SIZE) {
		table->mapPage((void*)v, paddr, PageTable::PermissionRW);
	}
}