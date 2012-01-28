#include "Elf.h"
#include "MemArea.h"
#include "Util.h"

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
Elf::Entry Elf::load(AddressSpace *space, void *data, int size)
{
	Elf32_Ehdr *hdr = (Elf32_Ehdr*)data;

	// World's stupidest ELF loader.  Loop across program headers and
	// copy each into the address space
	for(int i=0; i<hdr->e_phnum; i++) {
		Elf32_Phdr *phdr = (Elf32_Phdr*)((char*)data + hdr->e_phoff + hdr->e_phentsize * i);
		if(phdr->p_type != PT_LOAD) {
			continue;
		}

		// Add a memory area for each program header
		int aligned = PAGE_ADDR_ROUND_DOWN(phdr->p_vaddr);
		MemArea *area = new MemAreaPages(phdr->p_memsz + phdr->p_vaddr - aligned);
		space->map(area, (void*)phdr->p_vaddr, 0, area->size());

		// Copy the data into the section, and zero-init the space at the end
		memcpy((void*)phdr->p_vaddr, (void*)((char*)data + phdr->p_offset), phdr->p_filesz);
		memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
	}

	return (Entry)hdr->e_entry;
}