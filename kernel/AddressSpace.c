#include "AddressSpace.h"
#include "Slab.h"
#include "AsmFuncs.h"
#include "Util.h"

struct AddressSpace KernelSpace;

struct SlabAllocator areaSlab;
struct SlabAllocator AddressSpaceSlab;

extern char vectorStart[];
extern char vectorEnd[];

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

	L2Page = Page_Alloc();
	L2Table = (unsigned*)PAGE_TO_VADDR(L2Page);
	LIST_ADD_TAIL(space->L2Tables, L2Page->list);

	memset(L2Table, 0, PAGE_L2_TABLE_SIZE * sizeof(unsigned));
	for(i=1; i<4; i++) {
		L2Table[i*PAGE_L2_TABLE_SIZE] = 0x80000000;
	}

	table[idx] = VADDR_TO_PADDR(L2Table) | PTE_TYPE_COARSE;
}

void AddressSpace_MapPage(struct AddressSpace *space, void *vaddr, PAddr paddr)
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

SECTION_LOW void AddressSpace_MapSectionLow(struct AddressSpace *space, void *vaddr, PAddr paddr)
{
	unsigned *table;
	int idx;

	table = (unsigned*)PAGE_TO_PADDR(LIST_HEAD(space->table, struct Page, list));
	idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	table[idx] = (paddr & PTE_SECTION_BASE_MASK) | PTE_SECTION_AP_READ_WRITE | PTE_TYPE_SECTION;
}

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
		AddressSpace_MapPage(space, vaddr, PAGE_TO_PADDR(page));
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

	space->table = Page_AllocContig(4, 4);
	space->tablePAddr = PAGE_TO_PADDR(LIST_HEAD(space->table, struct Page, list));
	base = (unsigned*)PADDR_TO_VADDR(space->tablePAddr);

	kernel_nr = KERNEL_START >> PTE_SECTION_BASE_SHIFT;
	memset(base, 0, kernel_nr * sizeof(unsigned));

	kernelTable = (unsigned*)PADDR_TO_VADDR(KernelSpace.tablePAddr);
	memcpy(base + kernel_nr, kernelTable + kernel_nr, PAGE_TABLE_SIZE - kernel_nr);

	return space;
}

void AddressSpace_Init()
{
	int i;
	unsigned int n;
	struct Page *vectorPage;
	char *vector;

	vectorPage = Page_Alloc();
	vector = PAGE_TO_VADDR(vectorPage);
	AddressSpace_MapPage(&KernelSpace, (void*)0xffff0000, PAGE_TO_PADDR(vectorPage));
	memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);

	Slab_Init(&AddressSpaceSlab, sizeof(struct AddressSpace));
	Slab_Init(&areaSlab, sizeof(struct Area));
}

SECTION_LOW void AddressSpace_InitLow()
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

	kernelSpace->table = Page_AllocContigLow(4, 4);
	kernelSpace->tablePAddr = PAGE_TO_PADDR(LIST_HEAD(kernelSpace->table, struct Page, list));
	LIST_INIT(kernelSpace->L2Tables);
	LIST_INIT(kernelSpace->areas);

	for(vaddr = 0, paddr = 0; vaddr < KERNEL_START; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		AddressSpace_MapSectionLow(kernelSpace, (void*)vaddr, paddr);
	}

	for(vaddr = KERNEL_START, paddr = 0; vaddr > 0; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		AddressSpace_MapSectionLow(kernelSpace, (void*)vaddr, paddr);
	}
}