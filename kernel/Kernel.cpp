#include "Kernel.h"
#include "Util.h"
#include "Sched.h"
#include "Message.h"
#include "Object.h"
#include "ProcessManager.h"

extern "C" {
	char InitStack[256];
	PAddr KernelTablePAddr;
}

Process *Kernel::sProcess;
int Kernel::sObjects[KernelObjectCount];

extern char vectorStart[];
extern char vectorEnd[];

void Kernel::init()
{
	PageTable *pageTable = new PageTable(Page::fromPAddr(KernelTablePAddr));
	for(unsigned vaddr = 0; vaddr < KERNEL_START; vaddr += PageTable::SectionSize) {
		pageTable->mapSection((void*)vaddr, 0, PageTable::PermissionNone);
	}

	AddressSpace *addressSpace = new AddressSpace(pageTable);
	sProcess = new Process(addressSpace);

	Page *vectorPage = Page::alloc();
	char *vector = (char*)vectorPage->vaddr();
	pageTable->mapPage((void*)0xffff0000, vectorPage->paddr(), PageTable::PermissionRWPriv);
	::memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);

	for(int i=0; i<KernelObjectCount; i++) {
		sObjects[i] = OBJECT_INVALID;
	}
}

void Kernel::setObject(enum KernelObject idx, int obj)
{
	sObjects[idx] = obj;
}

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