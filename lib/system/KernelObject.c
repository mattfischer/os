#include "include/System.h"

#include <kernel/include/Syscalls.h>
#include "Swi.h"

int Kernel_GetObject(enum KernelObject idx)
{
	return swi(SyscallKernelGetObject, idx, 0, 0, 0);
}

void Kernel_SetObject(enum KernelObject idx, int obj)
{
	swi(SyscallKernelSetObject, idx, obj, 0, 0);
}