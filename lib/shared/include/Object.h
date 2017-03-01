#ifndef SHARED_OBJECT_H
#define SHARED_OBJECT_H

#include <kernel/include/MessageFmt.h>

#define OBJECT_INVALID 0x7fffffff

#ifdef __cplusplus
extern "C" {
#endif

int Object_Create(int parent, void *data);
void Object_Release(int obj);

int Object_Send(int obj, const void *msg, int msgSize, void *reply, int replySize);
int Object_Sendxs(int obj, const struct MessageHeader *sendMsg, void *reply, int replySize);
int Object_Sendhs(int obj, const void *msg, int msgSize, int objectsOffset, int objectsSize, void *reply, int replySize);
int Object_Sendsx(int obj, const void *msg, int msgSize, struct MessageHeader *replyMsg);
int Object_Sendhx(int obj, const void *msg, int msgSize, int objectsOffset, int objectsSize, struct MessageHeader *replyMsg);
int Object_Sendx(int obj, const struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);

int Object_Post(int obj, unsigned type, unsigned value);

#ifdef __cplusplus
}
#endif

#endif