#include "Kernel.h"
#include "Page.h"
#include "PageTable.h"
#include "Pte.h"

extern "C" {
	extern PAddr KernelTablePAddr;
}

void Kernel::initLow()
{
	Page *pagesLow = (Page*)VADDR_TO_PADDR(Page::fromNumberLow(0));
	for(int i=0; i<Page::fromVAddrLow(__KernelEnd)->numberLow() + 1; i++) {
		pagesLow[i].setFlagsLow(Page::FlagsInUse);
	}

	Page *kernelTablePages = Page::allocContigLow(4, 4);

	PAddr *kernelTablePAddrLow = (PAddr*)VADDR_TO_PADDR(&KernelTablePAddr);
	*kernelTablePAddrLow = kernelTablePages->paddrLow();

	PAddr paddr = 0;
	for(unsigned vaddr = 0; vaddr < KERNEL_START; vaddr += PageTable::SectionSize) {
		PageTable::mapSectionLow(kernelTablePages, (void*)vaddr, paddr, PageTable::PermissionRWPriv);
		paddr += PageTable::SectionSize;
	}

	paddr = 0;
	for(unsigned vaddr = KERNEL_START; vaddr > 0; vaddr += PageTable::SectionSize, paddr += PageTable::SectionSize) {
		PageTable::mapSectionLow(kernelTablePages, (void*)vaddr, paddr, PageTable::PermissionRWPriv);
		paddr += PageTable::SectionSize;
	}
}

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

Page::Flags Page::flagsLow() {
	return mFlags;
}

void Page::setFlagsLow(Flags flags) {
	mFlags = flags;
}

int Page::numberLow() {
	return this - sPages;
}

PAddr Page::paddrLow() {
	return (PAddr)(numberLow() << PAGE_SHIFT);
}

void Page::initLow()
{
	Page *pages = (Page*)VADDR_TO_PADDR(sPages);

	for(int i=0; i<N_PAGES; i++) {
		pages[i].mFlags = FlagsFree;
	}
}

Page *Page::allocContigLow(int align, int num)
{
	for(int i=0; i<N_PAGES; i += align) {
		int j;

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

Page *Page::fromNumberLow(int n)
{
	return &sPages[n];
}

Page *Page::fromPAddrLow(PAddr paddr)
{
	return fromNumberLow(paddr >> PAGE_SHIFT);
}

Page *Page::fromVAddrLow(void *vaddr)
{
	return fromPAddrLow(VADDR_TO_PADDR(vaddr));
}

extern "C" {
	void EntryLow();
}

void EntryLow()
{
	Page::initLow();
	Kernel::initLow();
}
