#include "PageTable.h"

#include "Util.h"
#include "Slab.h"

#define PAGE_TABLE_SIZE 4096

#define PAGE_TABLE_SECTION_SIZE (1024 * 1024)
#define PAGE_TABLE_SECTION_MASK 0xfff00000
#define PAGE_TABLE_SECTION_SHIFT 20

#define PTE_TYPE_MASK 3
#define PTE_TYPE_DISABLED 0
#define PTE_TYPE_COARSE 1
#define PTE_TYPE_SECTION 2

#define PTE_SECTION_AP_SHIFT 10
#define PTE_SECTION_AP_READ_WRITE (0x3 << PTE_SECTION_AP_SHIFT)
#define PTE_SECTION_BASE_MASK 0xfff00000
#define PTE_SECTION_BASE_SHIFT 20

#define PTE_COARSE_BASE_MASK 0xfffffc00
#define PTE_COARSE_BASE_SHIFT 10

#define PAGE_L2_TABLE_SIZE 256

#define PTE_L2_TYPE_MASK 3
#define PTE_L2_TYPE_DISABLED 0
#define PTE_L2_TYPE_LARGE 1
#define PTE_L2_TYPE_SMALL 2

#define PTE_L2_AP0_SHIFT 4
#define PTE_L2_AP1_SHIFT 6
#define PTE_L2_AP2_SHIFT 8
#define PTE_L2_AP3_SHIFT 10

#define PTE_L2_AP_READ_WRITE 0x3
#define PTE_L2_AP_ALL_READ_WRITE 0xff0

#define PTE_L2_BASE_MASK 0xfffff000
#define PTE_L2_BASE_SHIFT 12

static struct SlabAllocator pageTableSlab;

struct PageTable KernelPageTable;

struct PageTable *PageTable_Create()
{
	struct PageTable *pageTable;
	int kernel_nr;
	unsigned *base;
	unsigned *kernelTable;

	pageTable = (struct PageTable*)Slab_Allocate(&pageTableSlab);
	pageTable->table = Page_AllocContig(4, 4);
	pageTable->tablePAddr = PAGE_TO_PADDR(pageTable->table);

	base = (unsigned*)PADDR_TO_VADDR(pageTable->tablePAddr);

	kernel_nr = KERNEL_START >> PTE_SECTION_BASE_SHIFT;
	memset(base, 0, kernel_nr * sizeof(unsigned));

	kernelTable = (unsigned*)PADDR_TO_VADDR(KernelPageTable.tablePAddr);
	memcpy(base + kernel_nr, kernelTable + kernel_nr, (PAGE_TABLE_SIZE - kernel_nr) * sizeof(unsigned));

	return pageTable;
}

static void allocL2Table(struct PageTable *pageTable, void *vaddr)
{
	unsigned *table;
	int idx;
	struct Page *L2Page;
	unsigned *L2Table;
	int l2idx;
	unsigned l2pte;
	int i, j;

	table = (unsigned*)PADDR_TO_VADDR(pageTable->tablePAddr);
	idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;

	LIST_FOREACH(pageTable->L2Tables, L2Page, struct Page, list) {
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
	LIST_ADD_TAIL(pageTable->L2Tables, L2Page->list);

	memset(L2Table, 0, PAGE_L2_TABLE_SIZE * sizeof(unsigned));
	for(i=1; i<4; i++) {
		L2Table[i*PAGE_L2_TABLE_SIZE] = 0x80000000;
	}

	table[idx] = VADDR_TO_PADDR(L2Table) | PTE_TYPE_COARSE;
}

void PageTable_MapPage(struct PageTable *pageTable, void *vaddr, PAddr paddr)
{
	unsigned *table;
	int idx;
	unsigned pte;
	unsigned *L2Table;
	int l2idx;

	table = (unsigned*)PADDR_TO_VADDR(pageTable->tablePAddr);
	idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	pte = table[idx];

	if((pte & PTE_TYPE_MASK) == PTE_TYPE_SECTION ||
	   (pte & PTE_TYPE_MASK) == PTE_TYPE_DISABLED) {
	   allocL2Table(pageTable, vaddr);
	}

	pte = table[idx];

	if((pte & PTE_TYPE_MASK) == PTE_TYPE_COARSE) {
		L2Table = (unsigned*)PADDR_TO_VADDR(pte & PTE_COARSE_BASE_MASK);
		l2idx = ((unsigned)vaddr & (~PAGE_TABLE_SECTION_MASK)) >> PAGE_SHIFT;
		L2Table[l2idx] = (paddr & PTE_COARSE_BASE_MASK) | PTE_L2_AP_ALL_READ_WRITE | PTE_L2_TYPE_SMALL;
	}
}

SECTION_LOW void PageTable_MapSectionLow(struct PageTable *pageTable, void *vaddr, PAddr paddr)
{
	unsigned *table;
	int idx;

	table = (unsigned*)pageTable->tablePAddr;
	idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	table[idx] = (paddr & PTE_SECTION_BASE_MASK) | PTE_SECTION_AP_READ_WRITE | PTE_TYPE_SECTION;
}

PAddr PageTable_TranslateVAddr(struct PageTable *pageTable, void *addr)
{
	unsigned *table = (unsigned*)PADDR_TO_VADDR(pageTable->tablePAddr);
	int idx = (unsigned)addr >> PAGE_TABLE_SECTION_SHIFT;
	unsigned pte = table[idx];

	if((pte & PTE_TYPE_MASK) == PTE_TYPE_SECTION) {
		return (pte & PTE_SECTION_BASE_MASK) | ((unsigned)addr & (~PTE_SECTION_BASE_MASK));
	}

	unsigned *L2Table = (unsigned*)PADDR_TO_VADDR(pte & PTE_COARSE_BASE_MASK);
	int l2idx = ((unsigned)addr & (~PAGE_TABLE_SECTION_MASK)) >> PAGE_SHIFT;
	return (L2Table[l2idx] & PTE_L2_BASE_MASK) | ((unsigned)addr & (~PTE_L2_BASE_MASK));
}

void PageTable_Init()
{
	Slab_Init(&pageTableSlab, sizeof(struct PageTable));
}

SECTION_LOW void PageTable_InitLow()
{
	struct PageTable *kernelTableLow;
	unsigned int vaddr;
	PAddr paddr;
	int i;
	struct Page *pagesLow = (struct Page*)VADDR_TO_PADDR(Pages);

	for(i=0; i<VADDR_TO_PAGE_NR(__KernelEnd) + 1; i++) {
		pagesLow[i].flags = PAGE_INUSE;
		LIST_ENTRY_CLEAR(pagesLow[i].list);
	}

	kernelTableLow = (struct PageTable*)VADDR_TO_PADDR(&KernelPageTable);
	kernelTableLow->table = Page_AllocContigLow(4, 4);
	kernelTableLow->tablePAddr = PAGE_TO_PADDR(kernelTableLow->table);
	LIST_INIT(kernelTableLow->L2Tables);

	for(vaddr = 0, paddr = 0; vaddr < KERNEL_START; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		PageTable_MapSectionLow(kernelTableLow, (void*)vaddr, paddr);
	}

	for(vaddr = KERNEL_START, paddr = 0; vaddr > 0; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		PageTable_MapSectionLow(kernelTableLow, (void*)vaddr, paddr);
	}
}