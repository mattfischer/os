#include "AddressSpace.h"
#include "Util.h"
#include "Kernel.h"
#include "AsmFuncs.h"

Slab<AddressSpace> AddressSpace::sSlab;

static Slab<struct Mapping> mappingSlab;

AddressSpace::AddressSpace(PageTable *pageTable)
{
	if(pageTable == NULL) {
		pageTable = new PageTable(Kernel::process()->addressSpace()->pageTable());
	}

	mPageTable = pageTable;
}

void AddressSpace::map(MemArea *area, void *vaddr, unsigned int offset, unsigned int size)
{
	struct Mapping *mapping = mappingSlab.allocate();
	mapping->vaddr = (void*)PAGE_ADDR_ROUND_DOWN(vaddr);
	mapping->offset = PAGE_ADDR_ROUND_DOWN(offset);
	mapping->size = PAGE_SIZE_ROUND_UP(size + offset - mapping->offset);
	mapping->area = area;

	area->map(mPageTable, vaddr, mapping->offset, mapping->size);

	for(struct Mapping *mappingCursor = mMappings.head(); mappingCursor != NULL; mappingCursor = mMappings.next(mappingCursor)) {
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

		void *srcKernel = (void*)srcPtr;
		void *destKernel = (void*)destPtr;

		if(srcSpace != NULL) {
			srcKernel = PADDR_TO_VADDR(srcSpace->pageTable()->translateVAddr(srcKernel));
		}

		if(destSpace != NULL ) {
			destKernel = PADDR_TO_VADDR(destSpace->pageTable()->translateVAddr(destKernel));
		}

		::memcpy(destKernel, srcKernel, copySize);
		srcPtr += copySize;
		destPtr += copySize;
		size -= copySize;
	}
}
