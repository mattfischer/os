#include <Object.h>

#include <kernel/include/Syscalls.h>

#include "Swi.h"

int Object_Create(int parent, unsigned data)
{
	return swi(SyscallObjectCreate, (unsigned int)parent, (unsigned int)data, 0, 0);
}

void Object_Release(int obj)
{
	swi(SyscallObjectRelease, obj, 0, 0, 0);
}

int Object_Sendx(int obj, const struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	return swi(SyscallObjectSend, (unsigned int)obj, (unsigned int)sendMsg, (unsigned int)replyMsg, 0);
}
