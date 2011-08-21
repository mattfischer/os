#include "include/System.h"

#include <kernel/include/Syscalls.h>
#include "Swi.h"

void Yield()
{
	swi(SyscallYield, 0, 0, 0, 0);
}