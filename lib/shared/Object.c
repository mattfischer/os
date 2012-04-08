#include <Object.h>

int Object_Send(int obj, const void *msg, int msgSize, void *reply, int replySize)
{
	const struct BufferSegment sendSegs[] = { msg, msgSize };
	const struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return Object_Sendx(obj, &sendMsg, &replyMsg);
}

int Object_Sendxs(int obj, const struct MessageHeader *sendMsg, void *reply, int replySize)
{
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return Object_Sendx(obj, sendMsg, &replyMsg);
}

int Object_Sendsx(int obj, const void *msg, int msgSize, struct MessageHeader *replyMsg)
{
	const struct BufferSegment sendSegs[] = { msg, msgSize };
	const struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };

	return Object_Sendx(obj, &sendMsg, replyMsg);
}

int Object_Receive(int obj, void *recv, int recvSize)
{
	struct BufferSegment recvSegs[] = { recv, recvSize };
	struct MessageHeader recvMsg = { recvSegs, 1, 0, 0 };

	return Object_Receivex(obj, &recvMsg);
}