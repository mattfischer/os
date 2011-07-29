#ifndef SLAB_H
#define SLAB_H

#include "Page.h"

struct SlabAllocator {
	int order;
	int numPerPage;
	int bitfieldLen;
	int dataStart;
	struct List pages;
};

void SlabInit(struct SlabAllocator *slab, int size);
void *SlabAllocate(struct SlabAllocator *slab);
void SlabFree(struct SlabAllocator *slab, void *p);

#endif