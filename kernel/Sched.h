#ifndef SCHED_H
#define SCHED_H

#include "Task.h"

void Sched_RunFirst();
void Sched_RunNext();
void Sched_AddHead(struct Task *task);
void Sched_AddTail(struct Task *task);
void Sched_Init();

extern struct Task *Current;

#endif