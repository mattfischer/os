#include "ProcManager.h"
#include "Sched.h"
#include "InitFs.h"
#include "AsmFuncs.h"
#include "Elf.h"
#include "Util.h"

struct StartupInfo {
	char name[16];
};

static void startUser()
{
	int stackSize = PAGE_SIZE;
	struct Page *stackPages = PageAlloc(stackSize >> PAGE_SHIFT);
	char *stack = (char*)(KERNEL_START - stackSize);
	MapCreate(Current->addressSpace, stack, stackPages);
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

static void procManagerMain(void *param)
{
	struct Task *task = createUserTask("test");
	TaskAdd(task);

	while(1) {
		Schedule();
	}
}

void ProcManagerStart()
{
	struct Task *task = TaskCreate(procManagerMain);
	TaskAdd(task);

	ScheduleFirst();
}