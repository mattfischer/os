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

#include <kernel/include/ProcManagerFmt.h>

struct StartupInfo {
	char name[16];
};

static struct Object *procManager;

static void startUser(void *param)
{
	struct StartupInfo *startupInfo;
	struct Task *task;
	int stackSize;
	LIST(struct Page) stackPages;
	char *stack;
	int size;
	void *data;
	void *entry;

	startupInfo = (struct StartupInfo*)param;

	stackSize = PAGE_SIZE;
	stackPages = Page_AllocMulti(stackSize >> PAGE_SHIFT);
	stack = (char*)(KERNEL_START - stackSize);
	AddressSpace_Map(Current->process->addressSpace, stack, stackPages);

	data = InitFs_Lookup(startupInfo->name, &size);
	entry = Elf_Load(Current->process->addressSpace, data, size);

	EnterUser(entry, stack + stackSize);
}

static void startUserProcess(const char *name)
{
	struct Process *process;
	struct Task *task;
	struct StartupInfo *startupInfo;

	process = Process_Create();
	Process_RefObject(process, procManager);
	task = Task_Create(process);

	startupInfo = (struct StartupInfo *)Task_StackAllocate(task, sizeof(struct StartupInfo));
	strcpy(startupInfo->name, name);

	Task_Start(task, startUser, startupInfo);
}

static void procManagerMain(void *param)
{
	procManager = Object_Create();

	startUserProcess("test");

	while(1) {
		struct ProcManagerMsg msg;
		struct MessageHeader header;
		struct Message *message;

		header.size = sizeof(msg);
		header.body = &msg;

		message = Object_ReceiveMessage(procManager, &header);

		switch(msg.type) {
			case ProcManagerNameLookup:
			{
				struct Object *object = Name_Lookup(msg.u.lookup.name);
				struct ProcManagerMsgNameLookupReply reply;

				reply.obj = (unsigned int)object;
				header.body = &reply;
				header.size = sizeof(reply);
				header.objectsSize = 1;
				header.objectsOffset = offsetof(struct ProcManagerMsgNameLookupReply, obj);

				Object_ReplyMessage(message, &header);
				break;
			}

			case ProcManagerNameSet:
			{
				Name_Set(msg.u.set.name, (struct Object*)msg.u.set.obj);

				Object_ReplyMessage(message, NULL);
				break;
			}
		}
	}
}

void ProcManager_Start()
{
	struct Task *task = Task_Create(NULL);
	Task_Start(task, procManagerMain, NULL);

	Sched_RunFirst();
}