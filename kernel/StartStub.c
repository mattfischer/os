#include "Sched.h"

static void task1Start()
{
	while(1) {
		Schedule();
	}
}

static void task2Start()
{
	while(1) {
		Schedule();
	}
}

void StartStub()
{
	struct Task *task;

	task = TaskCreate(task1Start);
	TaskAdd(task);

	task = TaskCreate(task2Start);
	TaskAdd(task);

	Schedule();
}