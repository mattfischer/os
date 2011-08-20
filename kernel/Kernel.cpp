#include "Kernel.h"
#include "AddressSpace.h"
#include "PageTable.h"
#include "Util.h"

#include "Defs.h"

extern "C" {
	PAddr KernelTablePAddr;
}

Process *Kernel::sProcess;

extern char vectorStart[];
extern char vectorEnd[];

SECTION_LOW void Kernel::initLow()
{
	unsigned int vaddr;
	PAddr paddr;
	int i;
	Page *pagesLow;
	unsigned *table;
	int idx;
	unsigned int perm;
	Page *kernelTablePages;
	PAddr *kernelTablePAddrLow;

	pagesLow = (Page*)VADDR_TO_PADDR(Page::fromNumberLow(0));
	for(i=0; i<Page::fromVAddrLow(__KernelEnd)->numberLow() + 1; i++) {
		pagesLow[i].setFlagsLow(Page::FlagsInUse);
	}

	kernelTablePages = Page::allocContigLow(4, 4);

	kernelTablePAddrLow = (PAddr*)VADDR_TO_PADDR(&KernelTablePAddr);
	*kernelTablePAddrLow = kernelTablePages->paddrLow();

	for(vaddr = 0, paddr = 0; vaddr < KERNEL_START; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		PageTable::mapSectionLow(kernelTablePages, (void*)vaddr, paddr, PageTable::PermissionRWPriv);
	}

	for(vaddr = KERNEL_START, paddr = 0; vaddr > 0; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		PageTable::mapSectionLow(kernelTablePages, (void*)vaddr, paddr, PageTable::PermissionRWPriv);
	}
}

void Kernel::init()
{
	Page *pages;
	PageTable *pageTable;
	AddressSpace *addressSpace;
	struct Page *vectorPage;
	char *vector;

	pages = Page::fromPAddr(KernelTablePAddr);
	pageTable = new PageTable(pages);
	addressSpace = new AddressSpace(pageTable);
	sProcess = new Process(addressSpace);

	vectorPage = Page::alloc();
	vector = (char*)vectorPage->vaddr();
	pageTable->mapPage((void*)0xffff0000, vectorPage->paddr(), PageTable::PermissionRWPriv);
	::memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);
}