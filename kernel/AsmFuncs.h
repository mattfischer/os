#ifndef ASM_FUNCS_H
#define ASM_FUNCS_H

#include "Task.h"

extern "C" {
	void EnterUser(void (*userStart)(), void* userStack);
	void SetMMUBase(PAddr table);
	void SwitchToAsm(Task *current, Task *next);
	void RunFirstAsm(Task *next);
	void FlushTLB();
}

#endif