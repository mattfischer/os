#include "Kernel.h"
#include "Util.h"
#include "Sched.h"
#include "Message.h"
#include "Object.h"
#include "ProcessManager.h"

// State information used for early bootup in assembly.  Extern C to avoid name mangling
extern "C" {
	char InitStack[256];
	PAddr KernelTablePAddr;
}

//! Kernel process
Process *Kernel::sProcess;
//! List of special kernel objects
int Kernel::sObjects[KernelObjectCount];

// Start of vector area
extern char vectorStart[];
// End of vector area
extern char vectorEnd[];

/*!
 * \brief Initialize kernel state
 */
void Kernel::init()
{
	// Now that we've pivoted to high addresses, lock out the low area of the address space
	PageTable *pageTable = new PageTable(Page::fromPAddr(KernelTablePAddr));
	for(unsigned vaddr = 0; vaddr < KERNEL_START; vaddr += PageTable::SectionSize) {
		pageTable->mapSection((void*)vaddr, 0, PageTable::PermissionNone);
	}

	// Construct the kernel address space and process out of the already-allocated page table
	AddressSpace *addressSpace = new AddressSpace(pageTable);
	sProcess = new Process(addressSpace);

	// Allocate a page to hold the vectors
	Page *vectorPage = Page::alloc();
	char *vector = (char*)vectorPage->vaddr();

	// Map the page to the high vectory area, and copy the vector entries into it
	pageTable->mapPage((void*)0xffff0000, vectorPage->paddr(), PageTable::PermissionRWPriv);
	::memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);

	// Initialize the kernel object array
	for(int i=0; i<KernelObjectCount; i++) {
		sObjects[i] = OBJECT_INVALID;
	}
}

/*!
 * \brief Set a special kernel object
 * \param idx Kernel object index
 * \param obj Object index
 */
void Kernel::setObject(enum KernelObject idx, int obj)
{
	sObjects[idx] = obj;
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
			return Sched::current()->process()->dupObjectRef(process(), sObjects[arg0]);

		case SyscallKernelSetObject:
			sObjects[arg0] = process()->dupObjectRef(Sched::current()->process(), arg1);
			return 0;
	}
}