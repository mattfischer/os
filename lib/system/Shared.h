#ifndef SHARED_H
#define SHARED_H

#include <kernel/include/MessageFmt.h>

int SendMessage(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
int ReceiveMessage(int obj, struct MessageHeader *recvMsg);
int ReplyMessage(int message, struct MessageHeader *replyMsg);
int CreateObject();
void UnrefObject(int obj);
void SetName(const char *name, int obj);
int LookupName(const char *name);

#endif