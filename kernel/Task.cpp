#include "Task.h"
#include "Util.h"
#include "Sched.h"

struct Task *Task_Create(struct Process *process)
{
	int i;
	struct Task *task;
	struct Page *stack;

	stack = Page_Alloc();
	task = (struct Task*)(PAGE_TO_VADDR(stack) + PAGE_SIZE - sizeof(struct Task));

	task->stack = stack;
	task->state = TaskStateInit;
	memset(task->regs, 0, 16 * sizeof(unsigned int));
	task->regs[R_SP] = (unsigned int)task;

	task->process = process;
	task->effectiveAddressSpace = NULL;

	return task;
}

void *Task_StackAllocate(struct Task *task, int size)
{
	task->regs[R_SP] -= size;

	return (void*)task->regs[R_SP];
}

void Task_Start(struct Task *task, void (*start)(void *), void *param)
{
	task->regs[0]    = (unsigned int)param;
	task->regs[R_PC] = (unsigned int)start;

	Sched_Add(task);
}
