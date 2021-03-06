#include <Message.h>

int Message_Reply(int msg, int ret, const void *reply, int replySize)
{
	struct BufferSegment replySegs[] = { (void*)reply, replySize };
	const struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return Message_Replyx(msg, ret, &replyMsg);
}

int Message_Replyh(int msg, int ret, const void *reply, int replySize, int objectsOffset, int objectsSize)
{
	struct BufferSegment replySegs[] = { (void*)reply, replySize };
	const struct MessageHeader replyMsg = { replySegs, 1, objectsOffset, objectsSize };

	return Message_Replyx(msg, ret, &replyMsg);
}