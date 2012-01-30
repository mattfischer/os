#include "Slab.hpp"

#include "Page.hpp"

// Page header, containing allocation bitfield
struct SlabHead {
	unsigned int bitfield[1];
};

/*!
 * \brief Constructor
 * \param size Size of items
 */
SlabBase::SlabBase(int size)
{
	int i;
	int alignedSize;

	// Determine the order of the item size
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

/*!
 * \brief Allocate an item
 * \return Item
 */
void *SlabBase::allocate()
{
	// Search through the list of pages for one with a non-full bitfield
	for(Page *page = mPages.head(); page != NULL; page = mPages.next(page)) {
		struct SlabHead *head = (struct SlabHead*)page->vaddr();

		// Search for an open slot
		for(int i=mDataStart; i<mNumPerPage; i++) {
			int idx = i >> 5;
			int bit = i & 0x1f;
			int val = 1 << bit;

			if((head->bitfield[idx] & val) != 0) {
				continue;
			}

			// Slot found.  Fill it, and return the corresponding item pointer
			head->bitfield[idx] |= val;

			return (char*)page->vaddr() + (i << mOrder);
		}
	}

	// No item found.  Allocate a new page and add it to the list
	Page *page = Page::alloc();
	mPages.addTail(page);

	// Initialize the bitfield in the new page
	struct SlabHead *head = (struct SlabHead*)page->vaddr();
	for(int i=0; i<mBitfieldLen; i++) {
		head->bitfield[i] = 0;
	}

	// Return the first item in the new page
	head->bitfield[0] = 1 << mDataStart;
	return (char*)page->vaddr() + (mDataStart << mOrder);
}

/*!
 * \brief Free an item
 * \param p Item to free
 */
void SlabBase::free(void *p)
{
	Page *page = Page::fromVAddr(p);
	char *addr = (char*)page->vaddr();
	struct SlabHead *head = (struct SlabHead*)addr;
	int i = ((char*)p - addr) >> mOrder;
	int idx = i >> 5;
	int bit = i & 0x1f;
	int val = 1 << bit;

	head->bitfield[idx] &= ~val;

	// Check to see if any other slots on the page are full
	for(int i=0; i<mBitfieldLen; i++) {
		if(head->bitfield[i] != 0) {
			return;
		}
	}

	// All slots are free.  Free the page.
	mPages.remove(page);
	page->free();
}
