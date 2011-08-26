#ifndef SHARED_OBJECT_H
#define SHARED_OBJECT_H

#include <kernel/include/MessageFmt.h>

#define OBJECT_INVALID 0x7fffffff

#ifdef __cplusplus
extern "C" {
#endif

int Object_Create(int parent, void *data);
void Object_Release(int obj);

int Object_Send(int obj, void *msg, int msgSize, void *reply, int replySize);
int Object_Sendxs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize);
int Object_Sendsx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg);
int Object_Sendx(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);

int Object_Receive(int obj, void *recv, int recvSize);
int Object_Receivex(int obj, struct MessageHeader *recvMsg);

#ifdef __cplusplus
}
#endif

#endif