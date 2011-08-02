#ifndef OBJECT_H
#define OBJECT_H

#include "List.h"
#include "Task.h"

#include <kernel/include/MessageFmt.h>

struct Message {
	struct Task *sender;
	struct MessageHeader sendMsg;
	struct MessageHeader replyMsg;
	struct ListEntry list;
};

struct Object {
	LIST(struct Task) receivers;
	LIST(struct Message) messages;
};

struct Object *Object_Create();

int Object_SendMessage(struct Object *object, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
struct Message *Object_ReceiveMessage(struct Object *object, struct MessageHeader *recvMsg);
int Object_ReplyMessage(struct Message *message, struct MessageHeader *replyMsg);

#endif