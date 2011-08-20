#ifndef OBJECT_H
#define OBJECT_H

#include "List.h"
#include "Task.h"

#include <kernel/include/MessageFmt.h>

#define INVALID_OBJECT 0x7fffffff

class Message {
public:
	Message(Task *sender, struct MessageHeader &sendMsg, struct MessageHeader &replyMsg);

	Task *sender() { return mSender; }
	Task *receiver() { return mReceiver; }
	void setReceiver(Task *receiver) { mReceiver = receiver; }

	struct MessageHeader &sendMsg() { return mSendMsg; }
	struct MessageHeader &replyMsg() { return mReplyMsg; }

	int ret() { return mRet; }
	int *translateCache() { return mTranslateCache; }

	int read(void *buffer, int offset, int size);
	int reply(int ret, struct MessageHeader *replyMsg);

private:
	Task *mSender;
	Task *mReceiver;
	struct MessageHeader mSendMsg;
	struct MessageHeader mReplyMsg;
	int mRet;
	int mTranslateCache[MESSAGE_MAX_OBJECTS];

public:
	struct ListEntry<struct Message> list;
};

class Object {
public:
	Object();

	int send(struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
	struct Message *receive(struct MessageHeader *recvMsg);

	void *operator new(size_t) { return sSlab.allocate(); }

private:
	List<Task, &Task::list> mReceivers;
	List<struct Message, &Message::list> mMessages;

	static SlabAllocator<Object> sSlab;
};

int CreateObject();
void ReleaseObject(int obj);

#endif