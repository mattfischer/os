#include "Elf.hpp"

#include "MemArea.hpp"
#include "Object.hpp"
#include "AddressSpace.hpp"

#include <lib/shared/include/Name.h>
#include <lib/shared/include/IO.h>

#include <string.h>

// Constants and types below are lifted directly from the ELF specification

typedef unsigned int Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Off;
typedef signed int Elf32_Sword;
typedef unsigned int Elf32_Word;

#define EI_NIDENT	16

enum {
	ET_NONE		= 0,
	ET_REL		= 1,
	ET_EXEC		= 2,
	ET_DYN		= 3,
	ET_CORE		= 4,
	ET_LOPROC	= 0xff00,
	ET_HIPROC	= 0xffff
};

typedef struct {
	unsigned char	e_nident[EI_NIDENT];
	Elf32_Half		e_type;
	Elf32_Half		e_machine;
	Elf32_Word		e_version;
	Elf32_Addr		e_entry;
	Elf32_Off		e_phoff;
	Elf32_Off		e_shoff;
	Elf32_Word		e_flags;
	Elf32_Half		e_ehsize;
	Elf32_Half		e_phentsize;
	Elf32_Half		e_phnum;
	Elf32_Half		e_shentsize;
	Elf32_Half		e_shnum;
	Elf32_Half		e_shstrndx;
} Elf32_Ehdr;

enum {
	PT_NULL		= 0,
	PT_LOAD		= 1,
	PT_DYNAMIC	= 2,
	PT_INTERP	= 3,
	PT_NOTE		= 4,
	PT_SHLIB	= 5,
	PT_PHDR		= 6,
	PT_LOPROC	= 0x70000000,
	PT_HIPROC	= 0x7fffffff
};

typedef struct {
	Elf32_Word		p_type;
	Elf32_Off		p_offset;
	Elf32_Addr		p_vaddr;
	Elf32_Addr		p_paddr;
	Elf32_Word		p_filesz;
	Elf32_Word		p_memsz;
	Elf32_Word		p_flags;
	Elf32_Word		p_align;
} Elf32_Phdr;

/*!
 * \brief Load an ELF file into an address space
 * \param space Address space
 * \param data ELF file data
 * \param size Size of data
 */
Elf::Entry Elf::load(AddressSpace *space, const char *name)
{
	Elf32_Ehdr hdr;
	int obj;

	// World's stupidest ELF loader.  Loop across program headers and
	// copy each into the address space
	obj = Name_Open(name);
	File_Read(obj, &hdr, sizeof(hdr));

	Elf32_Phdr phdrs[hdr.e_phnum];
	File_Seek(obj, hdr.e_phoff);
	File_Read(obj, phdrs, sizeof(phdrs));

	for(int i=0; i<hdr.e_phnum; i++) {
		if(phdrs[i].p_type != PT_LOAD) {
			continue;
		}

		// Add a memory area for each program header
		int aligned = PAGE_ADDR_ROUND_DOWN(phdrs[i].p_vaddr);
		MemArea *area = new MemAreaPages(phdrs[i].p_memsz + phdrs[i].p_vaddr - aligned);
		space->map(area, (void*)phdrs[i].p_vaddr, 0, area->size());

		// Copy the data into the section, and zero-init the space at the end
		File_Seek(obj, phdrs[i].p_offset);
		File_Read(obj, (void*)phdrs[i].p_vaddr, phdrs[i].p_filesz);
		memset((void*)(phdrs[i].p_vaddr + phdrs[i].p_filesz), 0, phdrs[i].p_memsz - phdrs[i].p_filesz);
	}

	Object_Release(obj);

	return (Entry)hdr.e_entry;
}
