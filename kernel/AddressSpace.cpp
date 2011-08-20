#include "AddressSpace.h"
#include "AsmFuncs.h"
#include "Util.h"

AddressSpace *AddressSpace::Kernel;
SlabAllocator<struct AddressSpace> AddressSpace::sSlab;

static struct SlabAllocator<struct Mapping> mappingSlab;

extern char vectorStart[];
extern char vectorEnd[];

AddressSpace::AddressSpace(struct PageTable *pageTable)
{
	LIST_INIT(mMappings);

	if(pageTable == NULL) {
		pageTable = PageTable_Create();
	}

	mPageTable = pageTable;
}

void AddressSpace::map(struct MemArea *area, void *vaddr, unsigned int offset, unsigned int size)
{
	struct Mapping *mapping;
	struct Mapping *mappingCursor;
	unsigned v;

	mapping = mappingSlab.allocate();
	mapping->vaddr = (void*)PAGE_ADDR_ROUND_DOWN(vaddr);
	mapping->offset = PAGE_ADDR_ROUND_DOWN(offset);
	mapping->size = PAGE_SIZE_ROUND_UP(size + offset - mapping->offset);
	mapping->area = area;

	v = (unsigned)vaddr;

	switch(area->type) {
		case MemAreaTypePages:
		{
			struct Page *page;
			unsigned int skipPages;

			skipPages = mapping->offset >> PAGE_SHIFT;
			LIST_FOREACH(area->u.pages, page, struct Page, list) {
				if(skipPages > 0) {
					skipPages--;
					continue;
				}

				PageTable_MapPage(mPageTable, (void*)v, PAGE_TO_PADDR(page), PageTablePermissionRW);
				v += PAGE_SIZE;
			}
			break;
		}

		case MemAreaTypePhys:
		{
			PAddr paddr;
			for(paddr = area->u.paddr; paddr < area->u.paddr + mapping->size; paddr += PAGE_SIZE, v += PAGE_SIZE) {
				PageTable_MapPage(mPageTable, (void*)v, paddr, PageTablePermissionRW);
			}
			break;
		}
	}

	LIST_FOREACH(mMappings, mappingCursor, struct Mapping, list) {
		if(mappingCursor->vaddr > mapping->vaddr) {
			LIST_ADD_AFTER(mMappings, mapping->list, mappingCursor->list);
			break;
		}
	}

	FlushTLB();
}

static unsigned nextPageBoundary(unsigned addr)
{
	return (addr & PAGE_MASK) + PAGE_SIZE;
}

void AddressSpace::memcpy(AddressSpace *destSpace, void *dest, AddressSpace *srcSpace, void *src, int size)
{
	unsigned srcPtr = (unsigned)src;
	unsigned destPtr = (unsigned)dest;

	while(size > 0) {
		int srcSize = nextPageBoundary(srcPtr) - srcPtr;
		int destSize = nextPageBoundary(destPtr) - destPtr;
		int copySize = min(min(srcSize, destSize), size);

		void *srcKernel;
		void *destKernel;

		if(srcSpace == NULL) {
			srcKernel = (void*)srcPtr;
		} else {
			srcKernel = PADDR_TO_VADDR(PageTable_TranslateVAddr(srcSpace->pageTable(), (void*)srcPtr));
		}

		if(destSpace == NULL ) {
			destKernel = (void*)destPtr;
		} else {
			destKernel = PADDR_TO_VADDR(PageTable_TranslateVAddr(destSpace->pageTable(), (void*)destPtr));
		}

		::memcpy(destKernel, srcKernel, copySize);
		srcPtr += copySize;
		destPtr += copySize;
		size -= copySize;
	}
}

void AddressSpace::init()
{
	int i;
	unsigned int n;
	struct Page *vectorPage;
	char *vector;

	Kernel = new AddressSpace(&KernelPageTable);

	vectorPage = Page_Alloc();
	vector = PAGE_TO_VADDR(vectorPage);
	PageTable_MapPage(Kernel->pageTable(), (void*)0xffff0000, PAGE_TO_PADDR(vectorPage), PageTablePermissionRWPriv);
	::memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);
}
