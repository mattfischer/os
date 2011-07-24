#include "Memory.h"
#include "Sched.h"
#include "Defs.h"

SECTION(".entry") void EntryInitKernelMap(unsigned *map)
{
	unsigned int i;

	for(i=0; i<PAGE_TABLE_SIZE; i++) {
		if(i < 3072) {
			map[i] = (i << PTE_SECTION_BASE_SHIFT) | PTE_SECTION_AP_READ_WRITE | PTE_TYPE_SECTION;
		} else {
			map[i]= ((i - 3072) << PTE_SECTION_BASE_SHIFT) | PTE_SECTION_AP_READ_WRITE | PTE_TYPE_SECTION;
		}
	}
}

void EntryHigh()
{
	MemoryInit();
	SchedInit();

	StartStub();
}