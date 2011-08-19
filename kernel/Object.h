#ifndef OBJECT_H
#define OBJECT_H

#include "List.h"
#include "Task.h"

#include <kernel/include/MessageFmt.h>

#define INVALID_OBJECT 0x7fffffff

struct Message {
	struct Task *sender;
	struct Task *receiver;
	struct MessageHeader sendMsg;
	struct MessageHeader replyMsg;
	int ret;
	struct ListEntry list;
	int translateCache[MESSAGE_MAX_OBJECTS];
};

struct Object {
	LIST(struct Task) receivers;
	LIST(struct Message) messages;
};

struct Object *Object_Create();

int Object_SendMessage(struct Object *object, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
struct Message *Object_ReceiveMessage(struct Object *object, struct MessageHeader *recvMsg);
int Object_ReadMessage(struct Message *message, void *buffer, int offset, int size);
int Object_ReplyMessage(struct Message *message, int ret, struct MessageHeader *replyMsg);

int CreateObject();
void ReleaseObject(int obj);

#endif