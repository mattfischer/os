#include "Task.h"
#include "Util.h"
#include "Sched.h"

//! Slab allocator for tasks
Slab<Task> Task::sSlab;

/*!
 * \brief Constructor
 * \param process Owning process
 */
Task::Task(Process *process)
{
	Page *stack = Page::alloc();

	mStack = stack;
	mState = StateInit;
	memset(mRegs, 0, 16 * sizeof(unsigned int));
	mRegs[R_SP] = (unsigned)mStack->vaddr() + PAGE_SIZE;

	mProcess = process;
	mEffectiveAddressSpace = NULL;
}

/*!
 * \brief Allocate memory from the top of the task's stack
 *
 * This may only be called on a task that is in StateInit
 * \param size
 * \return Allocated memory
 */
void *Task::stackAllocate(int size)
{
	mRegs[R_SP] -= size;

	return (void*)mRegs[R_SP];
}

/*!
 * \brief Start executing the task
 * \param start Task function
 * \param param Parameter to pass to starting function
 */
void Task::start(void (*start)(void *), void *param)
{
	// Set the pc and sp of the saved register area
	mRegs[0]    = (unsigned int)param;
	mRegs[R_PC] = (unsigned int)start;

	// Add the task to the scheduler queue
	Sched::add(this);
}
