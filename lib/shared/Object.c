#include <Object.h>

int Object_Send(int obj, const void *msg, int msgSize, void *reply, int replySize)
{
	struct BufferSegment sendSegs[] = { (void*)msg, msgSize };
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
	struct BufferSegment sendSegs[] = { (void*)msg, msgSize };
	const struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };

	return Object_Sendx(obj, &sendMsg, replyMsg);
}

int Object_Sendhs(int obj, const void *msg, int msgSize, int objectsOffset, int objectsSize, void *reply, int replySize)
{
	struct BufferSegment sendSegs[] = { (void*)msg, msgSize };
	const struct MessageHeader sendMsg = { sendSegs, 1, objectsOffset, objectsSize };

	return Object_Sendxs(obj, &sendMsg, reply, replySize);
}

int Object_Sendhx(int obj, const void *msg, int msgSize, int objectsOffset, int objectsSize, struct MessageHeader *replyMsg)
{
	struct BufferSegment sendSegs[] = { (void*)msg, msgSize };
	const struct MessageHeader sendMsg = { sendSegs, 1, objectsOffset, objectsSize };

	return Object_Sendx(obj, &sendMsg, replyMsg);
}
