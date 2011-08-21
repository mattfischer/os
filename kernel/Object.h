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

	static SlabAllocator<Object> sSlab;
};

int CreateObject();
void ReleaseObject(int obj);

#endif