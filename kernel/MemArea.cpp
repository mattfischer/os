#include "MemArea.h"

#include "Slab.h"

struct SlabAllocator memAreaSlab;

struct MemArea *MemArea_CreatePages(int size)
{
	struct MemArea *area;

	area = (struct MemArea*)Slab_Allocate(&memAreaSlab);
	area->type = MemAreaTypePages;
	area->size = PAGE_SIZE_ROUND_UP(size);
	area->u.pages = Page_AllocMulti(area->size >> PAGE_SHIFT);

	return area;
}

struct MemArea *MemArea_CreatePhys(int size, PAddr paddr)
{
	struct MemArea *area;

	area = (struct MemArea*)Slab_Allocate(&memAreaSlab);
	area->type = MemAreaTypePhys;
	area->size = PAGE_SIZE_ROUND_UP(size);
	area->u.paddr = paddr;

	return area;
}


void MemArea_Init()
{
	Slab_Init(&memAreaSlab, sizeof(struct MemArea));
}