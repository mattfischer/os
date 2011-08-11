#ifndef MEM_AREA_H
#define MEM_AREA_H

#include "List.h"
#include "Page.h"

struct MemArea {
	int size;
	LIST(struct Page) pages;
};

struct MemArea *MemArea_Create(int size);
void MemArea_Init();

#endif