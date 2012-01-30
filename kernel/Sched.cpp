#include "Sched.hpp"

#include "Slab.hpp"
#include "Util.hpp"
#include "AsmFuncs.hpp"
#include "Kernel.hpp"

//! Queue of tasks that are ready to run
List<Task> Sched::sRunList;

//! Currently running task
Task *Sched::sCurrent = NULL;

/*!
 * \brief Add a task to the run queue
 * \param task Task to add
 */
void Sched::add(Task *task)
{
	task->setState(Task::StateReady);
	sRunList.addTail(task);
}

/*!
 * \brief Switch to a task
 * \param task Task to switch to
 */
void Sched::switchTo(Task *task)
{
	task->setState(Task::StateRunning);

	if(task != sCurrent) {
		// If the target task is a kernel thread, it has no address space, so
		// the currently active address space can stay in place.  This optimizes
		// for the case where a userspace thread incurs work on a kernel thread
		// and then switches back to the user thread--using this technique, no
		// MMU switches are necessary
		if(task->process() != Kernel::process()) {
			task->setEffectiveAddressSpace(task->process()->addressSpace());
			SetMMUBase(task->process()->addressSpace()->pageTable()->tablePAddr());
		} else {
			task->setEffectiveAddressSpace(sCurrent->effectiveAddressSpace());
		}

		// Switch to new task
		Task *old = sCurrent;
		sCurrent = task;
		SwitchToAsm(old->regs(), task->regs());
	}
}

/*!
 * \brief Switch to the next task in the run queue
 */
void Sched::runNext()
{
	// If the current task is still running, add to the
	// tail of the runqueue
	if(sCurrent->state() == Task::StateRunning) {
		add(sCurrent);
	}

	// Grab new task and switch to it
	Task *next = sRunList.removeHead();

	switchTo(next);
}

/*!
 * \brief Run first task in run queue
 */
void Sched::runFirst()
{
	Task *task = sRunList.removeHead();

	task->setState(Task::StateRunning);

	task->setEffectiveAddressSpace(task->process()->addressSpace());
	SetMMUBase(task->process()->addressSpace()->pageTable()->tablePAddr());

	sCurrent = task;
	RunFirstAsm(task->regs());
}
