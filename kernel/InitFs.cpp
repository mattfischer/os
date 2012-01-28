#include "InitFs.h"
#include "Util.h"

#include <kernel/include/InitFsFmt.h>

// These symbols are populated by the linker script, and point
// to the beginning and end of the initfs data
extern char __InitFsStart[];
extern char __InitFsEnd[];

/*!
 * \brief Look up a file from the initfs
 * \param name Name of file
 * \param size Location to write file size to
 * \return Starting address of file
 */
void *InitFs_Lookup(const char *name, int *size)
{
	// Search through the initfs searching for a matching file
	struct InitFsFileHeader *header = (struct InitFsFileHeader*)__InitFsStart;
	while((void*)header < (void*)__InitFsEnd) {
		if(!strcmp(header->name, name)) {
			// Filename matches
			if(size) {
				*size = header->size;
			}
			return (char*)header + sizeof(struct InitFsFileHeader);
		}

		// Skip to the next file record
		header = (struct InitFsFileHeader*)((char*)header + sizeof(struct InitFsFileHeader) + header->size);
	}

	return NULL;
}