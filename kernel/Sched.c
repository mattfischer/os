#include "Sched.h"
#include "Slab.h"
#include "InitFs.h"
#include "Elf.h"
#include "Util.h"
#include "AsmFuncs.h"

LIST(struct Task) runList;
struct Task *Current = NULL;

void Sched_AddHead(struct Task *task)
{
       task->state = TaskStateReady;
       LIST_ADD_HEAD(runList, task->list);
}

void Sched_AddTail(struct Task *task)
{
       task->state = TaskStateReady;
       LIST_ADD_TAIL(runList, task->list);
}

static void switchTo(struct Task *current, struct Task *next)
{
	next->state = TaskStateRunning;

	if(next->process != NULL) {
		next->effectiveAddressSpace = next->process->addressSpace;
		SetMMUBase(next->process->addressSpace->tablePAddr);
	} else {
		next->effectiveAddressSpace = Current->effectiveAddressSpace;
	}

	SwitchToAsm(current, next);
}

void Sched_RunNext()
{
	struct Task *next;
	struct Task *old;
	
	if(Current->state == TaskStateRunning) {
		Sched_AddTail(Current);
	}

	next = LIST_HEAD(runList, struct Task, list);
    LIST_REMOVE(runList, next->list);

	if(next != Current) {
		old = Current;
		Current = next;
		switchTo(old, Current);
	}
}

static void runFirst(struct Task *next)
{
	next->state = TaskStateRunning;

	if(next->process != NULL) {
		next->effectiveAddressSpace = next->process->addressSpace;
		SetMMUBase(next->process->addressSpace->tablePAddr);
	} else {
		next->effectiveAddressSpace = &KernelSpace;
	}

	RunFirstAsm(next);
}

void Sched_RunFirst()
{
	Current = LIST_HEAD(runList, struct Task, list);
    LIST_REMOVE(runList, Current->list);

	runFirst(Current);
}

void Sched_Init()
{
	LIST_INIT(runList);
}