#include "Slab.h"
#include "AddressSpace.h"
#include "Page.h"
#include "Defs.h"

struct SlabHead {
	unsigned int bitfield[1];
};

SlabBase::SlabBase(int size)
{
	int i;
	int alignedSize;

	for(i=0; i<32; i++) {
		alignedSize = 1 << i;
		if(alignedSize >= size) {
			break;
		}
	}

	mOrder = i;
	mNumPerPage = PAGE_SIZE >> mOrder;
	mBitfieldLen = (mNumPerPage + 31) >> 5;
	mDataStart = (mBitfieldLen * 4 + alignedSize - 1) >> mOrder;
}

void *SlabBase::allocateBase()
{
	Page *page;
	struct SlabHead *head;
	int i, j;

	for(page = mPages.head(); page != NULL; page = mPages.next(page)) {
		head = (struct SlabHead*)page->vaddr();

		for(i=mDataStart; i<mNumPerPage; i++) {
			int idx = i >> 5;
			int bit = i & 0x1f;
			int val = 1 << bit;

			if((head->bitfield[idx] & val) != 0) {
				continue;
			}

			head->bitfield[idx] |= val;

			return (char*)page->vaddr() + (i << mOrder);
		}
	}

	page = Page::alloc();
	mPages.addTail(page);

	head = (struct SlabHead*)page->vaddr();
	for(i=0; i<mBitfieldLen; i++) {
		head->bitfield[i] = 0;
	}
	head->bitfield[0] = 1 << mDataStart;
	return (char*)page->vaddr() + (mDataStart << mOrder);
}

void SlabBase::freeBase(void *p)
{
	Page *page = Page::fromVAddr(p);
	Page *cursor;
	Page *prev;
	char *addr = (char*)page->vaddr();
	struct SlabHead *head = (struct SlabHead*)addr;
	int i = ((char*)p - addr) >> mOrder;
	int idx = i >> 5;
	int bit = i & 0x1f;
	int val = 1 << bit;

	head->bitfield[idx] &= ~val;

	for(i=0; i<mBitfieldLen; i++) {
		if(head->bitfield[i] != 0) {
			return;
		}
	}

	mPages.remove(page);
	page->free();
}
