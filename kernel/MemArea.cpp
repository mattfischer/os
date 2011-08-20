#include "MemArea.h"

#include "Slab.h"

struct SlabAllocator<struct MemArea> memAreaSlab;

struct MemArea *MemArea_CreatePages(int size)
{
	struct MemArea *area;

	area = memAreaSlab.Allocate();
	area->type = MemAreaTypePages;
	area->size = PAGE_SIZE_ROUND_UP(size);
	area->u.pages = Page_AllocMulti(area->size >> PAGE_SHIFT);

	return area;
}

struct MemArea *MemArea_CreatePhys(int size, PAddr paddr)
{
	struct MemArea *area;

	area = memAreaSlab.Allocate();
	area->type = MemAreaTypePhys;
	area->size = PAGE_SIZE_ROUND_UP(size);
	area->u.paddr = paddr;

	return area;
}
