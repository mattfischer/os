#include <lib/system/Message.h>

#include "private/Internal.h"
#include <kernel/include/Syscalls.h>

int SendMessage(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	return swi(SyscallSendMessage, (unsigned int)obj, (unsigned int)sendMsg, (unsigned int)replyMsg);
}

int ReceiveMessage(int obj, struct MessageHeader *recvMsg)
{
	return swi(SyscallReceiveMessage, (unsigned int)obj, (unsigned int)recvMsg, 0);
}

int ReplyMessage(int message, unsigned int ret, struct MessageHeader *replyMsg)
{
	return swi(SyscallReplyMessage, (unsigned int)message, ret, (unsigned int)replyMsg);
}
