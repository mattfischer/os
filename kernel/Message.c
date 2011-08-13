#include "Message.h"

#include "Object.h"
#include "Sched.h"

int SendMessage(int obj, void *msg, int msgSize, void *reply, int replySize)
{
	struct MessageHeader sendMsg;
	struct MessageHeader replyMsg;

	sendMsg.size = msgSize;
	sendMsg.body = msg;
	sendMsg.objectsSize = 0;
	sendMsg.objectsOffset = 0;

	replyMsg.size = replySize;
	replyMsg.body = reply;
	replyMsg.objectsSize = 0;
	replyMsg.objectsOffset = 0;

	return SendMessagex(obj, &sendMsg, &replyMsg);
}

int SendMessagexs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize)
{
	struct MessageHeader replyMsg;

	replyMsg.size = replySize;
	replyMsg.body = reply;
	replyMsg.objectsSize = 0;
	replyMsg.objectsOffset = 0;

	return SendMessagex(obj, sendMsg, &replyMsg);
}

int SendMessagesx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg)
{
	struct MessageHeader sendMsg;

	sendMsg.size = msgSize;
	sendMsg.body = msg;
	sendMsg.objectsSize = 0;
	sendMsg.objectsOffset = 0;

	return SendMessagex(obj, &sendMsg, replyMsg);
}

int SendMessagex(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	struct Object *object = Current->process->objects[obj];
	return Object_SendMessage(object, sendMsg, replyMsg);
}

int ReceiveMessage(int obj, void *recv, int recvSize)
{
	struct MessageHeader recvMsg;

	recvMsg.size = recvSize;
	recvMsg.body = recv;
	recvMsg.objectsSize = 0;
	recvMsg.objectsOffset = 0;

	return ReceiveMessagex(obj, &recvMsg);
}

int ReceiveMessagex(int obj, struct MessageHeader *recvMsg)
{
	struct Object *object = Current->process->objects[obj];
	struct Message *message = Object_ReceiveMessage(object, recvMsg);
	return Process_RefMessage(Current->process, message);
}

int ReadMessage(int msg, void *buffer, int offset, int size)
{
	struct Message *message = Current->process->messages[msg];

	return Object_ReadMessage(message, buffer, offset, size);
}

int ReplyMessage(int msg, int ret, void *reply, int replySize)
{
	struct MessageHeader replyMsg;

	replyMsg.size = replySize;
	replyMsg.body = reply;
	replyMsg.objectsSize = 0;
	replyMsg.objectsOffset = 0;

	return ReplyMessagex(msg, ret, &replyMsg);
}

int ReplyMessagex(int msg, int ret, struct MessageHeader *replyMsg)
{
	struct Message *message = Current->process->messages[msg];
	int r = Object_ReplyMessage(message, ret, replyMsg);
	Process_UnrefMessage(Current->process, msg);

	return r;
}
