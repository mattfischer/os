#include "InitFs.h"
#include "Util.h"

#include <kernel/include/InitFsFmt.h>

extern char __InitFsStart[];
extern char __InitFsEnd[];
void *InitFs_Lookup(const char *name, int *size)
{
	struct InitFsFileHeader *header;

	header = (struct InitFsFileHeader*)__InitFsStart;
	while((void*)header < (void*)__InitFsEnd) {
		if(!strcmp(header->name, name)) {
			if(size) {
				*size = header->size;
			}
			return (char*)header + sizeof(struct InitFsFileHeader);
		}

		header = (struct InitFsFileHeader*)((char*)header + sizeof(struct InitFsFileHeader) + header->size);
	}

	return NULL;
}