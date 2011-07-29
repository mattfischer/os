#ifndef TASK_H
#define TASK_H

#include "AddressSpace.h"
#include "Defs.h"
#include "Page.h"
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
	struct AddressSpace *effectiveAddressSpace;
	struct Page *stack;
	struct ListEntry list;
};

struct Task *Task_Create(struct AddressSpace *addressSpace);
void *Task_StackAllocate(struct Task *task, int size);
void Task_Start(struct Task *task, void (*start)(void *), void *param);

#endif