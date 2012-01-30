#include "Kernel.hpp"

#include "Util.hpp"
#include "Sched.hpp"
#include "Message.hpp"
#include "Object.hpp"
#include "ProcessManager.hpp"
#include "InitFs.hpp"

#include <lib/shared/include/Kernel.h>

// State information used for early bootup in assembly.  Extern C to avoid name mangling
extern "C" {
	char InitStack[256];
	PAddr KernelTablePAddr;
	void Yield();
}

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
			Yield();
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

void Yield()
{
	Sched::runNext();
}