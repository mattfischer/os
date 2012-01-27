#ifndef SLAB_H
#define SLAB_H

#include "List.h"
#include "Page.h"

/*!
 * \brief Base class for slab allocators.
 *
 * This allows most of the functionality of the allocator to avoid being duplicated
 * for each template instantiation
 */
class SlabBase {
public:
	SlabBase(int size);

	void *allocate();
	void free(void *p);

private:
	int mOrder; //!< Power of two for item size
	int mNumPerPage; //!< Number of items per page
	int mBitfieldLen; //!< Length of bitfield
	int mDataStart; //!< Offset to start of data
	List<Page> mPages; //!< Page list
};

/*!
 * \brief Slab allocator
 *
 * Allocator for fixed-size object, using the page allocator along with a
 * bitfield embedded into the allocated pages to allocate objects in an efficient,
 * fragmentation-free manner.
 */
template<typename T>
class Slab : public SlabBase {
public:
	Slab() : SlabBase(sizeof(T)) {}

	/*!
	 * \brief Allocate an object
	 * \return Allocated object
	 */
	T *allocate() { return (T*)SlabBase::allocate(); }
	/*!
	 * \brief Free an object
	 * \param p Object to free
	 */
	void free(T *p) { SlabBase::free(p); }
};

#endif