#include "Page.h"
#include "AddressSpace.h"
#include "Defs.h"

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

List<Page, &Page::list> Page::allocMulti(int num)
{
	List<Page, &Page::list> list;
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

void Page::freeList(List<Page, &Page::list> list)
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

SECTION_LOW Page::Flags Page::flagsLow() {
	return mFlags;
}

SECTION_LOW void Page::setFlagsLow(Flags flags) {
	mFlags = flags;
}

SECTION_LOW int Page::numberLow() {
	return this - sPages;
}

SECTION_LOW PAddr Page::paddrLow() {
	return (PAddr)(numberLow() << PAGE_SHIFT);
}

SECTION_LOW void Page::initLow()
{
	Page *pages = (Page*)VADDR_TO_PADDR(sPages);
	int i;

	for(i=0; i<N_PAGES; i++) {
		pages[i].mFlags = FlagsFree;
	}
}

SECTION_LOW Page *Page::allocContigLow(int align, int num)
{
	int i, j;

	for(i=0; i<N_PAGES; i += align) {
		for(j=0; j<num; j++) {
			Page *page = fromNumberLow(i + j);
			Page *pageLow = (Page*)VADDR_TO_PADDR(page);

			if(pageLow->flagsLow() == FlagsInUse) {
				break;
			}
		}

		if(j == num) {
			for(j=0; j<num; j++) {
				Page *page = fromNumberLow(i + j);
				Page *pageLow = (Page*)VADDR_TO_PADDR(page);

				pageLow->setFlagsLow(FlagsInUse);
			}
			return fromNumberLow(i);
		}
	}

	return NULL;
}

SECTION_LOW Page *Page::fromNumberLow(int n)
{
	return &sPages[n];
}

SECTION_LOW Page *Page::fromPAddrLow(PAddr paddr)
{
	return fromNumberLow(paddr >> PAGE_SHIFT);
}

SECTION_LOW Page *Page::fromVAddrLow(void *vaddr)
{
	return fromPAddrLow(VADDR_TO_PADDR(vaddr));
}
