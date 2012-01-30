#ifndef ELF_H
#define ELF_H

#include "AddressSpace.hpp"

/*!
 * \brief ELF file loader
 */
class Elf {
public:
	typedef void (*Entry)();
	static Entry load(AddressSpace *space, const char *name);
};

#endif