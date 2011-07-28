#ifndef SCHED_H
#define SCHED_H

#include "Defs.h"
#include "Page.h"
#include "Map.h"

#define R_SP 13
#define R_PC 15

struct Task {
	unsigned int regs[16];
	struct AddressSpace *addressSpace;
	struct Page *stack;
	char name[16];
	struct Task *next;
};

void Schedule();
void ScheduleFirst();

void TaskAdd(struct Task *task);

struct Task *TaskCreate(void (*start)());

void SchedInit();

extern struct Task *Current;

#endif