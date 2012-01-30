#include "Page.hpp"

/*!
 * \brief Page list--one entry per page of physical memory
 *
 * This is the largest single data structure in the kernel.  However,
 * the memory expenditure is well worth it, as it allows the kernel
 * to track processes' use of physical memory throughout the system
 */
Page Page::sPages[N_PAGES];

/*!
 * \brief Allocate a page
 * \return Page pointer
 */
Page *Page::alloc()
{
	// World's stupidest page allocator--just scan linearly through
	// the list looking for a free page.
	for(int i=0; i<N_PAGES; i++) {
		Page *page = fromNumber(i);

		if(page->flags() == FlagsFree) {
			// Mark as in use, and return
			page->setFlags(FlagsInUse);
			return page;
		}
	}

	return NULL;
}

/*!
 * \brief Allocate multiple pages
 * \param num Number of pages to allocate
 * \return List of pages
 */
List<Page> Page::allocMulti(int num)
{
	List<Page> list;

	// Even stupider than the single page allocator--scan
	// linearly through the page list multiple times!
	for(int n=0; n < num; n++) {
		Page *page = alloc();
		list.addTail(page);
	}

	return list;
}

/*!
 * \brief Allocate contiguous pages
 * \param align Desired alignment
 * \param num Number of pages
 * \return Pointer to the first allocated page
 */
Page *Page::allocContig(int align, int num)
{
	// Another stupid page allocator--scan stepwise
	// through the page list, looking for a set of
	// contiguous free pages
	for(int i=0; i<N_PAGES; i += align) {
		int j;
		for(j=0; j<num; j++) {
			Page *page = fromNumber(i + j);

			if(page->flags() == FlagsInUse) {
				break;
			}
		}

		// If enough pages were found, mark as in use and return
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

/*!
 * \brief Free page
 */
void Page::free()
{
	mFlags = FlagsFree;
}

/*!
 * \brief Free a list of pages
 * \param list List of pages
 */
void Page::freeList(List<Page> list)
{
	Page *next;
	for(Page *page = list.head(); page != NULL; page = next)
	{
		next = list.next(page);
		list.remove(page);
		page->free();
	}
}
