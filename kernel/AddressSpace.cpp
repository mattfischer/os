#include "AddressSpace.hpp"

#include "Kernel.hpp"
#include "AsmFuncs.hpp"
#include "PageTable.hpp"
#include "MemArea.hpp"
#include "Process.hpp"

#include <algorithm>
#include <string.h>

//! Slab allocator for address spaces
Slab<AddressSpace> AddressSpace::sSlab;

//! Slab allocator for mappings
static Slab<struct Mapping> mappingSlab;

/*!
 * \brief Constructor
 * \param Page table to use, or 0 to allocate a new one
 */
AddressSpace::AddressSpace(PageTable *pageTable)
{
	if(pageTable == 0) {
		pageTable = new PageTable(Kernel::process()->addressSpace()->pageTable());
	}

	mPageTable = pageTable;
}

AddressSpace::~AddressSpace()
{
	delete mPageTable;

	struct Mapping *next;
	for(struct Mapping *mapping = mMappings.head(); mapping != 0; mapping = next) {
		next = mMappings.next(mapping);
		mappingSlab.free(mapping);
	}
}

/*!
 * \brief Map a memory area into the address space
 * \param area Area to map
 * \param vaddr Virtual address to map
 * \param offset Offset into memory area
 * \param size Size of mapping
 */
void AddressSpace::map(MemArea *area, void *vaddr, unsigned int offset, unsigned int size)
{
	struct Mapping *mapping = mappingSlab.allocate();
	mapping->vaddr = (void*)PAGE_ADDR_ROUND_DOWN(vaddr);
	mapping->offset = PAGE_ADDR_ROUND_DOWN(offset);
	mapping->size = PAGE_SIZE_ROUND_UP(size + offset - mapping->offset);
	mapping->area = area;

	// Map the area into the page table
	area->map(mPageTable, vaddr, mapping->offset, mapping->size);

	// Now add the mapping into the list of mappings, in sorted order
	for(struct Mapping *mappingCursor = mMappings.head(); mappingCursor != 0; mappingCursor = mMappings.next(mappingCursor)) {
		if(mappingCursor->vaddr > mapping->vaddr) {
			mMappings.addAfter(mapping, mappingCursor);
			break;
		}
	}

	// Since mappings have changed, TLB entries must be flushed
	FlushTLB();
}

// Round virtual address up to page boundary
static unsigned nextPageBoundary(unsigned addr)
{
	return (addr & PAGE_MASK) + PAGE_SIZE;
}

/*!
 * \brief Copy data between two address spaces
 * \param destSpace Destination address space, or 0 for kernel address
 * \param dest Destination virtual address
 * \param srcSpace Source address space, or 0 for kernel address
 * \param src Source virtual address
 * \param size Size of data to copy
 */
void AddressSpace::memcpy(AddressSpace *destSpace, void *dest, AddressSpace *srcSpace, void *src, int size)
{
	unsigned srcPtr = (unsigned)src;
	unsigned destPtr = (unsigned)dest;

	// Loop through the copy, breaking on source and destination page boundaries
	while(size > 0) {
		// Compute the size for this portion of the copy
		int srcSize = nextPageBoundary(srcPtr) - srcPtr;
		int destSize = nextPageBoundary(destPtr) - destPtr;
		int copySize = std::min(std::min(srcSize, destSize), size);

		void *srcKernel = (void*)srcPtr;
		void *destKernel = (void*)destPtr;

		// Translate addresses to kernel space
		if(srcSpace != 0) {
			srcKernel = PADDR_TO_VADDR(srcSpace->pageTable()->translateVAddr(srcKernel));
		}

		if(destSpace != 0 ) {
			destKernel = PADDR_TO_VADDR(destSpace->pageTable()->translateVAddr(destKernel));
		}

		// Copy memory
		::memcpy(destKernel, srcKernel, copySize);
		srcPtr += copySize;
		destPtr += copySize;
		size -= copySize;
	}
}
