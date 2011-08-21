#include "Message.h"

#include "Object.h"
#include "Sched.h"

int SendMessage(int obj, void *msg, int msgSize, void *reply, int replySize)
{
	struct BufferSegment sendSegs[] = { msg, msgSize };
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return SendMessagex(obj, &sendMsg, &replyMsg);
}

int SendMessagexs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize)
{
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return SendMessagex(obj, sendMsg, &replyMsg);
}

int SendMessagesx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg)
{
	struct BufferSegment sendSegs[] = { msg, msgSize };
	struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };

	return SendMessagex(obj, &sendMsg, replyMsg);
}

int SendMessagex(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	struct Object *object = Sched::current()->process()->object(obj);
	return object->send(sendMsg, replyMsg);
}

int ReceiveMessage(int obj, void *recv, int recvSize)
{
	struct BufferSegment recvSegs[] = { recv, recvSize };
	struct MessageHeader recvMsg = { recvSegs, 1, 0, 0 };

	return ReceiveMessagex(obj, &recvMsg);
}

int ReceiveMessagex(int obj, struct MessageHeader *recvMsg)
{
	struct Object *object = Sched::current()->process()->object(obj);
	struct Message *message = object->receive(recvMsg);
	return Sched::current()->process()->refMessage(message);
}

int ReadMessage(int msg, void *buffer, int offset, int size)
{
	struct Message *message = Sched::current()->process()->message(msg);

	return message->read(buffer, offset, size);
}

int ReplyMessage(int msg, int ret, void *reply, int replySize)
{
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return ReplyMessagex(msg, ret, &replyMsg);
}

int ReplyMessagex(int msg, int ret, struct MessageHeader *replyMsg)
{
	struct Message *message = Sched::current()->process()->message(msg);
	int r = message->reply(ret, replyMsg);
	Sched::current()->process()->unrefMessage(msg);

	return r;
}
