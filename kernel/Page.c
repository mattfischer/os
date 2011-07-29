#include "Page.h"
#include "Map.h"
#include "Defs.h"

struct Page Pages[N_PAGES];

struct Page *PageAlloc()
{
	int i;

	for(i=0; i<N_PAGES; i++) {
		struct Page *page = PAGE(i);

		if(page->flags == PAGE_FREE) {
			page->flags = PAGE_INUSE;
			return page;
		}
	}

	return NULL;
}

struct List PageAllocMulti(int num)
{
	struct List list;
	int n;

	LIST_INIT(list);

	for(n=0; n < num; n++) {
		struct Page *page = PageAlloc();
		LIST_ADD_TAIL(list, page->list);
	}

	return list;
}

struct List PageAllocContig(int align, int num)
{
	struct List list;
	int i, j;

	LIST_INIT(list);
	for(i=0; i<N_PAGES; i += align) {
		for(j=0; j<num; j++) {
			struct Page *page = PAGE(i + j);

			if(page->flags == PAGE_INUSE) {
				break;
			}
		}

		if(j == num) {
			for(j=0; j<num; j++) {
				struct Page *page = PAGE(i + j);

				page->flags = PAGE_INUSE;
				LIST_ADD_TAIL(list, page->list);
			}
			return list;
		}
	}

	return list;
}

SECTION_LOW struct List PageAllocContigLow(int align, int num)
{
	struct List list;
	int i, j;

	LIST_INIT(list);
	for(i=0; i<N_PAGES; i += align) {
		for(j=0; j<num; j++) {
			struct Page *page = PAGE(i + j);
			struct Page *pageLow = (struct Page*)VADDR_TO_PADDR(page);

			if(pageLow->flags == PAGE_INUSE) {
				break;
			}
		}

		if(j == num) {
			for(j=0; j<num; j++) {
				struct Page *page = PAGE(i + j);
				struct Page *pageLow = (struct Page*)VADDR_TO_PADDR(page);

				pageLow->flags = PAGE_INUSE;
				LIST_ADD_TAIL(list, pageLow->list);
			}
			return list;
		}
	}

	return list;
}

void PageFree(struct Page *page)
{
	page->flags = PAGE_FREE;
}

void PageFreeAll(struct List list)
{
	struct Page *page;
	struct Page *extra;

	LIST_FOREACH_CAN_REMOVE(list, page, extra, struct Page, list) {
		LIST_REMOVE(list, page->list);
		PageFree(page);
	}
}

SECTION_LOW void PageInitLow()
{
	struct Page *pages = (struct Page*)VADDR_TO_PADDR(Pages);
	int i;

	for(i=0; i<N_PAGES; i++) {
		pages[i].flags = PAGE_FREE;
		LIST_ENTRY_CLEAR(pages[i].list);
	}
}