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
	struct MemArea *stackArea;
	char *stackVAddr;
	int size;
	void *data;
	ElfEntry entry;

	startupInfo = (struct StartupInfo*)param;

	stackSize = PAGE_SIZE;
	stackArea = MemArea_CreatePages(stackSize);
	stackVAddr = (char*)(KERNEL_START - stackArea->size);
	AddressSpace_Map(Current->process->AddressSpace(), stackArea, stackVAddr, 0, stackArea->size);

	data = InitFs_Lookup(startupInfo->name, &size);
	entry = Elf_Load(Current->process->AddressSpace(), data, size);

	EnterUser(entry, stackVAddr + stackSize);
}

static void startUserProcess(const char *name, int stdinObject, int stdoutObject, int stderrObject)
{
	struct AddressSpace *addressSpace;
	Process *process;
	struct Task *task;
	struct StartupInfo *startupInfo;

	addressSpace = AddressSpace_Create();
	process = new Process(addressSpace);
	process->DupObjectRefTo(0, Current->process, stdinObject);
	process->DupObjectRefTo(1, Current->process, stdoutObject);
	process->DupObjectRefTo(2, Current->process, stdoutObject);
	task = Task_Create(process);

	startupInfo = (struct StartupInfo *)Task_StackAllocate(task, sizeof(struct StartupInfo));
	strcpy(startupInfo->name, name);

	Task_Start(task, startUser, startupInfo);
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
				int object = Name_Lookup(message.u.lookup.name);

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
				Name_Set(message.u.set.name, message.u.set.obj);

				ReplyMessage(msg, 0, NULL, 0);
				break;
			}

			case ProcManagerMapPhys:
			{
				struct MemArea *area = MemArea_CreatePhys(message.u.mapPhys.size, message.u.mapPhys.paddr);
				AddressSpace_Map(Current->process->Message(msg)->sender->process->AddressSpace(), area, (void*)message.u.mapPhys.vaddr, 0, area->size);

				ReplyMessage(msg, 0, NULL, 0);
				break;
			}

			case ProcManagerSbrk:
			{
				struct Process *process = Current->process->Message(msg)->sender->process;
				int increment = message.u.sbrk.increment;
				int ret;

				if(process->HeapTop() == NULL) {
					int size = PAGE_SIZE_ROUND_UP(increment);

					process->SetHeap(MemArea_CreatePages(size));
					process->SetHeapTop((char*)(0x10000000 + increment));
					process->SetHeapAreaTop((char*)(0x10000000 + size));
					AddressSpace_Map(process->AddressSpace(), process->Heap(), (void*)0x10000000, 0, size);
					ret = 0x10000000;
				} else {
					if(process->HeapTop() + increment < process->HeapAreaTop()) {
						ret = (int)process->HeapTop();
						process->SetHeapTop(process->HeapTop() + increment);
					} else {
						int size = PAGE_SIZE_ROUND_UP(increment);
						int extraPages = size >> PAGE_SHIFT;
						int i;

						ret = (int)process->HeapTop();
						for(i=0; i<extraPages; i++) {
							struct Page *page = Page_Alloc();
							LIST_ADD_TAIL(process->Heap()->u.pages, page->list);
							PageTable_MapPage(process->AddressSpace()->pageTable, process->HeapAreaTop(), PAGE_TO_PADDR(page), PageTablePermissionRW);
							process->SetHeapAreaTop(process->HeapAreaTop() + PAGE_SIZE);
						}
						process->SetHeapTop(process->HeapTop() + increment);
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
	struct Task *task = Task_Create(Process::Kernel);
	Task_Start(task, procManagerMain, NULL);

	Sched_RunFirst();
}