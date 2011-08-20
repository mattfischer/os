#include "Task.h"
#include "Util.h"
#include "Sched.h"

SlabAllocator<Task> Task::sSlab;

Task::Task(struct Process *process)
{
	int i;
	struct Task *task;
	struct Page *stack;

	stack = Page_Alloc();

	mStack = stack;
	mState = StateInit;
	memset(mRegs, 0, 16 * sizeof(unsigned int));
	mRegs[R_SP] = (unsigned)PAGE_TO_VADDR(mStack) + PAGE_SIZE;

	mProcess = process;
	mEffectiveAddressSpace = NULL;
}

void *Task::stackAllocate(int size)
{
	mRegs[R_SP] -= size;

	return (void*)mRegs[R_SP];
}

void Task::start(void (*start)(void *), void *param)
{
	mRegs[0]    = (unsigned int)param;
	mRegs[R_PC] = (unsigned int)start;

	Sched::add(this);
}
