#include <Object.h>

#include <kernel/include/Syscalls.h>

#include "Swi.h"

int Object_Create(int parent, void *data)
{
	return swi(SyscallObjectCreate, (unsigned int)parent, (unsigned int)data, 0, 0);
}

void Object_Release(int obj)
{
	swi(SyscallObjectRelease, obj, 0, 0, 0);
}

int Object_Sendx(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	return swi(SyscallObjectSend, (unsigned int)obj, (unsigned int)sendMsg, (unsigned int)replyMsg, 0);
}

int Object_Receivex(int obj, struct MessageHeader *recvMsg)
{
	return swi(SyscallObjectReceive, (unsigned int)obj, (unsigned int)recvMsg, 0, 0);
}
