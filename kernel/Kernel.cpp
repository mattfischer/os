#include "Kernel.h"
#include "Util.h"
#include "Sched.h"
#include "Message.h"
#include "Object.h"
#include "ProcessManager.h"

#include "Defs.h"

extern "C" {
	PAddr KernelTablePAddr;
}

Process *Kernel::sProcess;

extern char vectorStart[];
extern char vectorEnd[];

SECTION_LOW void Kernel::initLow()
{
	unsigned int vaddr;
	PAddr paddr;
	int i;
	Page *pagesLow;
	unsigned *table;
	int idx;
	unsigned int perm;
	Page *kernelTablePages;
	PAddr *kernelTablePAddrLow;

	pagesLow = (Page*)VADDR_TO_PADDR(Page::fromNumberLow(0));
	for(i=0; i<Page::fromVAddrLow(__KernelEnd)->numberLow() + 1; i++) {
		pagesLow[i].setFlagsLow(Page::FlagsInUse);
	}

	kernelTablePages = Page::allocContigLow(4, 4);

	kernelTablePAddrLow = (PAddr*)VADDR_TO_PADDR(&KernelTablePAddr);
	*kernelTablePAddrLow = kernelTablePages->paddrLow();

	for(vaddr = 0, paddr = 0; vaddr < KERNEL_START; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		PageTable::mapSectionLow(kernelTablePages, (void*)vaddr, paddr, PageTable::PermissionRWPriv);
	}

	for(vaddr = KERNEL_START, paddr = 0; vaddr > 0; vaddr += PAGE_TABLE_SECTION_SIZE, paddr += PAGE_TABLE_SECTION_SIZE) {
		PageTable::mapSectionLow(kernelTablePages, (void*)vaddr, paddr, PageTable::PermissionRWPriv);
	}
}

void Kernel::init()
{
	Page *pages;
	PageTable *pageTable;
	AddressSpace *addressSpace;
	struct Page *vectorPage;
	char *vector;

	pages = Page::fromPAddr(KernelTablePAddr);
	pageTable = new PageTable(pages);
	addressSpace = new AddressSpace(pageTable);
	sProcess = new Process(addressSpace);

	vectorPage = Page::alloc();
	vector = (char*)vectorPage->vaddr();
	pageTable->mapPage((void*)0xffff0000, vectorPage->paddr(), PageTable::PermissionRWPriv);
	::memcpy(vector, vectorStart, (unsigned)vectorEnd - (unsigned)vectorStart);
}


int Kernel::syscall(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
	struct Object *object;
	struct Message *message;
	int ret;

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

		case SyscallGetProcessManager:
			return Sched::current()->process()->dupObjectRef(Kernel::process(), ProcessManager::object());
	}
}