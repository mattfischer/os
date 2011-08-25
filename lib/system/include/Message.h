#ifndef MESSAGE_H
#define MESSAGE_H

#include <kernel/include/MessageFmt.h>

int Message_Read(int msg, void *buffer, int offset, int size);

int Message_Reply(int msg, int ret, void *reply, int replySize);
int Message_Replyx(int msg, int ret, struct MessageHeader *replyMsg);

#endif