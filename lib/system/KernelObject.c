#include <System.h>

#include <kernel/include/Syscalls.h>
#include "Swi.h"

int Kernel_GetNameServer()
{
	return swi(SyscallKernelGetNameServer, 0, 0, 0, 0);
}

void Kernel_SetNameServer(int obj)
{
	swi(SyscallKernelSetNameServer, obj, 0, 0, 0);
}