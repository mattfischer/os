#include "AddressSpace.h"
#include "Slab.h"
#include "AsmFuncs.h"
#include "Util.h"

struct AddressSpace *KernelSpace;

static struct SlabAllocator mappingSlab;
static struct SlabAllocator addressSpaceSlab;

extern char vectorStart[];
extern char vectorEnd[];

void AddressSpace_Map(struct AddressSpace *space, struct MemArea *area, void *vaddr, unsigned int offset, unsigned int size)
{
	struct Mapping *mapping;
	struct Mapping *mappingCursor;
	unsigned v;

	mapping = (struct Mapping*)Slab_Allocate(&mappingSlab);
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

				PageTable_MapPage(space->pageTable, (void*)v, PAGE_TO_PADDR(page), PageTablePermissionRW);
				v += PAGE_SIZE;
			}
			break;
		}

		case MemAreaTypePhys:
		{
			PAddr paddr;
			for(paddr = area->u.paddr; paddr < area->u.paddr + mapping->size; paddr += PAGE_SIZE, v += PAGE_SIZE) {
				PageTable_MapPage(space->pageTable, (void*)v, paddr, PageTablePermissionRW);
			}
			break;
		}
	}

	LIST_FOREACH(space->mappings, mappingCursor, struct Mapping, list) {
		if(mappingCursor->vaddr > mapping->vaddr) {
			LIST_ADD_AFTER(space->mappings, mapping->list, mappingCursor->list);
			break;
		}
	}

	FlushTLB();
}

struct AddressSpace *AddressSpace_Create()
{
	struct AddressSpace *space;
	unsigned *base;
	unsigned *kernelTable;
	int kernel_nr;

	space = (struct AddressSpace*)Slab_Allocate(&addressSpaceSlab);
	LIST_INIT(space->mappings);

	space->pageTable = PageTable_Create();

	return space;
}

static unsigned nextPageBoundary(unsigned addr)
{
	return (addr & PAGE_MASK) + PAGE_SIZE;
}

void AddressSpace_Memcpy(struct AddressSpace *destSpace, void *dest, struct AddressSpace *srcSpace, void *src, int size)
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
			srcKernel = PADDR_TO_VADDR(PageTable_TranslateVAddr(srcSpace->pageTable, (void*)srcPtr));
		}

		if(destSpace == NULL ) {
			destKernel = (void*)destPtr;
		} else {
			destKernel = PADDR_TO_VADDR(PageTable_TranslateVAddr(destSpace->pageTable, (void*)destPtr));
		}

		memcpy(destKernel, srcKernel, copySize);
		srcPtr += copySize;
		destPtr += copySize;
		size -= copySize;
	}
}

void AddressSpace_Init()
{
	int i;
	unsigned int n;
	struct Page *vectorPage;
	char *vector;

	Slab_Init(&addressSpaceSlab, sizeof(struct AddressSpace));
	Slab_Init(&mappingSlab, sizeof(struct Mapping));

	KernelSpace = AddressSpace_Create();
	KernelSpace->pageTable = &KernelPageTable;
	LIST_INIT(KernelSpace->mappings);

	vectorPage = Page_Alloc();
	vector = PAGE_TO_VADDR(vectorPage);
	PageTable_MapPage(KernelSpace->pageTable, (void*)0xffff0000, PAGE_TO_PADDR(vectorPage), PageTablePermissionRWPriv);
	memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);
}
