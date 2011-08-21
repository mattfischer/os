#include "Page.h"

Page Page::sPages[N_PAGES];

Page *Page::alloc()
{
	int i;

	for(i=0; i<N_PAGES; i++) {
		Page *page = fromNumber(i);

		if(page->flags() == FlagsFree) {
			page->setFlags(FlagsInUse);
			return page;
		}
	}

	return NULL;
}

List<Page> Page::allocMulti(int num)
{
	List<Page> list;
	int n;

	for(n=0; n < num; n++) {
		Page *page = alloc();
		list.addTail(page);
	}

	return list;
}

Page *Page::allocContig(int align, int num)
{
	int i, j;

	for(i=0; i<N_PAGES; i += align) {
		for(j=0; j<num; j++) {
			Page *page = fromNumber(i + j);

			if(page->flags() == FlagsInUse) {
				break;
			}
		}

		if(j == num) {
			for(j=0; j<num; j++) {
				Page *page = fromNumber(i + j);

				page->setFlags(FlagsInUse);
			}
			return fromNumber(i);
		}
	}

	return NULL;
}

void Page::free()
{
	mFlags = FlagsFree;
}

void Page::freeList(List<Page> list)
{
	Page *page;
	Page *next;

	for(page = list.head(); page != NULL; page = next)
	{
		next = list.next(page);
		list.remove(page);
		page->free();
	}
}
