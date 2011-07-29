#include "Map.h"
#include "Slab.h"
#include "Defs.h"
#include "AsmFuncs.h"
#include "Util.h"

struct AddressSpace KernelSpace;

extern char vectorStart[];
extern char vectorEnd[];

struct SlabAllocator mapSlab;

static void allocL2Table(struct AddressSpace *space, void *vaddr)
{
	unsigned *table;
	int idx;
	struct Page *L2Page;
	unsigned *L2Table;
	int l2idx;
	unsigned l2pte;
	int i, j;

	table = (unsigned*)PAGE_TO_VADDR(LIST_HEAD(space->table, struct Page, list));
	idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;

	LIST_FOREACH(space->L2Tables, L2Page, struct Page, list) {
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
	}

	L2Page = PageAlloc();
	L2Table = (unsigned*)PAGE_TO_VADDR(L2Page);
	LIST_ADD_TAIL(space->L2Tables, L2Page->list);

	memset(L2Table, 0, PAGE_L2_TABLE_SIZE * sizeof(unsigned));
	for(i=1; i<4; i++) {
		L2Table[i*PAGE_L2_TABLE_SIZE] = 0x80000000;
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

	table = (unsigned*)PAGE_TO_VADDR(LIST_HEAD(space->table, struct Page, list));
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

	table = (unsigned*)PAGE_TO_PADDR(LIST_HEAD(space->table, struct Page, list));
	idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	table[idx] = (paddr & PTE_SECTION_BASE_MASK) | PTE_SECTION_AP_READ_WRITE | PTE_TYPE_SECTION;
}

void MapCreate(struct AddressSpace *space, void *start, struct List pages)
{
	struct Page *page;
	struct Map *map;
	struct Map *mapCursor;
	int size;
	char *vaddr;

	vaddr = start;
	size = 0;

	LIST_FOREACH(pages, page, struct Page, list) {
		MapPage(space, vaddr, PAGE_TO_PADDR(page));
		vaddr += PAGE_SIZE;
		size += PAGE_SIZE;
	}

	map = (struct Map*)SlabAllocate(&mapSlab);
	map->start = start;
	map->size = size;
	map->pages = pages;

	LIST_FOREACH(space->maps, mapCursor, struct Map, list) {
		if(mapCursor->start > map->start) {
			LIST_ADD_AFTER(space->maps, map->list, mapCursor->list);
			break;
		}
	}

	FlushTLB();
}

void MapInit()
{
	int i;
	unsigned int n;
	struct Page *vectorPage;
	char *vector;

	SlabInit(&mapSlab, sizeof(struct Map));

	vectorPage = PageAlloc();
	vector = PAGE_TO_VADDR(vectorPage);
	MapPage(&KernelSpace, (void*)0xffff0000, PAGE_TO_PADDR(vectorPage));
	memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);
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
		LIST_ENTRY_CLEAR(pages[i].list);
	}

	kernelSpace->table = PageAllocContigLow(4, 4);
	kernelSpace->tablePAddr = PAGE_TO_PADDR(LIST_HEAD(kernelSpace->table, struct Page, list));
	LIST_INIT(kernelSpace->L2Tables);
	LIST_INIT(kernelSpace->maps);

	for(vaddr = 0, paddr = 0; vaddr < KERNEL_START; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		MapSectionLow(kernelSpace, (void*)vaddr, paddr);
	}

	for(vaddr = KERNEL_START, paddr = 0; vaddr > 0; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		MapSectionLow(kernelSpace, (void*)vaddr, paddr);
	}
}