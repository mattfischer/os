#ifndef SHARED_MESSAGE_H
#define SHARED_MESSAGE_H

#include <kernel/include/MessageFmt.h>

#ifdef __cplusplus
extern "C" {
#endif

int Message_Read(int msg, void *buffer, int offset, int size);

int Message_Reply(int msg, int ret, const void *reply, int replySize);
int Message_Replyh(int msg, int ret, const void *reply, int replySize, int objectsOffset, int objectsSize);
int Message_Replyx(int msg, int ret, const struct MessageHeader *replyMsg);

#ifdef __cplusplus
}
#endif

#endif