#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#include "List.hpp"
#include "Slab.hpp"
#include "MemArea.hpp"

#include <stddef.h>

class PageTable;

/*!
 * \brief A mapped area in an address space
 */
struct Mapping : public ListEntry {
	void *vaddr; //!< Virtual address
	unsigned int offset; //!< Offset into memory area
	unsigned int size; //!< Size of mapping
	Ref<MemArea> area; //!< Memory area backing this mapping
};

/*!
 * \brief Represents a set of mappings into a virtual address space.
 */
class AddressSpace {
public:
	AddressSpace(PageTable *pageTable = 0);
	~AddressSpace();

	static void init();

	/*!
	 * \brief Page table associated with this address space
	 * \return Page table
	 */
	struct PageTable *pageTable() { return mPageTable; }

	void map(MemArea *area, void *vaddr, unsigned int offset, unsigned int size);
	void expandMap(MemArea *area, unsigned int size);
	MemArea *lookupMap(void *vaddr);

	static void memcpy(AddressSpace *destSpace, void *dest, AddressSpace *srcSpace, void *src, int size);

	//! Allocator
	void *operator new(size_t size) { return sSlab.allocate(); }
	void operator delete(void *p) { sSlab.free((AddressSpace*)p); }

	static AddressSpace *Kernel;

private:
	PageTable *mPageTable; //!< Page table
	List<struct Mapping> mMappings; //!< List of mapped areas

	static Slab<AddressSpace> sSlab;
};
#endif
