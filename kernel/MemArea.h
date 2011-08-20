#ifndef MEM_AREA_H
#define MEM_AREA_H

#include "List.h"
#include "Page.h"
#include "PageTable.h"

enum MemAreaType {
	MemAreaTypePages,
	MemAreaTypePhys
};

struct MemArea {
	enum MemAreaType type;
	int size;

	union {
		LIST(struct Page) pages;
		PAddr paddr;
	} u;
};

struct MemArea *MemArea_CreatePages(int size);
struct MemArea *MemArea_CreatePhys(int size, PAddr paddr);

#endif