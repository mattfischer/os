#include <lib/system/Object.h>

#include "private/Internal.h"
#include <kernel/include/Syscalls.h>

int CreateObject()
{
	return swi(SyscallObjectCreate, 0, 0, 0);
}

void UnrefObject(int obj)
{
	swi(SyscallObjectUnref, obj, 0, 0);
}
