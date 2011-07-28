#include "Map.h"
#include "Slab.h"
#include "Defs.h"
#include "AsmFuncs.h"

struct AddressSpace KernelSpace;
struct Page *vectorPage;

extern char vectorStart[];
extern char vectorEnd[];

struct SlabAllocator mapSlab;

static void allocL2Table(struct AddressSpace *space, void *vaddr)
{
	unsigned *table;
	int idx;
	struct Page *L2Page;
	struct Page *L2Prev;
	unsigned *L2Table;
	int l2idx;
	unsigned l2pte;
	int i, j;

	table = (unsigned*)PAGE_TO_VADDR(space->table);
	idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;

	L2Page = space->L2Tables;
	L2Prev = NULL;

	while(L2Page != NULL) {
		L2Table = (unsigned*)PAGE_TO_VADDR(L2Page);
		for(i=0; i<4; i++) {
			l2idx = i*PAGE_L2_TABLE_SIZE;
			l2pte = L2Table[l2idx];
			if((l2pte & PTE_L2_TYPE_MASK) != PTE_L2_TYPE_DISABLED || (l2pte & 0x80000000) == 0) {
				continue;
			}

			for(j=0; j<PAGE_L2_TABLE_SIZE; j++) {
				L2Table[l2idx + j] = 0;
			}
			table[idx] = VADDR_TO_PADDR(L2Table + l2idx) | PTE_SECTION_AP_READ_WRITE | PTE_TYPE_COARSE;
			return;
		}

		L2Prev = L2Page;
		L2Page = L2Page->next;
	}

	L2Page = PageAlloc(1);
	L2Table = (unsigned*)PAGE_TO_VADDR(L2Page);
	if(L2Prev == NULL) {
		space->L2Tables = L2Page;
	} else {
		L2Prev->next = L2Page;
	}

	for(i=1; i<4; i++) {
		L2Table[i*PAGE_L2_TABLE_SIZE] = 0x80000000;
	}

	for(i=0; i<PAGE_L2_TABLE_SIZE; i++) {
		L2Table[i] = 0;
	}

	table[idx] = VADDR_TO_PADDR(L2Table) | PTE_TYPE_COARSE;
}

void MapPage(struct AddressSpace *space, void *vaddr, PAddr paddr)
{
	unsigned *table;
	int idx;
	unsigned pte;
	unsigned *L2Table;
	int l2idx;

	table = (unsigned*)PAGE_TO_VADDR(space->table);
	idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	pte = table[idx];

	if((pte & PTE_TYPE_MASK) == PTE_TYPE_SECTION ||
	   (pte & PTE_TYPE_MASK) == PTE_TYPE_DISABLED) {
	   allocL2Table(space, vaddr);
	}

	pte = table[idx];

	if((pte & PTE_TYPE_MASK) == PTE_TYPE_COARSE) {
		L2Table = (unsigned*)PADDR_TO_VADDR(pte & PTE_COARSE_BASE_MASK);
		l2idx = ((unsigned)vaddr & (~PAGE_TABLE_SECTION_MASK)) >> PAGE_SHIFT;
		L2Table[l2idx] = (paddr & PTE_COARSE_BASE_MASK) | PTE_L2_AP_ALL_READ_WRITE | PTE_L2_TYPE_SMALL;
	}
}

SECTION_LOW void MapSectionLow(struct AddressSpace *space, void *vaddr, PAddr paddr)
{
	unsigned *table;
	int idx;

	table = (unsigned*)PAGE_TO_PADDR(space->table);
	idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	table[idx] = (paddr & PTE_SECTION_BASE_MASK) | PTE_SECTION_AP_READ_WRITE | PTE_TYPE_SECTION;
}

void MapPages(struct AddressSpace *space, void *start, struct Page *pages)
{
	struct Page *page;
	struct Map *map;
	struct Map *mapCursor;
	struct Map *mapPrev;
	int size;
	char *vaddr;

	vaddr = start;
	size = 0;
	page = pages;
	while(page != NULL) {
		MapPage(space, vaddr, PAGE_TO_PADDR(page));
		page = page->next;
		vaddr += PAGE_SIZE;
		size += PAGE_SIZE;
	}

	map = (struct Map*)SlabAllocate(&mapSlab);
	map->start = start;
	map->size = size;
	map->pages = pages;

	mapCursor = space->maps;
	mapPrev = NULL;
	while(mapCursor != NULL) {
		if(mapCursor->start > map->start) {
			break;
		}

		mapPrev = mapCursor;
		mapCursor = mapCursor->next;
	}

	if(mapPrev == NULL) {
		space->maps = map;
	} else {
		mapPrev->next = map;
	}

	map->next = mapCursor;

	FlushTLB();
}

void MapInit()
{
	int i;
	unsigned int n;
	char *vector;

	SlabInit(&mapSlab, sizeof(struct Map));

	vectorPage = PageAlloc(1);
	vector = PAGE_TO_VADDR(vectorPage);
	MapPage(&KernelSpace, (void*)0xffff0000, PAGE_TO_PADDR(vectorPage));
	for(i=0; i<((unsigned)vectorEnd - (unsigned)vectorStart); i++) {
		vector[i] = vectorStart[i];
	}
}

SECTION_LOW void MapInitLow()
{
	struct Page *pages = (struct Page*)VADDR_TO_PADDR(Pages);
	struct AddressSpace *kernelSpace = (struct AddressSpace*)VADDR_TO_PADDR(&KernelSpace);
	unsigned int vaddr;
	PAddr paddr;
	int i;

	for(i=0; i<VADDR_TO_PAGE_NR(__KernelEnd) + 1; i++) {
		pages[i].flags = PAGE_INUSE;
		pages[i].next = NULL;
	}

	kernelSpace->table = PageAllocContigLow(4, 4);
	kernelSpace->tablePAddr = PAGE_TO_PADDR(kernelSpace->table);
	kernelSpace->L2Tables = NULL;
	kernelSpace->maps = NULL;

	for(vaddr = 0, paddr = 0; vaddr < KERNEL_START; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		MapSectionLow(kernelSpace, (void*)vaddr, paddr);
	}

	for(vaddr = KERNEL_START, paddr = 0; vaddr > 0; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		MapSectionLow(kernelSpace, (void*)vaddr, paddr);
	}
}