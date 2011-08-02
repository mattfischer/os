#ifndef OBJECT_H
#define OBJECT_H

#include "List.h"
#include "Task.h"

struct Message {
	struct Task *sender;
	void *sendBuf;
	int sendSize;
	void *replyBuf;
	int replySize;
	struct ListEntry list;
};

struct Object {
	LIST(struct Task) receivers;
	LIST(struct Message) messages;
};

struct Object *Object_Create();

int Object_SendMessage(struct Object *object, void *sendBuf, int sendSize, void *replyBuf, int replySize);
struct Message *Object_ReceiveMessage(struct Object *object, void *recvBuf, int recvSize);
int Object_ReplyMessage(struct Message *message, void *replyBuf, int replySize);

#endif