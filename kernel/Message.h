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

int SendMessage(int obj, void *msg, int msgSize, void *reply, int replySize);
int SendMessagexs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize);
int SendMessagesx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg);
int SendMessagex(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);

int ReceiveMessage(int obj, void *recv, int recvSize);
int ReceiveMessagex(int obj, struct MessageHeader *recvMsg);

int ReadMessage(int msg, void *buffer, int offset, int size);

int ReplyMessage(int msg, int ret, void *reply, int replySize);
int ReplyMessagex(int msg, int ret, struct MessageHeader *replyMsg);

#endif