#include "AddressSpace.h"
#include "Slab.h"
#include "AsmFuncs.h"
#include "Util.h"

struct AddressSpace KernelSpace;

static struct SlabAllocator mappingSlab;
static struct SlabAllocator addressSpaceSlab;

extern char vectorStart[];
extern char vectorEnd[];

void AddressSpace_Map(struct AddressSpace *space, void *start, struct MemArea *area)
{
	struct Page *page;
	struct Mapping *mapping;
	struct Mapping *mappingCursor;
	int size;
	char *vaddr;

	vaddr = start;
	size = 0;

	LIST_FOREACH(area->pages, page, struct Page, list) {
		PageTable_MapPage(space->pageTable, vaddr, PAGE_TO_PADDR(page));
		vaddr += PAGE_SIZE;
		size += PAGE_SIZE;
	}

	mapping = (struct Mapping*)Slab_Allocate(&mappingSlab);
	mapping->start = start;
	mapping->area = area;

	LIST_FOREACH(space->mappings, mappingCursor, struct Mapping, list) {
		if(mappingCursor->start > mapping->start) {
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

	Slab_Init(&addressSpaceSlab, sizeof(struct AddressSpace));
	Slab_Init(&mappingSlab, sizeof(struct Mapping));
}

SECTION_LOW void AddressSpace_InitLow()
{
	struct AddressSpace *kernelSpaceLow = (struct AddressSpace*)VADDR_TO_PADDR(&KernelSpace);

	kernelSpaceLow->pageTable = &KernelPageTable;
	LIST_INIT(kernelSpaceLow->mappings);
}