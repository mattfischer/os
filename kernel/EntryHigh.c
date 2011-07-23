#include "Memory.h"
#include "Defs.h"

SECTION(".entry") void EntrySetupInitMap(unsigned *map)
{
	unsigned int i;
	
	for(i=0; i<PAGE_TABLE_SIZE; i++) {
		if(i < 3072) {
			map[i] = (i << PTE_BASE_SHIFT) | PTE_AP_READ_WRITE | PTE_TYPE_SECTION;
		} else {
			map[i]= ((i - 3072) << PTE_BASE_SHIFT) | PTE_AP_READ_WRITE | PTE_TYPE_SECTION;
		}
	}
}

void EntryHigh()
{
	MemoryInit();
	StartStub();
}