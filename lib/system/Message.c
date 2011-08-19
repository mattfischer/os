#include "include/Message.h"
#include "Swi.h"

#include <kernel/include/Syscalls.h>

int SendMessage(int obj, void *msg, int msgSize, void *reply, int replySize)
{
	struct MessageHeader sendMsg;
	struct BufferSegment sendSegs[] = { msg, msgSize };
	struct MessageHeader replyMsg;
	struct BufferSegment replySegs[] = { reply, replySize };

	sendMsg.segments = sendSegs;
	sendMsg.numSegments = 1;
	sendMsg.objectsSize = 0;
	sendMsg.objectsOffset = 0;

	replyMsg.segments = replySegs;
	replyMsg.numSegments = 1;
	replyMsg.objectsSize = 0;
	replyMsg.objectsOffset = 0;

	return SendMessagex(obj, &sendMsg, &replyMsg);
}

int SendMessagexs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize)
{
	struct MessageHeader replyMsg;
	struct BufferSegment replySegs[] = { reply, replySize };

	replyMsg.segments = replySegs;
	replyMsg.numSegments = 1;
	replyMsg.objectsSize = 0;
	replyMsg.objectsOffset = 0;

	return SendMessagex(obj, sendMsg, &replyMsg);
}

int SendMessagesx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg)
{
	struct MessageHeader sendMsg;
	struct BufferSegment sendSegs[] = { msg, msgSize };

	sendMsg.segments = sendSegs;
	sendMsg.numSegments = 1;
	sendMsg.objectsSize = 0;
	sendMsg.objectsOffset = 0;

	return SendMessagex(obj, &sendMsg, replyMsg);

}

int SendMessagex(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	return swi(SyscallSendMessage, (unsigned int)obj, (unsigned int)sendMsg, (unsigned int)replyMsg, 0);
}

int ReceiveMessage(int obj, void *recv, int recvSize)
{
	struct MessageHeader recvMsg;
	struct BufferSegment recvSegs[] = { recv, recvSize };

	recvMsg.segments = recvSegs;
	recvMsg.numSegments = 1;
	recvMsg.objectsSize = 0;
	recvMsg.objectsOffset = 0;

	return ReceiveMessagex(obj, &recvMsg);
}

int ReceiveMessagex(int obj, struct MessageHeader *recvMsg)
{
	return swi(SyscallReceiveMessage, (unsigned int)obj, (unsigned int)recvMsg, 0, 0);
}

int ReadMessage(int msg, void *buffer, int offset, int size)
{
	return swi(SyscallReadMessage, (unsigned int)msg, (unsigned int)buffer, (unsigned int)offset, (unsigned int)size);
}

int ReplyMessage(int msg, int ret, void *reply, int replySize)
{
	struct MessageHeader replyMsg;
	struct BufferSegment replySegs[] = { reply, replySize };

	replyMsg.segments = replySegs;
	replyMsg.numSegments = 1;
	replyMsg.objectsSize = 0;
	replyMsg.objectsOffset = 0;

	return ReplyMessagex(msg, ret, &replyMsg);
}

int ReplyMessagex(int msg, int ret, struct MessageHeader *replyMsg)
{
	return swi(SyscallReplyMessage, (unsigned int)msg, (unsigned int)ret, (unsigned int)replyMsg, 0);
}
