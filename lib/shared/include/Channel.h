#ifndef SHARED_CHANNEL_H
#define SHARED_CHANNEL_H

#include <kernel/include/MessageFmt.h>

#ifdef __cplusplus
extern "C" {
#endif

int Channel_Create();
void Channel_Destroy(int chan);
int Channel_Receive(int chan, void *recv, int recvSize, unsigned *targetData);
int Channel_Receivex(int chan, struct MessageHeader *recvMsg, unsigned *targetData);

#ifdef __cplusplus
}
#endif

#endif
