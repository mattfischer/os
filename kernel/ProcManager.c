#include "ProcManager.h"
#include "Sched.h"
#include "InitFs.h"
#include "AsmFuncs.h"
#include "Elf.h"
#include "Util.h"

static void startUser()
{
	int stackSize = PAGE_SIZE;
	struct Page *stackPages = PageAlloc(stackSize >> PAGE_SHIFT);
	char *stack = (char*)(KERNEL_START - stackSize);
	MapPages(Current->addressSpace, stack, stackPages);

	int size;
	void *data = InitFsLookup(Current->name, &size);
	void *entry = ElfLoad(Current->addressSpace, data, size);

	EnterUser(entry, stack + stackSize);
}

struct Task *createUserTask(const char *name)
{
	struct Task *task = TaskCreate(startUser);
	strcpy(task->name, name);

	return task;
}

void procManagerMain(void *param)
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