#ifndef MESSAGE_H
#define MESSAGE_H

#include <kernel/include/MessageFmt.h>

int SendMessage(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
int ReceiveMessage(int obj, struct MessageHeader *recvMsg);
int ReplyMessage(int message, unsigned int ret, struct MessageHeader *replyMsg);

#endif