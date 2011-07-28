#include "Sched.h"
#include "InitFs.h"

static void task1Start()
{
	while(1) {
		Schedule();
	}
}

void StartStub()
{
	struct Task *task;

	task = TaskCreateKernel(task1Start);
	TaskAdd(task);


	task = TaskCreate("test");
	TaskAdd(task);

	ScheduleFirst();
}