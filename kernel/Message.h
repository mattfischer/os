#ifndef MESSAGE_H
#define MESSAGE_H

#include "List.h"
#include "Task.h"
#include "include/MessageFmt.h"

#include <lib/shared/include/Message.h>

class MessageBase : public ListEntry {
public:
	enum Type {
		TypeMessage,
		TypeEvent
	};

	MessageBase(Type type, Task *sender, Object *target)
	 : mType(type),
	   mSender(sender),
	   mTarget(target)
	{}

	Type type() { return mType; }
	Task *sender() { return mSender; }
	Object *target() { return mTarget; }

	virtual int read(struct MessageHeader *header) = 0;

private:
	Type mType;
	Task *mSender;
	Object *mTarget;
};

class Message : public MessageBase {
public:
	Message(Task *sender, Object *target, struct MessageHeader &sendMsg, struct MessageHeader &replyMsg);

	struct MessageHeader &sendMsg() { return mSendMsg; }
	struct MessageHeader &replyMsg() { return mReplyMsg; }

	int ret() { return mRet; }

	int read(void *buffer, int offset, int size);
	virtual int read(struct MessageHeader *header);

	int reply(int ret, struct MessageHeader *replyMsg);

	void info(struct MessageInfo *info);

private:
	struct MessageHeader mSendMsg;
	struct MessageHeader mReplyMsg;
	int mRet;
	int mTranslateCache[MESSAGE_MAX_OBJECTS];
};

class MessageEvent : public MessageBase {
public:
	MessageEvent(Task *sender, Object *target, unsigned type, unsigned value)
	: MessageBase(TypeEvent, sender, target),
	  mType(type),
	  mValue(value)
	{}

	virtual int read(struct MessageHeader *header);

	void *operator new(size_t size) { return sSlab.allocate(); }
	void operator delete(void *p) { sSlab.free((MessageEvent*)p); }

private:
	unsigned mType;
	unsigned mValue;

	static Slab<MessageEvent> sSlab;
};
#endif