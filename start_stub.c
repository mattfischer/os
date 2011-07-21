#include "sched.h"

struct Task task1;
struct Task task2;

char stack1[256];
char stack2[256];
char initStack[256];

void start_stub()
{
	taskInit(&task1, stack1 + sizeof(stack1));
	taskInit(&task2, stack2 + sizeof(stack2));

	taskAdd(&task1);
	taskAdd(&task2);

	schedule();
}