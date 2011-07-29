#ifndef ASM_FUNCS_H
#define ASM_FUNCS_H

#include "Sched.h"
#include "AddressSpace.h"

void EnterUser(void (*userStart)(), void* userStack);
void SetMMUBase(PAddr table);
void SwitchToAsm(struct Task *current, struct Task *next);
void RunFirstAsm(struct Task *next);
void FlushTLB();

#endif