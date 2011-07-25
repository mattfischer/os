#include "Map.h"
#include "Slab.h"
#include "Defs.h"

struct AddressSpace kernelSpace;
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
			if((l2pte & PTE_L2_TYPE_MASK) != PTE_L2_TYPE_DISABLED || (l2pte & 0x80000000)) {
				continue;
			}

			for(j=0; j<PAGE_L2_TABLE_SIZE; j++) {
				L2Table[l2idx + j] = 0;
			}
			table[idx] = (unsigned)VADDR_TO_PADDR(L2Table + l2idx) | PTE_SECTION_AP_READ_WRITE | PTE_TYPE_COARSE;
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

	table[idx] = (unsigned)VADDR_TO_PADDR(L2Table) | PTE_TYPE_COARSE;
}

void MapPage(struct AddressSpace *space, void *vaddr, struct Page *page)
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
		L2Table[l2idx] = ((unsigned)PAGE_TO_PADDR(page) & PTE_COARSE_BASE_MASK) | PTE_L2_AP_ALL_READ_WRITE | PTE_L2_TYPE_SMALL;
	}
}

void MapPages(struct AddressSpace *space, void *start, struct Page *pages)
{
	struct Page *page;
	struct Map *map;
	struct Map *mapCursor;
	struct Map *mapPrev;
	int size;
	void *vaddr;

	vaddr = start;
	size = 0;
	page = pages;
	while(page != NULL) {
		MapPage(space, vaddr, page);
		page = page->next;
		vaddr = (char*)vaddr + PAGE_SIZE;
		size += PAGE_SIZE;
	}

	map = (struct Map*)SlabAllocate(&mapSlab);
	map->start = start;
	map->size = size;
	map->pages = pages;

	mapCursor = space->maps;
	mapPrev = NULL;
	while(mapCursor != NULL) {
		if((unsigned)mapCursor->start > (unsigned)map->start) {
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
}

void MapInit()
{
	int i;
	unsigned int n;
	char *vector;

	SlabInit(&mapSlab, sizeof(struct Map));

	for(i=0; i<VADDR_TO_PAGE_NR(__KernelEnd); i++) {
		Pages[i].flags = PAGE_INUSE;
	}

	kernelSpace.table = VADDR_TO_PAGE(KernelMap);
	kernelSpace.L2Tables = NULL;

	vectorPage = PageAlloc(1);
	vector = PAGE_TO_VADDR(vectorPage);
	MapPage(&kernelSpace, (void*)0xffff0000, vectorPage);
	for(i=0; i<((unsigned)vectorEnd - (unsigned)vectorStart); i++) {
		vector[i] = vectorStart[i];
	}
}