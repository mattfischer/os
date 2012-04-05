#ifndef ELF_H
#define ELF_H

class AddressSpace;

/*!
 * \brief ELF file loader
 */
class Elf {
public:
	typedef void (*Entry)();
	static Entry load(AddressSpace *space, const char *name);
};

#endif
