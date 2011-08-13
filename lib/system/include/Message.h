#ifndef MESSAGE_H
#define MESSAGE_H

#include <kernel/include/MessageFmt.h>

int SendMessage(int obj, void *msg, int msgSize, void *reply, int replySize);
int SendMessagexs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize);
int SendMessagesx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg);
int SendMessagex(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);

int ReceiveMessage(int obj, void *recv, int recvSize);
int ReceiveMessagex(int obj, struct MessageHeader *recvMsg);

int ReplyMessage(int message, int ret, void *reply, int replySize);
int ReplyMessagex(int message, int ret, struct MessageHeader *replyMsg);

#endif