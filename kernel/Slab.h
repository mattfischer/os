#ifndef SLAB_H
#define SLAB_H

#include "Page.h"

class SlabBase {
public:
	SlabBase(int size);

	void *AllocateBase();
	void FreeBase(void *p);

private:
	int mOrder;
	int mNumPerPage;
	int mBitfieldLen;
	int mDataStart;
	LIST(struct Page) mPages;
};

template<typename T>
class SlabAllocator : public SlabBase {
public:
	SlabAllocator() : SlabBase(sizeof(T)) {}

	T *Allocate() { return (T*)AllocateBase(); }
	void Free(T *p) { FreeBase(p); }
};

#endif