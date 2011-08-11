#include "AddressSpace.h"
#include "Slab.h"
#include "AsmFuncs.h"
#include "Util.h"

struct AddressSpace KernelSpace;

struct SlabAllocator areaSlab;
struct SlabAllocator AddressSpaceSlab;

extern char vectorStart[];
extern char vectorEnd[];

void AddressSpace_Map(struct AddressSpace *space, void *start, LIST(struct Page) pages)
{
	struct Page *page;
	struct Area *area;
	struct Area *areaCursor;
	int size;
	char *vaddr;

	vaddr = start;
	size = 0;

	LIST_FOREACH(pages, page, struct Page, list) {
		PageTable_MapPage(space->pageTable, vaddr, PAGE_TO_PADDR(page));
		vaddr += PAGE_SIZE;
		size += PAGE_SIZE;
	}

	area = (struct Area*)Slab_Allocate(&areaSlab);
	area->start = start;
	area->size = size;
	area->pages = pages;

	LIST_FOREACH(space->areas, areaCursor, struct Area, list) {
		if(areaCursor->start > area->start) {
			LIST_ADD_AFTER(space->areas, area->list, areaCursor->list);
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

	space = (struct AddressSpace*)Slab_Allocate(&AddressSpaceSlab);
	LIST_INIT(space->areas);

	space->pageTable = PageTable_Create();

	return space;
}

void AddressSpace_CopyFrom(struct AddressSpace *space, void *dest, void *source, int size)
{
	int copied = 0;

	while(copied < size) {
		unsigned p = (unsigned)source + copied;
		unsigned aligned = p & PAGE_MASK;
		unsigned alignedNext = aligned + PAGE_SIZE;
		int pageSize = min(alignedNext - p, size - copied);
		void *addr = PADDR_TO_VADDR(PageTable_TranslateVAddr(space->pageTable, (void*)p));
		memcpy((char*)dest + copied, addr, pageSize);
		copied += pageSize;
	}
}

void AddressSpace_CopyTo(struct AddressSpace *space, void *dest, void *source, int size)
{
	int copied = 0;

	while(copied < size) {
		unsigned p = (unsigned)dest + copied;
		unsigned aligned = p & PAGE_MASK;
		unsigned alignedNext = aligned + PAGE_SIZE;
		int pageSize = min(alignedNext - p, size - copied);
		void *addr = PADDR_TO_VADDR(PageTable_TranslateVAddr(space->pageTable, (void*)p));
		memcpy(addr, (char*)source + copied, pageSize);
		copied += pageSize;
	}
}

void AddressSpace_Init()
{
	int i;
	unsigned int n;
	struct Page *vectorPage;
	char *vector;

	vectorPage = Page_Alloc();
	vector = PAGE_TO_VADDR(vectorPage);
	PageTable_MapPage(KernelSpace.pageTable, (void*)0xffff0000, PAGE_TO_PADDR(vectorPage));
	memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);

	Slab_Init(&AddressSpaceSlab, sizeof(struct AddressSpace));
	Slab_Init(&areaSlab, sizeof(struct Area));
}

SECTION_LOW void AddressSpace_InitLow()
{
	struct AddressSpace *kernelSpaceLow = (struct AddressSpace*)VADDR_TO_PADDR(&KernelSpace);

	kernelSpaceLow->pageTable = &KernelPageTable;
	LIST_INIT(kernelSpaceLow->areas);
}