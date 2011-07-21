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

void switchTo(struct Task *current, struct Task *next);
void schedule();

struct Task *taskRemoveHead();
void taskAdd(struct Task *task);

void taskInit(struct Task *task, void (*start)(), void *stack);

#endif