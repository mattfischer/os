#include "Sched.h"
#include "Slab.h"
#include "InitFs.h"
#include "Elf.h"
#include "Util.h"
#include "AsmFuncs.h"

LIST(struct Task) runList;
struct Task *Current = NULL;

void Sched_Add(struct Task *task)
{
	task->state = TaskStateReady;
	LIST_ADD_TAIL(runList, task->list);
}

static void switchTo(struct Task *current, struct Task *next)
{
	next->state = TaskStateRunning;

	if(next->process->AddressSpace() != KernelSpace) {
		next->effectiveAddressSpace = next->process->AddressSpace();
		SetMMUBase(next->process->AddressSpace()->pageTable->tablePAddr);
	} else {
		next->effectiveAddressSpace = Current->effectiveAddressSpace;
	}

	Current = next;
	SwitchToAsm(current, next);
}

void Sched_SwitchTo(struct Task *task)
{
	switchTo(Current, task);
}

void Sched_RunNext()
{
	struct Task *next;
	struct Task *old;
	
	if(Current->state == TaskStateRunning) {
		Sched_Add(Current);
	}

	next = LIST_HEAD(runList, struct Task, list);
    LIST_REMOVE(runList, next->list);

	if(next != Current) {
		switchTo(Current, next);
	}
}

static void runFirst(struct Task *task)
{
	task->state = TaskStateRunning;

	task->effectiveAddressSpace = task->process->AddressSpace();
	SetMMUBase(task->process->AddressSpace()->pageTable->tablePAddr);

	Current = task;
	RunFirstAsm(task);
}

void Sched_RunFirst()
{
	struct Task *task = LIST_HEAD(runList, struct Task, list);
    LIST_REMOVE(runList, task->list);

	runFirst(task);
}

void Sched_Init()
{
	LIST_INIT(runList);
}