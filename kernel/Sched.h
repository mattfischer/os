#ifndef SCHED_H
#define SCHED_H

#include "Task.h"

void Sched_RunFirst();
void Sched_RunNext();
void Sched_Add(struct Task *task);
void Sched_SwitchTo(struct Task *task);
void Sched_Init();

extern struct Task *Current;

#endif