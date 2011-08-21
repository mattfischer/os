#include "PageTable.h"

#include "Util.h"
#include "Pte.h"

struct Slab<PageTable> PageTable::sSlab;

PageTable::PageTable(PageTable *copy)
{
	mPages = Page::allocContig(4, 4);
	mTablePAddr = mPages->paddr();

	unsigned *base = (unsigned*)PADDR_TO_VADDR(mTablePAddr);
	unsigned *copyBase = (unsigned*)PADDR_TO_VADDR(copy->mTablePAddr);

	memcpy(base, copyBase, PAGE_TABLE_SIZE * sizeof(unsigned));
}

PageTable::PageTable(Page *pages)
{
	mPages = pages;
	mTablePAddr = mPages->paddr();
}

void PageTable::allocL2Table(void *vaddr)
{
	unsigned *table = (unsigned*)PADDR_TO_VADDR(mTablePAddr);
	int idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;

	for(Page *L2Page = mL2Tables.head(); L2Page != NULL; L2Page = mL2Tables.next(L2Page)) {
		unsigned *L2Table = (unsigned*)L2Page->vaddr();
		for(int i=0; i<4; i++) {
			int l2idx = i*PAGE_L2_TABLE_SIZE;
			unsigned l2pte = L2Table[l2idx];
			if((l2pte & PTE_L2_TYPE_MASK) != PTE_L2_TYPE_DISABLED || (l2pte & 0x80000000) == 0) {
				continue;
			}

			for(int j=0; j<PAGE_L2_TABLE_SIZE; j++) {
				L2Table[l2idx + j] = 0;
			}
			table[idx] = VADDR_TO_PADDR(L2Table + l2idx) | PTE_SECTION_AP_READ_WRITE | PTE_TYPE_COARSE;
			return;
		}
	}

	Page *L2Page = Page::alloc();
	unsigned *L2Table = (unsigned*)L2Page->vaddr();
	mL2Tables.addTail(L2Page);

	memset(L2Table, 0, PAGE_L2_TABLE_SIZE * sizeof(unsigned));
	for(int i=1; i<4; i++) {
		L2Table[i*PAGE_L2_TABLE_SIZE] = 0x80000000;
	}

	table[idx] = VADDR_TO_PADDR(L2Table) | PTE_TYPE_COARSE;
}

void PageTable::mapPage(void *vaddr, PAddr paddr, Permission permission)
{
	unsigned *table = (unsigned*)PADDR_TO_VADDR(mTablePAddr);
	int idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	unsigned pte = table[idx];

	if((pte & PTE_TYPE_MASK) == PTE_TYPE_SECTION ||
	   (pte & PTE_TYPE_MASK) == PTE_TYPE_DISABLED) {
	   allocL2Table(vaddr);
	}

	pte = table[idx];

	unsigned int perm;
	switch(permission) {
		case PermissionNone: perm = PTE_L2_AP_ALL_NONE; break;
		case PermissionRO: perm = PTE_L2_AP_ALL_READ_ONLY; break;
		case PermissionRW: perm = PTE_L2_AP_ALL_READ_WRITE; break;
		case PermissionRWPriv: perm = PTE_L2_AP_ALL_READ_WRITE_PRIV; break;
	}

	if((pte & PTE_TYPE_MASK) == PTE_TYPE_COARSE) {
		unsigned *L2Table = (unsigned*)PADDR_TO_VADDR(pte & PTE_COARSE_BASE_MASK);
		int l2idx = ((unsigned)vaddr & (~PAGE_TABLE_SECTION_MASK)) >> PAGE_SHIFT;
		L2Table[l2idx] = (paddr & PTE_COARSE_BASE_MASK) | perm | PTE_L2_TYPE_SMALL;
	}
}

void PageTable::mapSection(void *vaddr, PAddr paddr, Permission permission)
{
	unsigned int perm;

	switch(permission) {
		case PermissionNone: perm = PTE_L2_AP_ALL_NONE; break;
		case PermissionRO: perm = PTE_L2_AP_ALL_READ_ONLY; break;
		case PermissionRW: perm = PTE_L2_AP_ALL_READ_WRITE; break;
		case PermissionRWPriv: perm = PTE_L2_AP_ALL_READ_WRITE_PRIV; break;
	}

	unsigned *table = (unsigned*)PADDR_TO_VADDR(mTablePAddr);
	unsigned int idx = (unsigned int)vaddr >> PAGE_TABLE_SECTION_SHIFT;
	table[idx] = (paddr & PTE_SECTION_BASE_MASK) | perm | PTE_TYPE_SECTION;
}

PAddr PageTable::translateVAddr(void *addr)
{
	unsigned *table = (unsigned*)PADDR_TO_VADDR(mTablePAddr);
	int idx = (unsigned)addr >> PAGE_TABLE_SECTION_SHIFT;
	unsigned pte = table[idx];

	if((pte & PTE_TYPE_MASK) == PTE_TYPE_SECTION) {
		return (pte & PTE_SECTION_BASE_MASK) | ((unsigned)addr & (~PTE_SECTION_BASE_MASK));
	}

	unsigned *L2Table = (unsigned*)PADDR_TO_VADDR(pte & PTE_COARSE_BASE_MASK);
	int l2idx = ((unsigned)addr & (~PAGE_TABLE_SECTION_MASK)) >> PAGE_SHIFT;
	return (L2Table[l2idx] & PTE_L2_BASE_MASK) | ((unsigned)addr & (~PTE_L2_BASE_MASK));
}
