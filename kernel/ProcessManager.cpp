#include "ProcessManager.h"
#include "Sched.h"
#include "InitFs.h"
#include "AsmFuncs.h"
#include "Elf.h"
#include "Util.h"
#include "AddressSpace.h"
#include "Process.h"
#include "Object.h"
#include "Message.h"
#include "Kernel.h"

#include <kernel/include/ProcManagerFmt.h>

struct StartupInfo {
	char name[16];
};

int ProcessManager::sObject;

static void startUser(void *param)
{
	struct StartupInfo *startupInfo = (struct StartupInfo*)param;

	int stackSize = PAGE_SIZE;
	MemArea *stackArea = new MemAreaPages(stackSize);
	char *stackVAddr = (char*)(KERNEL_START - stackArea->size());
	Sched::current()->process()->addressSpace()->map(stackArea, stackVAddr, 0, stackArea->size());

	int size;
	void *data = InitFs_Lookup(startupInfo->name, &size);
	Elf::Entry entry = Elf::load(Sched::current()->process()->addressSpace(), data, size);

	EnterUser(entry, stackVAddr + stackSize);
}

static void startUserProcess(const char *name, int stdinObject, int stdoutObject, int stderrObject)
{
	Process *process = new Process();
	process->dupObjectRefTo(0, Sched::current()->process(), stdinObject);
	process->dupObjectRefTo(1, Sched::current()->process(), stdoutObject);
	process->dupObjectRefTo(2, Sched::current()->process(), stdoutObject);
	Task *task = new Task(process);

	struct StartupInfo *startupInfo = (struct StartupInfo *)task->stackAllocate(sizeof(struct StartupInfo));
	strcpy(startupInfo->name, name);

	task->start(startUser, startupInfo);
}

void ProcessManager::main(void *param)
{
	sObject = Object_Create(OBJECT_INVALID, NULL);

	Kernel::setObject(KernelObjectProcManager, sObject);

	startUserProcess("init", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);

	while(1) {
		struct ProcManagerMsg message;
		struct BufferSegment recvSegs[] = { &message, sizeof(message) };
		struct MessageHeader recvHdr = { recvSegs, 1, 0, 0 };
		int msg = Object_Receivex(object(), &recvHdr);

		switch(message.type) {
			case ProcManagerMapPhys:
			{
				MemArea *area = new MemAreaPhys(message.u.mapPhys.size, message.u.mapPhys.paddr);
				Sched::current()->process()->message(msg)->sender()->process()->addressSpace()->map(area, (void*)message.u.mapPhys.vaddr, 0, area->size());

				Message_Reply(msg, 0, NULL, 0);
				break;
			}

			case ProcManagerSbrk:
			{
				Process *process = Sched::current()->process()->message(msg)->sender()->process();
				int increment = message.u.sbrk.increment;
				int ret;

				if(process->heapTop() == NULL) {
					int size = PAGE_SIZE_ROUND_UP(increment);

					process->setHeap(new MemAreaPages(size));
					process->setHeapTop((char*)(0x10000000 + increment));
					process->setHeapAreaTop((char*)(0x10000000 + size));
					process->addressSpace()->map(process->heap(), (void*)0x10000000, 0, size);
					ret = 0x10000000;
				} else {
					if(process->heapTop() + increment < process->heapAreaTop()) {
						ret = (int)process->heapTop();
						process->setHeapTop(process->heapTop() + increment);
					} else {
						int size = PAGE_SIZE_ROUND_UP(increment);
						int extraPages = size >> PAGE_SHIFT;

						ret = (int)process->heapTop();
						for(int i=0; i<extraPages; i++) {
							Page *page = Page::alloc();
							process->heap()->pages().addTail(page);
							process->addressSpace()->pageTable()->mapPage(process->heapAreaTop(), page->paddr(), PageTable::PermissionRW);
							process->setHeapAreaTop(process->heapAreaTop() + PAGE_SIZE);
						}
						process->setHeapTop(process->heapTop() + increment);
					}
				}

				Message_Reply(msg, ret, NULL, 0);
				break;
			}

			case ProcManagerSpawnProcess:
			{
				startUserProcess(message.u.spawn.name, message.u.spawn.stdinObject, message.u.spawn.stdoutObject, message.u.spawn.stderrObject);
				Object_Release(message.u.spawn.stdinObject);
				Object_Release(message.u.spawn.stdoutObject);
				Object_Release(message.u.spawn.stderrObject);

				Message_Reply(msg, 0, NULL, 0);
			}
		}
	}
}

void ProcessManager::start()
{
	Task *task = new Task(Kernel::process());
	task->start(main, NULL);

	Sched::runFirst();
}