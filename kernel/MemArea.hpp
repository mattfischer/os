#ifndef MEM_AREA_H
#define MEM_AREA_H

#include "List.hpp"
#include "Slab.hpp"
#include "Page.hpp"
#include "Ref.hpp"

#include <stddef.h>

class PageTable;

/*!
 * \brief Represents a region of physical memory.  Abstract base class.
 */
class MemArea : public RefObject {
public:
	MemArea(int size);

	/*!
	 * \brief Size of area
	 * \return Size
	 */
	int size() { return mSize; }
	void expand(int newSize);

	/*!
	 * \brief Map this area into a page table
	 * \param table Table to map into
	 * \param vaddr Virtual address to map
	 * \param offset Offset within area to map
	 * \param size Size of mapping
	 */
	virtual void map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size) = 0;

protected:
	virtual void doExpand(int size);
	virtual void free() = 0;
	virtual void onLastRef();

private:
	int mSize; //!< Size of area
};

/*!
 * \brief A memory area backed by a set of allocated pages
 */
class MemAreaPages : public MemArea {
public:
	MemAreaPages(int size);
	~MemAreaPages();

	virtual void map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size);

	/*!
	 * \brief Get page list
	 * \return Pages
	 */
	List<Page>& pages() { return mPages; }

	//! Allocator
	void *operator new(size_t size) { return sSlab.allocate(); }
	void operator delete(void *p) { sSlab.free((MemAreaPages*)p); }

protected:
	virtual void doExpand(int newSize);
	virtual void free() { delete this; }

private:
	List<Page> mPages; //!< Page list

	static Slab<MemAreaPages> sSlab;
};

/*!
 * \brief A memory area backed by a range of physical addresses, used for memory-mapped I/O
 */
class MemAreaPhys : public MemArea {
public:
	MemAreaPhys(int size, PAddr paddr);

	virtual void map(PageTable *table, void *vaddr, unsigned int offset, unsigned int size);

	//! Allocator
	void *operator new(size_t size) { return sSlab.allocate(); }
	void operator delete(void *p) { sSlab.free((MemAreaPhys*)p); }

protected:
	virtual void free() { delete this; }

private:
	PAddr mPAddr; //!< Starting physical address of area

	static Slab<MemAreaPhys> sSlab;
};

#endif
