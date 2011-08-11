#include "MemArea.h"

#include "Slab.h"

struct SlabAllocator memAreaSlab;

struct MemArea *MemArea_Create(int size)
{
	struct MemArea *area;

	area = (struct MemArea*)Slab_Allocate(&memAreaSlab);
	area->size = (size + PAGE_SIZE - 1) & PAGE_MASK;
	area->pages = Page_AllocMulti(area->size >> PAGE_SHIFT);

	return area;
}

void MemArea_Init()
{
	Slab_Init(&memAreaSlab, sizeof(struct MemArea));
}