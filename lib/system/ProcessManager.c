#include "include/ProcessManager.h"

#include <kernel/include/Syscalls.h>
#include "Swi.h"

int GetProcessManager()
{
	return swi(SyscallGetProcessManager, 0, 0, 0, 0);
}