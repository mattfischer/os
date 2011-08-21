#ifndef SLAB_H
#define SLAB_H

#include "List.h"
#include "Page.h"

class SlabBase {
public:
	SlabBase(int size);

	void *allocateBase();
	void freeBase(void *p);

private:
	int mOrder;
	int mNumPerPage;
	int mBitfieldLen;
	int mDataStart;
	List<Page> mPages;
};

template<typename T>
class SlabAllocator : public SlabBase {
public:
	SlabAllocator() : SlabBase(sizeof(T)) {}

	T *allocate() { return (T*)allocateBase(); }
	void free(T *p) { freeBase(p); }
};

#endif