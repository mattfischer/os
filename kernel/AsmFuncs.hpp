#ifndef ASM_FUNCS_H
#define ASM_FUNCS_H

#include "Task.hpp"

extern "C" {
	void EnterUser(void (*userStart)(), void* userStack);
	void SetMMUBase(PAddr table);
	void SwitchToAsm(unsigned *regsCurrent, unsigned *regsNext);
	void RunFirstAsm(unsigned *regs);
	void FlushTLB();
}

#endif