#ifndef OBJECT_H
#define OBJECT_H

#include "List.h"
#include "Task.h"
#include "Message.h"

#include <kernel/include/MessageFmt.h>

#define INVALID_OBJECT 0x7fffffff

class Object {
public:
	Object();

	int send(struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
	Message *receive(struct MessageHeader *recvMsg);

	void *operator new(size_t) { return sSlab.allocate(); }

private:
	List<Task> mReceivers;
	List<Message> mMessages;

	static Slab<Object> sSlab;
};

int Object_Create();
void Object_Release(int obj);

int Object_Send(int obj, void *msg, int msgSize, void *reply, int replySize);
int Object_Sendxs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize);
int Object_Sendsx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg);
int Object_Sendx(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);

int Object_Receive(int obj, void *recv, int recvSize);
int Object_Receivex(int obj, struct MessageHeader *recvMsg);

#endif