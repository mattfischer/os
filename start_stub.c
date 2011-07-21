#include "sched.h"

struct Task task1;
struct Task task2;

char stack1[256];
char stack2[256];
char initStack[256];

static void task1Start()
{
	while(1) {
		schedule();
	}
}

static void task2Start()
{
	while(1) {
		schedule();
	}
}

void start_stub()
{
	taskInit(&task1, task1Start, stack1 + sizeof(stack1));
	taskInit(&task2, task2Start, stack2 + sizeof(stack2));

	taskAdd(&task1);
	taskAdd(&task2);

	schedule();
}