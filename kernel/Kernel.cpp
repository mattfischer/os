#include "Kernel.hpp"

#include "Sched.hpp"
#include "Message.hpp"
#include "Object.hpp"
#include "ProcessManager.hpp"
#include "InitFs.hpp"
#include "Pte.hpp"
#include "PageTable.hpp"
#include "AddressSpace.hpp"
#include "Process.hpp"
#include "Task.hpp"

#include <lib/shared/include/Kernel.h>

#include <string.h>

// State information used for early bootup in assembly.  Extern C to avoid name mangling
extern "C" {
	__attribute__((aligned (PAGE_SIZE) )) char InitStack[PAGE_SIZE];
	__attribute__((aligned (PAGE_SIZE) )) char IRQStack[PAGE_SIZE];
	unsigned BuildInitPageTable();
}

__attribute__((aligned (PAGE_TABLE_SIZE * sizeof(unsigned)) )) unsigned initPageTable[PAGE_TABLE_SIZE];

//! Kernel process
Process *Kernel::sProcess;

// Start of vector area
extern char vectorStart[];
// End of vector area
extern char vectorEnd[];

/*!
 * \brief Initialize kernel state
 */
void Kernel::init()
{
	// Mark as in use all pages used by the kernel itself, as well as the initial
	// page table, which is located directly after it in memory.
	Page *pages = Page::fromNumber(0);
	for(int i=0; i<=Page::fromVAddr(&__KernelEnd)->number(); i++) {
		pages[i].setFlags(Page::FlagsInUse);
	}

	// Now that we've pivoted to high addresses, lock out the low area of the address space
	PageTable *pageTable = new PageTable(Page::fromVAddr(initPageTable));
	for(unsigned vaddr = 0; vaddr < KERNEL_START; vaddr += PageTable::SectionSize) {
		pageTable->mapSection((void*)vaddr, 0, PageTable::PermissionNone);
	}

	// Allocate a page to hold the vectors
	Page *vectorPage = Page::alloc();
	char *vector = (char*)vectorPage->vaddr();

	// Map the page to the high vectory area, and copy the vector entries into it
	pageTable->mapPage((void*)0xffff0000, vectorPage->paddr(), PageTable::PermissionRWPriv);
	::memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);

	// Construct the kernel address space and process out of the already-allocated page table
	AddressSpace *addressSpace = new AddressSpace(pageTable);
	sProcess = new Process(addressSpace);

	// Construct a task out of the kernel process and the current stack, and mark it 
	// as the current task.  As of this point, the scheduler is initialized, and we
	// can begin switching tasks.
	Task *task = new Task(sProcess, Page::fromVAddr(InitStack));
	Sched::setCurrent(task);
}

/*!
 * \brief Set a special kernel object
 * \param idx Kernel object index
 * \param obj Object index
 */
void Kernel::setObject(enum KernelObject idx, int obj)
{
	switch(idx) {
		case KernelObjectNameServer:
			InitFs::setNameServer(Sched::current()->process()->object(obj));
			break;
	}
}

int Kernel::getObject(enum KernelObject idx)
{
	Object *obj;

	switch(idx) {
		case KernelObjectProcManager:
			obj = Sched::current()->process()->processObject();
			break;

		case KernelObjectNameServer:
			obj = InitFs::nameServer();
			break;
	}

	return Sched::current()->process()->refObject(obj);
}

/*!
 * \brief Syscall handler
 */
int Kernel::syscall(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
	switch(code) {
		case SyscallYield:
			Sched::runNext();
			return 0;

		case SyscallObjectCreate:
			return Object_Create((unsigned int)arg0, (void*)arg1);

		case SyscallObjectRelease:
			Object_Release(arg0);
			return 0;

		case SyscallObjectSend:
			return Object_Sendx(arg0, (struct MessageHeader*)arg1, (struct MessageHeader*)arg2);

		case SyscallObjectPost:
			Object_Post(arg0, arg1, arg2);
			return 0;

		case SyscallObjectReceive:
			return Object_Receivex(arg0, (struct MessageHeader*)arg1);

		case SyscallMessageRead:
			return Message_Read(arg0, (void*)arg1, (int)arg2, (int)arg3);

		case SyscallMessageReply:
			return Message_Replyx(arg0, (int)arg1, (struct MessageHeader*)arg2);

		case SyscallMessageInfo:
			Message_Info(arg0, (struct MessageInfo*)arg1);
			return 0;

		case SyscallKernelGetObject:
			return Kernel_GetObject((KernelObject)arg0);

		case SyscallKernelSetObject:
			Kernel_SetObject((KernelObject)arg0, (int)arg1);
			return 0;
	}
}

int Kernel_GetObject(enum KernelObject idx)
{
	return Kernel::getObject(idx);
}

void Kernel_SetObject(enum KernelObject idx, int obj)
{
	Kernel::setObject(idx, obj);
}

/*!
 * \brief Set up the initial page table
 *
 * This function is called before the MMU is turned on, so it must be located
 * in the low address range.  This is the purpose of the section attribute.
 * \return Page table physical address
 */
__attribute__ ((section(".low"))) unsigned BuildInitPageTable()
{
	unsigned *table = (unsigned*)(VADDR_TO_PADDR(initPageTable));

	// Identity-map the first portion of the virtual address space to physical memory
	PAddr paddr = 0;
	for(unsigned idx = 0; idx < (KERNEL_START >> PAGE_TABLE_SECTION_SHIFT); idx++) {
		table[idx] = (paddr & PTE_SECTION_BASE_MASK) | PTE_L2_AP_ALL_READ_WRITE_PRIV | PTE_TYPE_SECTION;
		paddr += PageTable::SectionSize;
	}

	// Duplicate the mapping of physical memory into the high address range
	paddr = 0;
	for(unsigned idx = (KERNEL_START >> PAGE_TABLE_SECTION_SHIFT); idx < PAGE_TABLE_SIZE; idx++) {
		table[idx] = (paddr & PTE_SECTION_BASE_MASK) | PTE_L2_AP_ALL_READ_WRITE_PRIV | PTE_TYPE_SECTION;
		paddr += PageTable::SectionSize;
	}

	return VADDR_TO_PADDR(initPageTable);
}
