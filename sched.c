#include "sched.h"

#define NULL (void*)0

struct RunList runList = {NULL, NULL};
struct Task *currentTask = NULL;

void taskAdd(struct Task *task)
{
	if(runList.tail == NULL) {
		runList.head = task;
	} else {
		runList.tail->next = task;
	}
	
	runList.tail = task;
}

struct Task *taskRemoveHead()
{
	struct Task *task;
	
	task = runList.head;
	runList.head = runList.head->next;
	if(runList.head == NULL) {
		runList.tail = NULL;
	}
	
	return task;
}

static void startRoutine()
{
	while(1) {
		schedule();
	}
}

void taskInit(struct Task *task, void *stack)
{
	int i;
	
	for(i=0; i<16; i++) {
		task->regs[i] = 0;
	}
	
	task->regs[R_IP] = startRoutine;
	task->regs[R_SP] = stack;
}

void schedule()
{
	struct Task *newTask;
	struct Task *oldCurrent;
	
	if(currentTask != NULL) {
		taskAdd(currentTask);
	}

	newTask = taskRemoveHead();

	if(newTask != currentTask) {
		oldCurrent = currentTask;
		currentTask = newTask;
		switchTo(oldCurrent, currentTask);
	}
}