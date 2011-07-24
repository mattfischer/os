#include "Memory.h"
#include "Defs.h"

SECTION(".kernelMap") unsigned KernelMap[PAGE_TABLE_SIZE];
char InitStack[256];

struct Page Pages[N_PAGES];

struct AddressSpace kernelSpace;
struct Page *vectorPage;

extern char vectorStart[];
extern char vectorEnd[];
void MemoryInit()
{
	int i;
	unsigned int n;
	char *vector;

	for(i=0; i<N_PAGES; i++) {
		Pages[i].flags = PAGE_FREE;
		Pages[i].next = NULL;
	}

	for(i=0; i<VADDR_TO_PAGE_NR(__KernelEnd); i++) {
		Pages[i].flags = PAGE_INUSE;
	}

	kernelSpace.pageTable = VADDR_TO_PAGE(KernelMap);
	kernelSpace.L2Tables = NULL;

	vectorPage = PageAlloc(1);
	vector = PAGE_TO_VADDR(vectorPage);
	MapPage(&kernelSpace, (void*)0xffff0000, vectorPage);
	for(i=0; i<((unsigned)vectorEnd - (unsigned)vectorStart); i++) {
		vector[i] = vectorStart[i];
	}
}

struct Page *PageAlloc(int num)
{
	struct Page *ret = NULL;
	struct Page *tail = NULL;
	int i;
	int n;

	for(i=0, n=0; i<N_PAGES && n < num; i++) {
		struct Page *page = PAGE(i);

		if(page->flags == PAGE_FREE) {
			page->flags = PAGE_INUSE;
			if(tail == NULL) {
				ret = page;
				tail = page;
			} else {
				tail->next = page;
			}
			tail = page;
			tail->next = NULL;
			n++;
		}
	}

	return ret;
}

struct Page *PageAllocContig(int align, int num)
{
	struct Page *ret;
	int i, j;

	for(i=0; i<N_PAGES; i += align) {
		struct Page *page = PAGE(i);

		for(j=0; j<num; j++) {
			if(PAGE(i + j)->flags == PAGE_INUSE) {
				break;
			}
		}

		if(j == num) {
			for(j=0; j<num; j++) {
				PAGE(i+j)->flags = PAGE_INUSE;
			}
			return page;
		}
	}

	return NULL;
}

void PageFree(struct Page *page)
{
	page->flags = PAGE_FREE;
	page->next = NULL;
}

void PageFreeAll(struct Page *page)
{
	struct Page *next;
	while(page != NULL) {
		next = page->next;
		PageFree(page);
		page = next;
	}
}

struct SlabHead {
	unsigned int bitfield[1];
};

void SlabInit(struct SlabAllocator *slab, int size)
{
	int i;
	int alignedSize;

	for(i=0; i<32; i++) {
		alignedSize = 1 << i;
		if(alignedSize >= size) {
			break;
		}
	}

	slab->order = i;
	slab->numPerPage = PAGE_SIZE >> slab->order;
	slab->bitfieldLen = (slab->numPerPage + 31) >> 5;
	slab->dataStart = (slab->bitfieldLen * 4 + alignedSize - 1) >> slab->order;
	slab->pages = NULL;
}

void *SlabAllocate(struct SlabAllocator *slab)
{
	struct Page *page;
	struct Page *prev;
	struct SlabHead *head;
	int i, j;

	prev = NULL;
	page = slab->pages;
	while(page != NULL) {
		head = (struct SlabHead*)PAGE_TO_VADDR(page);

		for(i=slab->dataStart; i<slab->numPerPage; i++) {
			int idx = i >> 5;
			int bit = i & 0x1f;
			int val = 1 << bit;

			if((head->bitfield[idx] & val) != 0) {
				continue;
			}

			head->bitfield[idx] |= val;

			return PAGE_TO_VADDR(page) + (i << slab->order);
		}

		prev = page;
		page = page->next;
	}

	page = PageAlloc(1);
	if(prev == NULL) {
		slab->pages = page;
	} else {
		prev->next = page;
	}
	head = (struct SlabHead*)PAGE_TO_VADDR(page);
	for(i=0; i<slab->bitfieldLen; i++) {
		head->bitfield[i] = 0;
	}
	head->bitfield[0] = 1 << slab->dataStart;
	return PAGE_TO_VADDR(page) + (slab->dataStart << slab->order);
}

void SlabFree(struct SlabAllocator *slab, void *p)
{
	struct Page *page = VADDR_TO_PAGE(p);
	struct Page *cursor;
	struct Page *prev;
	char *addr = PAGE_TO_VADDR(page);
	struct SlabHead *head = (struct SlabHead*)addr;
	int i = ((char*)p - addr) >> slab->order;
	int idx = i >> 5;
	int bit = i & 0x1f;
	int val = 1 << bit;

	head->bitfield[idx] &= ~val;

	for(i=0; i<slab->bitfieldLen; i++) {
		if(head->bitfield[i] != 0) {
			return;
		}
	}

	prev = NULL;
	cursor = slab->pages;
	while(cursor != page) {
		prev = cursor;
		cursor = cursor->next;
	}

	if(prev == NULL) {
		slab->pages = page->next;
	} else {
		prev->next = page->next;
	}

	PageFree(page);
}

void allocL2Table(struct AddressSpace *space, void *vaddr)
{
	unsigned *table;
	int idx;
	struct Page *L2Page;
	struct Page *L2Prev;
	unsigned *L2Table;
	int l2idx;
	unsigned l2pte;
	int i, j;

	table = (unsigned*)PAGE_TO_VADDR(space->pageTable);
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

	table = (unsigned*)PAGE_TO_VADDR(space->pageTable);
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