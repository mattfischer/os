#include "ProcManager.h"
#include "Sched.h"
#include "InitFs.h"
#include "AsmFuncs.h"
#include "Elf.h"
#include "Util.h"
#include "Message.h"
#include "AddressSpace.h"

struct StartupInfo {
	char name[16];
};

static void startUser()
{
	int stackSize = PAGE_SIZE;
	struct List stackPages = PageAllocMulti(stackSize >> PAGE_SHIFT);
	char *stack = (char*)(KERNEL_START - stackSize);
	AddressSpaceMap(Current->addressSpace, stack, stackPages);
	struct StartupInfo *startupInfo = (struct StartupInfo*)Current - 1;
	int size;
	void *data = InitFsLookup(startupInfo->name, &size);
	void *entry = ElfLoad(Current->addressSpace, data, size);

	EnterUser(entry, stack + stackSize);
}

static struct Task *createUserTask(const char *name)
{
	struct Task *task = TaskCreate(startUser);
	struct StartupInfo *startupInfo = (struct StartupInfo*)task->regs[R_SP] - 1;
	task->regs[R_SP] = (unsigned int)startupInfo;
	strcpy(startupInfo->name, name);

	return task;
}

static struct Channel *procManagerChannel;

static void testStart()
{
	struct Connection *connection = ConnectionCreate(Current, procManagerChannel);

	while(1) {
		MessageSend(connection);
	}
}

static void procManagerMain(void *param)
{
	struct Task *task = TaskCreate(testStart);
	struct Connection *connection;

	TaskAdd(task);

	procManagerChannel = ChannelCreate(Current);

	while(1) {
		connection = MessageReceive(procManagerChannel);
		MessageReply(connection);
	}
}

void ProcManagerStart()
{
	struct Task *task = TaskCreate(procManagerMain);
	TaskAdd(task);

	ScheduleFirst();
}