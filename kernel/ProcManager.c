#include "ProcManager.h"
#include "Sched.h"
#include "InitFs.h"
#include "AsmFuncs.h"
#include "Elf.h"
#include "Util.h"
#include "AddressSpace.h"
#include "Process.h"
#include "Object.h"

struct StartupInfo {
	char name[16];
	struct Task *task;
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
	task = startupInfo->task;

	stackSize = PAGE_SIZE;
	stackPages = Page_AllocMulti(stackSize >> PAGE_SHIFT);
	stack = (char*)(KERNEL_START - stackSize);
	AddressSpace_Map(task->process->addressSpace, stack, stackPages);

	data = InitFs_Lookup(startupInfo->name, &size);
	entry = Elf_Load(task->process->addressSpace, data, size);

	EnterUser(entry, stack + stackSize);
}

static void startUserTask(const char *name)
{
	struct Process *process;
	struct Task *task;
	struct StartupInfo *startupInfo;

	process = Process_Create();
	task = Task_Create(process);

	startupInfo = (struct StartupInfo *)Task_StackAllocate(task, sizeof(struct StartupInfo));
	strcpy(startupInfo->name, name);

	Task_Start(task, startUser, startupInfo);
}

static void testStart(void *param)
{
	int x = 0;

	while(1) {
		int r;
		Object_SendMessage(procManager, &x, sizeof(x), &r, sizeof(r));
		x = r;
	}
}

static void procManagerMain(void *param)
{
	struct Task *task;
	struct Message *message;

	procManager = Object_Create();

	task = Task_Create(NULL);
	Task_Start(task, testStart, NULL);

	while(1) {
		int x;
		message = Object_ReceiveMessage(procManager, &x, sizeof(x));
		x += 1;
		Object_ReplyMessage(message, &x, sizeof(x));
	}
}

void ProcManager_Start()
{
	struct Task *task = Task_Create(NULL);
	Task_Start(task, procManagerMain, NULL);

	Sched_RunFirst();
}