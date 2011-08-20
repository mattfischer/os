#include "AddressSpace.h"
#include "AsmFuncs.h"
#include "Util.h"

AddressSpace *AddressSpace::Kernel;
SlabAllocator<struct AddressSpace> AddressSpace::sSlab;

static struct SlabAllocator<struct Mapping> mappingSlab;

extern char vectorStart[];
extern char vectorEnd[];

AddressSpace::AddressSpace(struct PageTable *pageTable)
{
	if(pageTable == NULL) {
		pageTable = new PageTable();
	}

	mPageTable = pageTable;
}

void AddressSpace::map(MemArea *area, void *vaddr, unsigned int offset, unsigned int size)
{
	struct Mapping *mapping;
	struct Mapping *mappingCursor;
	unsigned v;

	mapping = mappingSlab.allocate();
	mapping->vaddr = (void*)PAGE_ADDR_ROUND_DOWN(vaddr);
	mapping->offset = PAGE_ADDR_ROUND_DOWN(offset);
	mapping->size = PAGE_SIZE_ROUND_UP(size + offset - mapping->offset);
	mapping->area = area;

	area->map(mPageTable, vaddr, mapping->offset, mapping->size);

	for(mappingCursor = mMappings.head(); mappingCursor != NULL; mappingCursor = mMappings.next(mappingCursor)) {
		if(mappingCursor->vaddr > mapping->vaddr) {
			mMappings.addAfter(mapping, mappingCursor);
			break;
		}
	}

	FlushTLB();
}

static unsigned nextPageBoundary(unsigned addr)
{
	return (addr & PAGE_MASK) + PAGE_SIZE;
}

void AddressSpace::memcpy(AddressSpace *destSpace, void *dest, AddressSpace *srcSpace, void *src, int size)
{
	unsigned srcPtr = (unsigned)src;
	unsigned destPtr = (unsigned)dest;

	while(size > 0) {
		int srcSize = nextPageBoundary(srcPtr) - srcPtr;
		int destSize = nextPageBoundary(destPtr) - destPtr;
		int copySize = min(min(srcSize, destSize), size);

		void *srcKernel;
		void *destKernel;

		if(srcSpace == NULL) {
			srcKernel = (void*)srcPtr;
		} else {
			srcKernel = PADDR_TO_VADDR(srcSpace->pageTable()->translateVAddr((void*)srcPtr));
		}

		if(destSpace == NULL ) {
			destKernel = (void*)destPtr;
		} else {
			destKernel = PADDR_TO_VADDR(destSpace->pageTable()->translateVAddr((void*)destPtr));
		}

		::memcpy(destKernel, srcKernel, copySize);
		srcPtr += copySize;
		destPtr += copySize;
		size -= copySize;
	}
}

void AddressSpace::init()
{
	int i;
	unsigned int n;
	struct Page *vectorPage;
	char *vector;

	Kernel = new AddressSpace(PageTable::Kernel);

	vectorPage = Page_Alloc();
	vector = PAGE_TO_VADDR(vectorPage);
	Kernel->pageTable()->mapPage((void*)0xffff0000, PAGE_TO_PADDR(vectorPage), PageTable::PermissionRWPriv);
	::memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);
}
