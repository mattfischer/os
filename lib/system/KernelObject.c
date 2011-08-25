#include "include/System.h"

#include <kernel/include/Syscalls.h>
#include "Swi.h"

int GetKernelObject(enum KernelObject idx)
{
	return swi(SyscallGetObject, idx, 0, 0, 0);
}

void SetKernelObject(enum KernelObject idx, int obj)
{
	swi(SyscallSetObject, idx, obj, 0, 0);
}