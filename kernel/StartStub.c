#include "Sched.h"

void swi();

static void task1Start()
{
	while(1) {
		swi();
	}
}

static void task2Start()
{
	while(1) {
		swi();
	}
}

void StartStub()
{
	struct Task *task;

	task = TaskCreate(task1Start);
	TaskAdd(task);

	task = TaskCreate(task2Start);
	TaskAdd(task);

	ScheduleFirst();
}