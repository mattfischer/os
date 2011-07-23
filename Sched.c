#include "sched.h"

#define NULL (void*)0

struct RunList runList = {NULL, NULL};
struct Task *currentTask = NULL;

void TaskAdd(struct Task *task)
{
	if(runList.tail == NULL) {
		runList.head = task;
	} else {
		runList.tail->next = task;
	}
	
	runList.tail = task;
}

struct Task *TaskRemoveHead()
{
	struct Task *task;
	
	task = runList.head;
	runList.head = runList.head->next;
	if(runList.head == NULL) {
		runList.tail = NULL;
	}
	
	return task;
}

void TaskInit(struct Task *task, void (*start)(), void *stack)
{
	int i;
	
	for(i=0; i<16; i++) {
		task->regs[i] = 0;
	}
	
	task->regs[R_IP] = (unsigned int)start;
	task->regs[R_SP] = (unsigned int)stack;
}

void Schedule()
{
	struct Task *newTask;
	struct Task *oldCurrent;
	
	if(currentTask != NULL) {
		TaskAdd(currentTask);
	}

	newTask = TaskRemoveHead();

	if(newTask != currentTask) {
		oldCurrent = currentTask;
		currentTask = newTask;
		SwitchTo(oldCurrent, currentTask);
	}
}