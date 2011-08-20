#include "ProcManager.h"
#include "Sched.h"
#include "InitFs.h"
#include "AsmFuncs.h"
#include "Elf.h"
#include "Util.h"
#include "AddressSpace.h"
#include "Process.h"
#include "Object.h"
#include "Name.h"
#include "Message.h"
#include "Kernel.h"

#include <kernel/include/ProcManagerFmt.h>

struct StartupInfo {
	char name[16];
};

int ProcessManager;

static void startUser(void *param)
{
	struct StartupInfo *startupInfo;
	struct Task *task;
	int stackSize;
	MemArea *stackArea;
	char *stackVAddr;
	int size;
	void *data;
	ElfEntry entry;

	startupInfo = (struct StartupInfo*)param;

	stackSize = PAGE_SIZE;
	stackArea = new MemAreaPages(stackSize);
	stackVAddr = (char*)(KERNEL_START - stackArea->size());
	Sched::current()->process()->addressSpace()->map(stackArea, stackVAddr, 0, stackArea->size());

	data = InitFs_Lookup(startupInfo->name, &size);
	entry = Elf_Load(Sched::current()->process()->addressSpace(), data, size);

	EnterUser(entry, stackVAddr + stackSize);
}

static void startUserProcess(const char *name, int stdinObject, int stdoutObject, int stderrObject)
{
	Process *process;
	struct Task *task;
	struct StartupInfo *startupInfo;

	process = new Process();
	process->dupObjectRefTo(0, Sched::current()->process(), stdinObject);
	process->dupObjectRefTo(1, Sched::current()->process(), stdoutObject);
	process->dupObjectRefTo(2, Sched::current()->process(), stdoutObject);
	task = new Task(process);

	startupInfo = (struct StartupInfo *)task->stackAllocate(sizeof(struct StartupInfo));
	strcpy(startupInfo->name, name);

	task->start(startUser, startupInfo);
}

static void procManagerMain(void *param)
{
	ProcessManager = CreateObject();

	startUserProcess("init", INVALID_OBJECT, INVALID_OBJECT, INVALID_OBJECT);

	while(1) {
		struct ProcManagerMsg message;
		struct MessageHeader header;
		struct BufferSegment segments[] = { &message, sizeof(message) };
		int msg;

		header.segments = segments;
		header.numSegments = 1;

		msg = ReceiveMessagex(ProcessManager, &header);

		switch(message.type) {
			case ProcManagerNameLookup:
			{
				int object = Name::lookup(message.u.lookup.name);

				segments[0].buffer = &object;
				segments[0].size = sizeof(object);
				header.segments = segments;
				header.numSegments = 1;
				header.objectsSize = 1;
				header.objectsOffset = 0;

				ReplyMessagex(msg, 0, &header);
				break;
			}

			case ProcManagerNameSet:
			{
				Name::set(message.u.set.name, message.u.set.obj);

				ReplyMessage(msg, 0, NULL, 0);
				break;
			}

			case ProcManagerMapPhys:
			{
				struct MemArea *area = new MemAreaPhys(message.u.mapPhys.size, message.u.mapPhys.paddr);
				Sched::current()->process()->message(msg)->sender()->process()->addressSpace()->map(area, (void*)message.u.mapPhys.vaddr, 0, area->size());

				ReplyMessage(msg, 0, NULL, 0);
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
						int i;

						ret = (int)process->heapTop();
						for(i=0; i<extraPages; i++) {
							Page *page = Page::alloc();
							process->heap()->pages().addTail(page);
							process->addressSpace()->pageTable()->mapPage(process->heapAreaTop(), page->paddr(), PageTable::PermissionRW);
							process->setHeapAreaTop(process->heapAreaTop() + PAGE_SIZE);
						}
						process->setHeapTop(process->heapTop() + increment);
					}
				}

				ReplyMessage(msg, ret, NULL, 0);
				break;
			}

			case ProcManagerSpawnProcess:
			{
				startUserProcess(message.u.spawn.name, message.u.spawn.stdinObject, message.u.spawn.stdoutObject, message.u.spawn.stderrObject);
				ReleaseObject(message.u.spawn.stdinObject);
				ReleaseObject(message.u.spawn.stdoutObject);
				ReleaseObject(message.u.spawn.stderrObject);

				ReplyMessage(msg, 0, NULL, 0);
			}
		}
	}
}

void ProcManager_Start()
{
	struct Task *task = new Task(Kernel::process());
	task->start(procManagerMain, NULL);

	Sched::runFirst();
}