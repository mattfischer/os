#include "Kernel.h"
#include "Page.h"
#include "PageTable.h"
#include "Pte.h"

extern "C" {
	extern PAddr KernelTablePAddr;
}

void Kernel::initLow()
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

	for(vaddr = 0, paddr = 0; vaddr < KERNEL_START; vaddr += PageTable::SectionSize, paddr += PageTable::SectionSize) {
		PageTable::mapSectionLow(kernelTablePages, (void*)vaddr, paddr, PageTable::PermissionRWPriv);
	}

	for(vaddr = KERNEL_START, paddr = 0; vaddr > 0; vaddr += PageTable::SectionSize, paddr += PageTable::SectionSize) {
		PageTable::mapSectionLow(kernelTablePages, (void*)vaddr, paddr, PageTable::PermissionRWPriv);
	}
}

void PageTable::mapSectionLow(Page *pageTable, void *vaddr, PAddr paddr, Permission permission)
{
	unsigned *table;
	int idx;
	unsigned int perm;

	switch(permission) {
		case PermissionNone: perm = PTE_L2_AP_ALL_NONE; break;
		case PermissionRO: perm = PTE_L2_AP_ALL_READ_ONLY; break;
		case PermissionRW: perm = PTE_L2_AP_ALL_READ_WRITE; break;
		case PermissionRWPriv: perm = PTE_L2_AP_ALL_READ_WRITE_PRIV; break;
	}

	table = (unsigned*)pageTable->paddrLow();
	idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
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
	int i;

	for(i=0; i<N_PAGES; i++) {
		pages[i].mFlags = FlagsFree;
	}
}

Page *Page::allocContigLow(int align, int num)
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
