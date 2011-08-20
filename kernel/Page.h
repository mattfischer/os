#ifndef PAGE_H
#define PAGE_H

#include "List.h"

#define PAGE_FREE 0
#define PAGE_INUSE 1

struct Page {
	unsigned int flags;
	struct ListEntry list;
};

#define KB 1024
#define MB (KB * 1024)

#define PAGE_SIZE (4 * KB)
#define PAGE_SHIFT 12
#define PAGE_MASK 0xfffff000

#define RAM_SIZE (128 * MB)
#define N_PAGES (RAM_SIZE >> PAGE_SHIFT)

#define PADDR_TO_PAGE_NR(paddr) ((paddr) >> PAGE_SHIFT)
#define VADDR_TO_PAGE_NR(vaddr) PADDR_TO_PAGE_NR(VADDR_TO_PADDR(vaddr))

#define PAGE(nr) (Pages + nr)
#define PAGE_NR(page) (page - Pages)

#define PADDR_TO_PAGE(paddr) PAGE(PADDR_TO_PAGE_NR(paddr))
#define VADDR_TO_PAGE(vaddr) PAGE(VADDR_TO_PAGE_NR(vaddr))

#define PAGE_NR_TO_PADDR(nr) ((PAddr)(nr << PAGE_SHIFT))
#define PAGE_NR_TO_VADDR(nr) PADDR_TO_VADDR(PAGE_NR_TO_PADDR(nr))

#define PAGE_TO_PADDR(page) PAGE_NR_TO_PADDR(PAGE_NR(page))
#define PAGE_TO_VADDR(page) PAGE_NR_TO_VADDR(PAGE_NR(page))

#define PAGE_SIZE_ROUND_UP(size) ((size + PAGE_SIZE - 1) & PAGE_MASK)
#define PAGE_ADDR_ROUND_DOWN(addr) ((unsigned)addr & PAGE_MASK)

struct Page *Page_AllocContig(int align, int num);
LIST(struct Page) Page_AllocMulti(int num);
struct Page *Page_Alloc();
void Page_Free(struct Page *page);
void Page_FreeList(LIST(struct Page) list);

void Page_Init();

void Page_InitLow();
struct Page *Page_AllocContigLow(int align, int num);

extern struct Page Pages[N_PAGES];

#endif
