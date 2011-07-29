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

void Slab_Init(struct SlabAllocator *slab, int size);
void *Slab_Allocate(struct SlabAllocator *slab);
void Slab_Free(struct SlabAllocator *slab, void *p);

#endif