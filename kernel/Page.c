#include "Page.h"
#include "Defs.h"

struct Page Pages[N_PAGES];
SECTION(".kernelMap") unsigned KernelMap[PAGE_TABLE_SIZE];

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

void PageInit()
{
	int i;

	for(i=0; i<N_PAGES; i++) {
		Pages[i].flags = PAGE_FREE;
		Pages[i].next = NULL;
	}
}