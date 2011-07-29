#include "Sched.h"
#include "Slab.h"
#include "InitFs.h"
#include "Elf.h"
#include "Util.h"
#include "AsmFuncs.h"

struct List runList;
struct Task *Current = NULL;

void TaskAdd(struct Task *task)
{
	task->state = TaskStateReady;
	LIST_ADD_TAIL(runList, task->list);
}

static struct Task *removeHead()
{
	struct Task *task;

	task = LIST_HEAD(runList, struct Task, list);
	LIST_REMOVE(runList, task->list);

	return task;
}

struct Task *TaskCreate(void (*start)())
{
	int i;
	struct Task *task;
	struct Page *stack;

	stack = PageAlloc(1);
	task = (struct Task*)(PAGE_TO_VADDR(stack) + PAGE_SIZE - sizeof(struct Task));

	task->stack = stack;
	task->state = TaskStateInit;
	memset(task->regs, 0, 16 * sizeof(unsigned int));
	task->regs[R_PC] = (unsigned int)start;
	task->regs[R_SP] = (unsigned int)task;

	task->addressSpace = AddressSpaceCreate();

	return task;
}

static void switchTo(struct Task *current, struct Task *next)
{
	next->state = TaskStateRunning;

	SetMMUBase(next->addressSpace->tablePAddr);
	SwitchToAsm(current, next);
}

static void runFirst(struct Task *next)
{
	next->state = TaskStateRunning;

	SetMMUBase(next->addressSpace->tablePAddr);
	RunFirstAsm(next);
}

void Schedule()
{
	struct Task *next;
	struct Task *old;
	
	if(Current->state == TaskStateRunning) {
		TaskAdd(Current);
	}

	next = removeHead();

	if(next != Current) {
		old = Current;
		Current = next;
		switchTo(old, Current);
	}
}

void ScheduleFirst()
{
	Current = removeHead();
	runFirst(Current);
}

void SchedInit()
{
	LIST_INIT(runList);
}