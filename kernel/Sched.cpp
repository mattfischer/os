#include "Sched.h"
#include "Slab.h"
#include "Util.h"
#include "AsmFuncs.h"
#include "Kernel.h"

List<Task> Sched::sRunList;
Task *Sched::sCurrent = NULL;

void Sched::add(Task *task)
{
	task->setState(Task::StateReady);
	sRunList.addTail(task);
}

void Sched::switchTo(Task *task)
{
	task->setState(Task::StateRunning);

	if(task->process() != Kernel::process()) {
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

	next = sRunList.removeHead();

	if(next != sCurrent) {
		switchTo(next);
	}
}

void Sched::runFirst()
{
	Task *task = sRunList.removeHead();

	task->setState(Task::StateRunning);

	task->setEffectiveAddressSpace(task->process()->addressSpace());
	SetMMUBase(task->process()->addressSpace()->pageTable()->tablePAddr());

	sCurrent = task;
	RunFirstAsm(task);
}
