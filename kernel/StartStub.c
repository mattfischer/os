#include "Sched.h"

struct Task task1;
struct Task task2;

char stack1[256];
char stack2[256];
char initStack[256];

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
	TaskInit(&task1, task1Start, stack1 + sizeof(stack1));
	TaskInit(&task2, task2Start, stack2 + sizeof(stack2));

	TaskAdd(&task1);
	TaskAdd(&task2);

	Schedule();
}