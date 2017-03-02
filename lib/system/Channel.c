#include <Object.h>

#include <kernel/include/Syscalls.h>

#include "Swi.h"

int Channel_Create()
{
	return swi(SyscallChannelCreate, 0, 0, 0, 0);
}

void Channel_Destroy(int chan)
{
	swi(SyscallChannelDestroy, (unsigned int)chan, 0, 0, 0);
}

int Channel_Receivex(int chan, struct MessageHeader *recvMsg, struct MessageInfo *info)
{
	return swi(SyscallChannelReceive, (unsigned int)chan, (unsigned int)recvMsg, (unsigned int)info, 0);
}
