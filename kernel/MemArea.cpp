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
	mPages = Page_AllocMulti(MemArea::size() >> PAGE_SHIFT);
}

void MemAreaPages::map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size)
{
	struct Page *page;
	unsigned int skipPages;
	unsigned v;

	v = (unsigned)vaddr;
	skipPages = offset >> PAGE_SHIFT;
	LIST_FOREACH(mPages, page, struct Page, list) {
		if(skipPages > 0) {
			skipPages--;
			continue;
		}

		table->mapPage((void*)v, PAGE_TO_PADDR(page), PageTable::PermissionRW);
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
	PAddr paddr;
	unsigned v;

	v = (unsigned)vaddr;
	for(paddr = mPAddr; paddr < mPAddr + size; paddr += PAGE_SIZE, v += PAGE_SIZE) {
		table->mapPage((void*)v, paddr, PageTable::PermissionRW);
	}
}