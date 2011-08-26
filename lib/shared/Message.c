#include <Message.h>

int Message_Reply(int msg, int ret, void *reply, int replySize)
{
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return Message_Replyx(msg, ret, &replyMsg);
}
