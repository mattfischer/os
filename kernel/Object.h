#ifndef OBJECT_H
#define OBJECT_H

#include "List.h"
#include "Task.h"
#include "Message.h"

#include <kernel/include/MessageFmt.h>

#define OBJECT_INVALID 0x7fffffff

class Object : public ListEntry {
public:
	Object(Object *parent, void *data);

	int send(struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
	Message *receive(struct MessageHeader *recvMsg);

	Object *parent() { return mParent; }
	void *data() { return mData; }

	void *operator new(size_t) { return sSlab.allocate(); }

private:
	List<Task> mReceivers;
	List<Message> mMessages;
	List<Object> mSendingChildren;
	void *mData;
	Object *mParent;

	static Slab<Object> sSlab;
};

int Object_Create(int parent, void *data);
void Object_Release(int obj);

int Object_Send(int obj, void *msg, int msgSize, void *reply, int replySize);
int Object_Sendxs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize);
int Object_Sendsx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg);
int Object_Sendx(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);

int Object_Receive(int obj, void *recv, int recvSize);
int Object_Receivex(int obj, struct MessageHeader *recvMsg);

#endif