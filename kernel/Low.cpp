#include "Kernel.h"
#include "Page.h"
#include "PageTable.h"
#include "Pte.h"

/*!
 * \file
 * \brief Low address functions, called before the MMU is enabled
 *
 * This file is given special annotations in the linker script to place its
 * code into the .low section, which will be linked in the low address range,
 * i.e. the physical addresses for RAM, which is where the kernel is initially
 * loaded.  Since the MMU has not yet been enabled, high addresses mean nothing,
 * so functions in this file can only call other functions in this file.  As such,
 * several pieces of other classes have their methods duplicated here, for use
 * during early kernel init.
 *
 * The primary goal of this code is to construct the initial kernel page table,
 * so that we can enable the MMU and pivot to high addresses.
 */

// Extern in this variable from Kernel.cpp
extern "C" {
	extern PAddr KernelTablePAddr;
}

/*!
 * \brief Basic kernel initialization (Low address)
 */
void Kernel::initLow()
{
	// First, mark as in use all pages occupied by the kernel code/data itself
	Page *pagesLow = (Page*)VADDR_TO_PADDR(Page::fromNumberLow(0));
	for(int i=0; i<Page::fromVAddrLow(__KernelEnd)->numberLow() + 1; i++) {
		pagesLow[i].setFlagsLow(Page::FlagsInUse);
	}

	// Allocate memory for the initial kernel page table (must be 16kb aligned)
	Page *kernelTablePages = Page::allocContigLow(4, 4);

	// Convert the address of KernelTablePAddr to a low address, and save the
	// physical address of the page table into it
	PAddr *kernelTablePAddrLow = (PAddr*)VADDR_TO_PADDR(&KernelTablePAddr);
	*kernelTablePAddrLow = kernelTablePages->paddrLow();

	// Identity-map the first portion of the virtual address space to physical memory
	PAddr paddr = 0;
	for(unsigned vaddr = 0; vaddr < KERNEL_START; vaddr += PageTable::SectionSize) {
		PageTable::mapSectionLow(kernelTablePages, (void*)vaddr, paddr, PageTable::PermissionRWPriv);
		paddr += PageTable::SectionSize;
	}

	// Duplicate the mapping of physical memory into the high address range
	paddr = 0;
	for(unsigned vaddr = KERNEL_START; vaddr > 0; vaddr += PageTable::SectionSize) {
		PageTable::mapSectionLow(kernelTablePages, (void*)vaddr, paddr, PageTable::PermissionRWPriv);
		paddr += PageTable::SectionSize;
	}
}

/*!
 * \brief Map a section-sized range of virtual address space (Low address version)
 * \param pageTable Table to map
 * \param vaddr Virtual address of mapping
 * \param paddr Physical address of mapping
 * \param permission Permission flags
 */
void PageTable::mapSectionLow(Page *pageTable, void *vaddr, PAddr paddr, Permission permission)
{
	unsigned int perm;

	switch(permission) {
		case PermissionNone: perm = PTE_L2_AP_ALL_NONE; break;
		case PermissionRO: perm = PTE_L2_AP_ALL_READ_ONLY; break;
		case PermissionRW: perm = PTE_L2_AP_ALL_READ_WRITE; break;
		case PermissionRWPriv: perm = PTE_L2_AP_ALL_READ_WRITE_PRIV; break;
	}

	unsigned *table = (unsigned*)pageTable->paddrLow();
	int idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	table[idx] = (paddr & PTE_SECTION_BASE_MASK) | perm | PTE_TYPE_SECTION;
}

/*!
 * \brief Retrieve page flags
 * \return Page flags
 */
Page::Flags Page::flagsLow() {
	return mFlags;
}

/*!
 * \brief Set page flags (Low address version)
 * \param flags Page flags
 */
void Page::setFlagsLow(Flags flags) {
	mFlags = flags;
}

/*!
 * \brief Retrieve page number (Low address version)
 * \return Page number
 */
int Page::numberLow() {
	return this - sPages;
}

/*!
 * \brief Retrieve physical address of page (Low address version)
 * \return Physical address of page
 */
PAddr Page::paddrLow() {
	return (PAddr)(numberLow() << PAGE_SHIFT);
}

/*!
 * \brief Initialize the paging system (Low address)
 */
void Page::initLow()
{
	// Convert the page list to a physical address, since we're
	// in low address mode
	Page *pages = (Page*)VADDR_TO_PADDR(sPages);

	// Mark each page in the list as in use
	for(int i=0; i<N_PAGES; i++) {
		pages[i].mFlags = FlagsFree;
	}
}

/*!
 * \brief Allocate contiguous pages (Low address version)
 * \param align Desired alignment
 * \param num Number of pages
 * \return Pointer to the first allocated page
 */
Page *Page::allocContigLow(int align, int num)
{
	// World's stupidest allocator: iterate stepwise through
	// the page list, searching for a free contiguous chunk
	for(int i=0; i<N_PAGES; i += align) {
		int j;

		// See if there are num contiguous free pages
		for(j=0; j<num; j++) {
			Page *page = fromNumberLow(i + j);
			Page *pageLow = (Page*)VADDR_TO_PADDR(page);

			if(pageLow->flagsLow() == FlagsInUse) {
				break;
			}
		}

		// If we found enough free pages, mark them all as used,
		// and return them
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

/*!
 * \brief Convert from page number to page pointer (Low address version)
 * \param n Page number
 * \return Page pointer corresponding to this number
 */
Page *Page::fromNumberLow(int n)
{
	return &sPages[n];
}

/*!
 * \brief Convert from physical address to page pointer (Low address version)
 * \param paddr Physical address
 * \return Page pointer corresponding to this address
 */
Page *Page::fromPAddrLow(PAddr paddr)
{
	return fromNumberLow(paddr >> PAGE_SHIFT);
}

/*!
 * \brief Convert from virtual address to page pointer (Low address version)
 * \param vaddr Virtual address
 * \return Page pointer corresponding to this address
 */
Page *Page::fromVAddrLow(void *vaddr)
{
	return fromPAddrLow(VADDR_TO_PADDR(vaddr));
}

// Use C linkage to avoid name mangling
extern "C" {
	void EntryLow();
}

/*!
 * \brief Entry point from assembly
 */
void EntryLow()
{
	// Get the paging system set up so that we can
	// allocate pages
	Page::initLow();

	// Now, initialize the basic kernel data structures
	Kernel::initLow();
}
