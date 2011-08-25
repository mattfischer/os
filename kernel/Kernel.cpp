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
		sObjects[i] = INVALID_OBJECT;
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

		case SyscallSendMessage:
			return SendMessagex(arg0, (struct MessageHeader*)arg1, (struct MessageHeader*)arg2);

		case SyscallReceiveMessage:
			return ReceiveMessagex(arg0, (struct MessageHeader*)arg1);

		case SyscallReadMessage:
			return ReadMessage(arg0, (void*)arg1, (int)arg2, (int)arg3);

		case SyscallReplyMessage:
			return ReplyMessagex(arg0, (int)arg1, (struct MessageHeader*)arg2);

		case SyscallCreateObject:
			return CreateObject();

		case SyscallReleaseObject:
			ReleaseObject(arg0);
			return 0;

		case SyscallGetObject:
			return Sched::current()->process()->dupObjectRef(process(), sObjects[arg0]);

		case SyscallSetObject:
			sObjects[arg0] = process()->dupObjectRef(Sched::current()->process(), arg1);
			return 0;
	}
}