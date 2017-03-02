#include <Message.h>

#include <kernel/include/Syscalls.h>
#include "Swi.h"
int Message_Read(int msg, void *buffer, int offset, int size)
{
	return swi(SyscallMessageRead, (unsigned int)msg, (unsigned int)buffer, (unsigned int)offset, (unsigned int)size);
}

int Message_Replyx(int msg, int ret, const struct MessageHeader *replyMsg)
{
	return swi(SyscallMessageReply, (unsigned int)msg, (unsigned int)ret, (unsigned int)replyMsg, 0);
}
