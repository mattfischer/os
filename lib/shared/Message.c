#include <Message.h>

int Message_Reply(int msg, int ret, const void *reply, int replySize)
{
	const struct BufferSegment replySegs[] = { reply, replySize };
	const struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return Message_Replyx(msg, ret, &replyMsg);
}
