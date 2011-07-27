#include "Elf.h"
#include "Util.h"

void *ElfLoad(struct AddressSpace *space, void *data, int size)
{
	Elf32_Ehdr *hdr = (Elf32_Ehdr*)data;
	int i;

	for(i=0; i<hdr->e_phnum; i++) {
		Elf32_Phdr *phdr;
		struct Page *pages;
		int nPages;

		phdr = (Elf32_Phdr*)((char*)data + hdr->e_phoff + hdr->e_phentsize * i);
		if(phdr->p_type != PT_LOAD) {
			continue;
		}

		nPages = (phdr->p_memsz + PAGE_SIZE - 1) >> PAGE_SHIFT;
		pages = PageAlloc(nPages);
		MapPages(space, (void*)phdr->p_vaddr, pages);
		memcpy((void*)phdr->p_vaddr, (void*)((char*)data + phdr->p_offset), phdr->p_filesz);
		memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
	}

	return (void*)hdr->e_entry;
}