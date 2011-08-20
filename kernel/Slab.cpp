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
	LIST_INIT(mPages);
}

void *SlabBase::allocateBase()
{
	struct Page *page;
	struct SlabHead *head;
	int i, j;

	LIST_FOREACH(mPages, page, struct Page, list) {
		head = (struct SlabHead*)PAGE_TO_VADDR(page);

		for(i=mDataStart; i<mNumPerPage; i++) {
			int idx = i >> 5;
			int bit = i & 0x1f;
			int val = 1 << bit;

			if((head->bitfield[idx] & val) != 0) {
				continue;
			}

			head->bitfield[idx] |= val;

			return PAGE_TO_VADDR(page) + (i << mOrder);
		}
	}

	page = Page_Alloc();
	LIST_ADD_TAIL(mPages, page->list);

	head = (struct SlabHead*)PAGE_TO_VADDR(page);
	for(i=0; i<mBitfieldLen; i++) {
		head->bitfield[i] = 0;
	}
	head->bitfield[0] = 1 << mDataStart;
	return PAGE_TO_VADDR(page) + (mDataStart << mOrder);
}

void SlabBase::freeBase(void *p)
{
	struct Page *page = VADDR_TO_PAGE(p);
	struct Page *cursor;
	struct Page *prev;
	char *addr = PAGE_TO_VADDR(page);
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

	LIST_REMOVE(mPages, page->list);
	Page_Free(page);
}
