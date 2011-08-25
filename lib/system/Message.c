#include "include/Message.h"
#include "Swi.h"

#include <kernel/include/Syscalls.h>

int Message_Read(int msg, void *buffer, int offset, int size)
{
	return swi(SyscallMessageRead, (unsigned int)msg, (unsigned int)buffer, (unsigned int)offset, (unsigned int)size);
}

int Message_Reply(int msg, int ret, void *reply, int replySize)
{
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return Message_Replyx(msg, ret, &replyMsg);
}

int Message_Replyx(int msg, int ret, struct MessageHeader *replyMsg)
{
	return swi(SyscallMessageReply, (unsigned int)msg, (unsigned int)ret, (unsigned int)replyMsg, 0);
}

void Message_Info(int msg, struct MessageInfo *info)
{
	swi(SyscallMessageInfo, (unsigned int)msg, (unsigned int)info, 0, 0);
}