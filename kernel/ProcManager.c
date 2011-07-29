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
	struct List stackPages = Page_AllocMulti(stackSize >> PAGE_SHIFT);
	char *stack = (char*)(KERNEL_START - stackSize);
	AddressSpace_Map(Current->addressSpace, stack, stackPages);
	struct StartupInfo *startupInfo = (struct StartupInfo*)Current - 1;
	int size;
	void *data = InitFs_Lookup(startupInfo->name, &size);
	void *entry = Elf_Load(Current->addressSpace, data, size);

	EnterUser(entry, stack + stackSize);
}

static struct Task *createUserTask(const char *name)
{
	struct Task *task = Task_Create(startUser);
	struct StartupInfo *startupInfo = (struct StartupInfo*)task->regs[R_SP] - 1;
	task->regs[R_SP] = (unsigned int)startupInfo;
	strcpy(startupInfo->name, name);

	return task;
}

static struct Channel *procManagerChannel;

static void testStart()
{
	struct Connection *connection = Connection_Create(Current, procManagerChannel);

	while(1) {
		Message_Send(connection);
	}
}

static void procManagerMain(void *param)
{
	struct Task *task = Task_Create(testStart);
	struct Connection *connection;

	Task_AddTail(task);

	procManagerChannel = Channel_Create(Current);

	while(1) {
		connection = Message_Receive(procManagerChannel);
		Message_Reply(connection);
	}
}

void ProcManager_Start()
{
	struct Task *task = Task_Create(procManagerMain);
	Task_AddTail(task);

	ScheduleFirst();
}