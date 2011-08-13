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

static int procManager;

static void startUser(void *param)
{
	struct StartupInfo *startupInfo;
	struct Task *task;
	int stackSize;
	struct MemArea *stackArea;
	char *stackVAddr;
	int size;
	void *data;
	void *entry;

	startupInfo = (struct StartupInfo*)param;

	stackSize = PAGE_SIZE;
	stackArea = MemArea_CreatePages(stackSize);
	stackVAddr = (char*)(KERNEL_START - stackArea->size);
	AddressSpace_Map(Current->process->addressSpace, stackArea, stackVAddr, 0, stackArea->size);

	data = InitFs_Lookup(startupInfo->name, &size);
	entry = Elf_Load(Current->process->addressSpace, data, size);

	EnterUser(entry, stackVAddr + stackSize);
}

static void startUserProcess(const char *name)
{
	struct AddressSpace *addressSpace;
	struct Process *process;
	struct Task *task;
	struct StartupInfo *startupInfo;

	addressSpace = AddressSpace_Create();
	process = Process_Create(addressSpace);
	Process_RefObject(process, Current->process->objects[procManager]);
	task = Task_Create(process);

	startupInfo = (struct StartupInfo *)Task_StackAllocate(task, sizeof(struct StartupInfo));
	strcpy(startupInfo->name, name);

	Task_Start(task, startUser, startupInfo);
}

static void procManagerMain(void *param)
{
	procManager = CreateObject();

	startUserProcess("server");
	startUserProcess("client");
	startUserProcess("client2");

	while(1) {
		struct ProcManagerMsg message;
		struct MessageHeader header;
		int msg;

		header.size = sizeof(message);
		header.body = &message;

		msg = ReceiveMessagex(procManager, &header);

		switch(message.type) {
			case ProcManagerNameLookup:
			{
				int object = Name_Lookup(message.u.lookup.name);
				struct ProcManagerMsgNameLookupReply reply;

				reply.obj = object;
				header.body = &reply;
				header.size = sizeof(reply);
				header.objectsSize = 1;
				header.objectsOffset = offsetof(struct ProcManagerMsgNameLookupReply, obj);

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
				AddressSpace_Map(Current->process->messages[msg]->sender->process->addressSpace, area, (void*)message.u.mapPhys.vaddr, 0, area->size);

				ReplyMessage(msg, 0, NULL, 0);
			}
		}
	}
}

void ProcManager_Start()
{
	struct Task *task = Task_Create(KernelProcess);
	Task_Start(task, procManagerMain, NULL);

	Sched_RunFirst();
}