#include "Sched.h"
#include "Slab.h"
#include "InitFs.h"
#include "Elf.h"
#include "Util.h"
#include "AsmFuncs.h"

List2<Task, &Task::list> runList;
Task *Current = NULL;

void Sched_Add(Task *task)
{
	task->setState(Task::StateReady);
	runList.addTail(task);
}

static void switchTo(Task *current, Task *next)
{
	next->setState(Task::StateRunning);

	if(next->process()->addressSpace() != AddressSpace::Kernel) {
		next->setEffectiveAddressSpace(next->process()->addressSpace());
		SetMMUBase(next->process()->addressSpace()->pageTable()->tablePAddr());
	} else {
		next->setEffectiveAddressSpace(Current->effectiveAddressSpace());
	}

	Current = next;
	SwitchToAsm(current, next);
}

void Sched_SwitchTo(Task *task)
{
	switchTo(Current, task);
}

void Sched_RunNext()
{
	Task *next;
	Task *old;
	
	if(Current->state() == Task::StateRunning) {
		Sched_Add(Current);
	}

	next = runList.head();
	runList.remove(next);

	if(next != Current) {
		switchTo(Current, next);
	}
}

static void runFirst(Task *task)
{
	task->setState(Task::StateRunning);

	task->setEffectiveAddressSpace(task->process()->addressSpace());
	SetMMUBase(task->process()->addressSpace()->pageTable()->tablePAddr());

	Current = task;
	RunFirstAsm(task);
}

void Sched_RunFirst()
{
	Task *task = runList.head();
    runList.remove(task);

	runFirst(task);
}
