#ifndef PAGE_H
#define PAGE_H

#include "List.hpp"

#define KB 1024
#define MB (KB * 1024)

//! Size of a page in bytes
#define PAGE_SIZE (4 * KB)

//! Shift amount to form a page size
#define PAGE_SHIFT 12

//! Bitmask to mask off page boundaries
#define PAGE_MASK 0xfffff000

//! Size of physical RAM
#define RAM_SIZE (128 * MB)

//! Number of RAM pages
#define N_PAGES (RAM_SIZE >> PAGE_SHIFT)

//! Type for all physical addresses
typedef unsigned int PAddr;

//! Convert physical address to virtual address
#define PADDR_TO_VADDR(paddr) ((char*)(paddr) + KERNEL_START)

//! Convert virtual address to physical address
#define VADDR_TO_PADDR(vaddr) ((PAddr)(vaddr) - KERNEL_START)

// These symbols are populated by the linker script, and point to
// the beginning and end of the kernel's in-memory image
extern char __KernelStart[];
extern char __KernelEnd[];

#define KERNEL_START (unsigned int)__KernelStart

/*!
 * \brief Represents a page of physical RAM, as well as page allocator
 */
class Page : public ListEntry {
public:
	enum Flags {
		FlagsFree,
		FlagsInUse
	};

	Page() {}

	/*!
	 * \brief Get page flags
	 * \return Page flags
	 */
	Flags flags() { return mFlags; }

	/*!
	 * \brief Set page flags
	 * \param flags New page flags
	 */
	void setFlags(Flags flags) { mFlags = flags; }

	/*!
	 * \brief Return page number
	 * \return Page number
	 */
	int number() { return this - sPages; }

	/*!
	 * \brief Physical address of page
	 * \return Physical address
	 */
	PAddr paddr() { return (PAddr)(number() << PAGE_SHIFT); }

	/*!
	 * \brief Virtual address of page
	 * \return Virtual address
	 */
	void *vaddr() { return PADDR_TO_VADDR(paddr()); }
	void free();

	static Page *allocContig(int align, int num);
	static List<Page> allocMulti(int num);
	static Page *alloc();
	static void freeList(List<Page> list);

	/*!
	 * \brief Retrieve a page pointer from its number
	 * \param n Page number
	 * \return Page pointer
	 */
	static Page *fromNumber(int n) { return &sPages[n]; }

	/*!
	 * \brief Retrieve a page pointer from its physical address
	 * \param paddr Physical address
	 * \return Page pointer
	 */
	static Page *fromPAddr(PAddr paddr) { return fromNumber(paddr >> PAGE_SHIFT); }

	/*!
	 * \brief Retrieve a page pointer from its virtual address
	 * \param vaddr Virtual address
	 * \return Page pointer
	 */
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
	Flags mFlags; //!< Flags

	static Page sPages[N_PAGES];
};

#define PAGE_SIZE_ROUND_UP(size) ((size + PAGE_SIZE - 1) & PAGE_MASK)
#define PAGE_ADDR_ROUND_DOWN(addr) ((unsigned)addr & PAGE_MASK)

#endif
