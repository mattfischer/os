#ifndef SCHED_H
#define SCHED_H

#define R_SP 13
#define R_IP 15

struct Task
{
	unsigned int regs[16];
	struct Task *next;
};

struct RunList
{
	struct Task *head;
	struct Task *tail;
};

extern struct RunList runList;
extern struct Task *currentTask;

void SwitchTo(struct Task *current, struct Task *next);
void Schedule();

struct Task *TaskRemoveHead();
void TaskAdd(struct Task *task);

void TaskInit(struct Task *task, void (*start)(), void *stack);

#endif