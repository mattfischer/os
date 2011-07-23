#ifndef MEMORY_H
#define MEMORY_H

#include "PageTable.h"

#define PAGE_FREE 0
#define PAGE_INUSE 1

struct Page {
	unsigned int flags;
	struct Page *next;
};

#define N_PAGES (1024 * 1024)

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

extern char __KernelStart[];
extern char __KernelEnd[];

#define PADDR_TO_VADDR(paddr) (__KernelStart + (char*)paddr)
#define VADDR_TO_PADDR(vaddr) ((char*)vaddr - __KernelStart)

#define PADDR_TO_PAGE_NR(paddr) (paddr >> PAGE_SHIFT)
#define VADDR_TO_PAGE_NR(vaddr) (VADDR_TO_PADDR(vaddr) >> PAGE_SHIFT)

#define PAGE(nr) (Pages + nr)
#define PAGE_NR(page) (page - Pages)

#define PADDR_TO_PAGE(paddr) PAGE(PADDR_TO_PAGE_NR(paddr))
#define VADDR_TO_PAGE(paddr) PAGE(VADDR_TO_PAGE_NR(vaddr))

#define PAGE_NR_TO_PADDR(nr) ((char*)(nr << PAGE_SHIFT))
#define PAGE_NR_TO_VADDR(nr) PADDR_TO_VADDR(PAGE_NR_TO_PADDR(nr))

#define PAGE_TO_PADDR(page) PAGE_NR_TO_PADDR(PAGE_NR(page))
#define PAGE_TO_VADDR(page) PAGE_NR_TO_VADDR(PAGE_NR(page))

struct Page *PageAllocContig(int align, int num);
struct Page *PageAlloc(int num);

void MemoryInit();

extern struct Page Pages[N_PAGES];
extern unsigned InitMap[PAGE_TABLE_SIZE];

#endif