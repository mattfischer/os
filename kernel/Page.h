#ifndef PAGE_H
#define PAGE_H

#include "List.h"

#define KB 1024
#define MB (KB * 1024)

#define PAGE_SIZE (4 * KB)
#define PAGE_SHIFT 12
#define PAGE_MASK 0xfffff000

#define RAM_SIZE (128 * MB)
#define N_PAGES (RAM_SIZE >> PAGE_SHIFT)

typedef unsigned int PAddr;

#define PADDR_TO_VADDR(paddr) ((char*)(paddr) + KERNEL_START)
#define VADDR_TO_PADDR(vaddr) ((PAddr)(vaddr) - KERNEL_START)

extern char __KernelStart[];
extern char __KernelEnd[];

#define KERNEL_START (unsigned int)__KernelStart

class Page {
public:
	enum Flags {
		FlagsFree,
		FlagsInUse
	};

	Page() {}

	Flags flags() { return mFlags; }
	void setFlags(Flags flags) { mFlags = flags; }

	int number() { return this - sPages; }
	PAddr paddr() { return (PAddr)(number() << PAGE_SHIFT); }
	void *vaddr() { return PADDR_TO_VADDR(paddr()); }
	void free();

	ListEntry2<Page> list;

	static Page *allocContig(int align, int num);
	static List2<Page, &Page::list> allocMulti(int num);
	static Page *alloc();
	static void freeList(List2<Page, &Page::list> list);

	static Page *fromNumber(int n) { return &sPages[n]; }
	static Page *fromPAddr(PAddr paddr) { return fromNumber(paddr >> PAGE_SHIFT); }
	static Page *fromVAddr(void *vaddr) { return fromPAddr(VADDR_TO_PADDR(vaddr)); }

	static void init();

	Flags flagsLow();
	void setFlagsLow(Flags flags);
	int numberLow();
	PAddr paddrLow();
	static Page *allocContigLow(int align, int num);
	static void initLow();
	static Page *fromNumberLow(int n);
	static Page *fromPAddrLow(PAddr paddr);
	static Page *fromVAddrLow(void *vaddr);

private:
	Flags mFlags;

	static Page sPages[N_PAGES];
};

#define PAGE_SIZE_ROUND_UP(size) ((size + PAGE_SIZE - 1) & PAGE_MASK)
#define PAGE_ADDR_ROUND_DOWN(addr) ((unsigned)addr & PAGE_MASK)

#endif
