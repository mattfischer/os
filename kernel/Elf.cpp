#include "Elf.h"
#include "Page.h"
#include "Util.h"

ElfEntry Elf_Load(AddressSpace *space, void *data, int size)
{
	Elf32_Ehdr *hdr = (Elf32_Ehdr*)data;
	int i;

	for(i=0; i<hdr->e_phnum; i++) {
		Elf32_Phdr *phdr;
		struct MemArea *area;
		int nPages;

		phdr = (Elf32_Phdr*)((char*)data + hdr->e_phoff + hdr->e_phentsize * i);
		if(phdr->p_type != PT_LOAD) {
			continue;
		}

		int aligned = PAGE_ADDR_ROUND_DOWN(phdr->p_vaddr);
		area = MemArea_CreatePages(phdr->p_memsz + phdr->p_vaddr - aligned);
		space->map(area, (void*)phdr->p_vaddr, 0, area->size);
		memcpy((void*)phdr->p_vaddr, (void*)((char*)data + phdr->p_offset), phdr->p_filesz);
		memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
	}

	return (ElfEntry)hdr->e_entry;
}