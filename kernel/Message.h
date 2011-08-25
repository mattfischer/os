#ifndef MESSAGE_H
#define MESSAGE_H

#include "List.h"
#include "Task.h"
#include "include/MessageFmt.h"

class Message : public ListEntry {
public:
	Message(Task *sender, struct MessageHeader &sendMsg, struct MessageHeader &replyMsg);

	Task *sender() { return mSender; }

	struct MessageHeader &sendMsg() { return mSendMsg; }
	struct MessageHeader &replyMsg() { return mReplyMsg; }

	int ret() { return mRet; }

	int read(void *buffer, int offset, int size);
	int read(struct MessageHeader *header);
	int reply(int ret, struct MessageHeader *replyMsg);

private:
	Task *mSender;
	struct MessageHeader mSendMsg;
	struct MessageHeader mReplyMsg;
	int mRet;
	int mTranslateCache[MESSAGE_MAX_OBJECTS];
};

int Message_Read(int msg, void *buffer, int offset, int size);

int Message_Reply(int msg, int ret, void *reply, int replySize);
int Message_Replyx(int msg, int ret, struct MessageHeader *replyMsg);

#endif