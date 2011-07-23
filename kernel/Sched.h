#ifndef SCHED_H
#define SCHED_H

#include "Defs.h"
#include "Memory.h"

#define R_SP 13
#define R_IP 15

struct Task {
	unsigned int regs[16];
	struct AddressSpace *addressSpace;
	struct Page *stack;
	struct Task *next;
};

void Schedule();

void TaskAdd(struct Task *task);

struct Task *TaskCreate(void (*start)());

void SchedInit();

extern struct Task *Current;

#endif