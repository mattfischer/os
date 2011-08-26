#ifndef OBJECT_H
#define OBJECT_H

#include "List.h"
#include "Task.h"
#include "Message.h"

#include <lib/shared/include/Object.h>

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

#endif