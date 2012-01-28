#ifndef ELF_H
#define ELF_H

#include "AddressSpace.h"

/*!
 * \brief ELF file loader
 */
class Elf {
public:
	typedef void (*Entry)();
	static Entry load(AddressSpace *space, void *data, int size);
};

#endif