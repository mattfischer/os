#include "Slab.h"
#include "Map.h"
#include "Page.h"
#include "Defs.h"

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
