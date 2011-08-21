#ifndef SLAB_H
#define SLAB_H

#include "List.h"
#include "Page.h"

class SlabBase {
public:
	SlabBase(int size);

	void *allocate();
	void free(void *p);

private:
	int mOrder;
	int mNumPerPage;
	int mBitfieldLen;
	int mDataStart;
	List<Page> mPages;
};

template<typename T>
class Slab : public SlabBase {
public:
	Slab() : SlabBase(sizeof(T)) {}

	T *allocate() { return (T*)SlabBase::allocate(); }
	void free(T *p) { SlabBase::free(p); }
};

#endif