#ifndef SCHED_H
#define SCHED_H

#include "Defs.h"
#include "Page.h"
#include "Map.h"
#include "Message.h"
#include "List.h"

#define R_SP 13
#define R_PC 15

enum TaskState {
	TaskStateInit,
	TaskStateRunning,
	TaskStateReady,
	TaskStateReceiveBlock,
	TaskStateSendBlock,
	TaskStateReplyBlock
};

struct Task {
	unsigned int regs[16];
	enum TaskState state;
	struct AddressSpace *addressSpace;
	struct Page *stack;
	struct ListEntry list;
};

void Schedule();
void ScheduleFirst();

void TaskAdd(struct Task *task);

struct Task *TaskCreate(void (*start)());

void SchedInit();

extern struct Task *Current;

#endif