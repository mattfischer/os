#include "include/Object.h"
#include "Swi.h"

#include <kernel/include/Syscalls.h>

int Object_Create(void *data)
{
	return swi(SyscallObjectCreate, (unsigned int)data, 0, 0, 0);
}

void Object_Release(int obj)
{
	swi(SyscallObjectRelease, obj, 0, 0, 0);
}

int Object_Send(int obj, void *msg, int msgSize, void *reply, int replySize)
{
	struct BufferSegment sendSegs[] = { msg, msgSize };
	struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return Object_Sendx(obj, &sendMsg, &replyMsg);
}

int Object_Sendxs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize)
{
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return Object_Sendx(obj, sendMsg, &replyMsg);
}

int Object_Sendsx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg)
{
	struct BufferSegment sendSegs[] = { msg, msgSize };
	struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };

	return Object_Sendx(obj, &sendMsg, replyMsg);
}

int Object_Sendx(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	return swi(SyscallObjectSend, (unsigned int)obj, (unsigned int)sendMsg, (unsigned int)replyMsg, 0);
}

int Object_Receive(int obj, void *recv, int recvSize)
{
	struct BufferSegment recvSegs[] = { recv, recvSize };
	struct MessageHeader recvMsg = { recvSegs, 1, 0, 0 };

	return Object_Receivex(obj, &recvMsg);
}

int Object_Receivex(int obj, struct MessageHeader *recvMsg)
{
	return swi(SyscallObjectReceive, (unsigned int)obj, (unsigned int)recvMsg, 0, 0);
}
