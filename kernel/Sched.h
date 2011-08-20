#ifndef SCHED_H
#define SCHED_H

#include "Task.h"
#include "List.h"

class Sched {
public:
	static void runFirst();
	static void runNext();
	static void add(Task *task);
	static void switchTo(Task *task);

	static Task *current() { return sCurrent; }
private:
	static Task *sCurrent;
	static List<Task> sRunList;
};

#endif