#include "Sched.h"
#include "Slab.h"
#include "InitFs.h"
#include "Elf.h"
#include "Util.h"
#include "AsmFuncs.h"

List2<Task, &Task::list> Sched::sRunList;
Task *Sched::sCurrent = NULL;

void Sched::add(Task *task)
{
	task->setState(Task::StateReady);
	sRunList.addTail(task);
}

static void switchTo(Task *current, Task *next)
{
}

void Sched::switchTo(Task *task)
{
	task->setState(Task::StateRunning);

	if(task->process()->addressSpace() != AddressSpace::Kernel) {
		task->setEffectiveAddressSpace(task->process()->addressSpace());
		SetMMUBase(task->process()->addressSpace()->pageTable()->tablePAddr());
	} else {
		task->setEffectiveAddressSpace(sCurrent->effectiveAddressSpace());
	}

	Task *old = sCurrent;
	sCurrent = task;
	SwitchToAsm(old, task);
}

void Sched::runNext()
{
	Task *next;
	Task *old;
	
	if(sCurrent->state() == Task::StateRunning) {
		add(sCurrent);
	}

	next = sRunList.head();
	sRunList.remove(next);

	if(next != sCurrent) {
		switchTo(next);
	}
}

void Sched::runFirst()
{
	Task *task = sRunList.head();
    sRunList.remove(task);

	task->setState(Task::StateRunning);

	task->setEffectiveAddressSpace(task->process()->addressSpace());
	SetMMUBase(task->process()->addressSpace()->pageTable()->tablePAddr());

	sCurrent = task;
	RunFirstAsm(task);
}
