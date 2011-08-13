#include "include/Object.h"
#include "Swi.h"

#include <kernel/include/Syscalls.h>

int CreateObject()
{
	return swi(SyscallCreateObject, 0, 0, 0);
}

void ReleaseObject(int obj)
{
	swi(SyscallReleaseObject, obj, 0, 0);
}
