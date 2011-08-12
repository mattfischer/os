#include "AddressSpace.h"
#include "Slab.h"
#include "AsmFuncs.h"
#include "Util.h"

struct AddressSpace KernelSpace;

static struct SlabAllocator mappingSlab;
static struct SlabAllocator addressSpaceSlab;

extern char vectorStart[];
extern char vectorEnd[];

void AddressSpace_Map(struct AddressSpace *space, struct MemArea *area, void *vaddr, unsigned int offset, unsigned int size)
{
	struct Mapping *mapping;
	struct Mapping *mappingCursor;

	mapping = (struct Mapping*)Slab_Allocate(&mappingSlab);
	mapping->vaddr = (void*)PAGE_ADDR_ROUND_DOWN(vaddr);
	mapping->offset = PAGE_ADDR_ROUND_DOWN(offset);
	mapping->size = PAGE_SIZE_ROUND_UP(size + offset - mapping->offset);
	mapping->area = area;

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

				PageTable_MapPage(space->pageTable, vaddr, PAGE_TO_PADDR(page), PageTablePermissionRW);
				vaddr += PAGE_SIZE;
			}
			break;
		}

		case MemAreaTypePhys:
		{
			PAddr paddr;
			for(paddr = area->u.paddr; paddr < area->u.paddr + mapping->size; paddr += PAGE_SIZE, vaddr += PAGE_SIZE) {
				PageTable_MapPage(space->pageTable, vaddr, paddr, PageTablePermissionRW);
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
	PageTable_MapPage(KernelSpace.pageTable, (void*)0xffff0000, PAGE_TO_PADDR(vectorPage), PageTablePermissionRWPriv);
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