#include "include/IO.h"

#include "include/Message.h"

#include <kernel/include/IOFmt.h>

#include <malloc.h>
#include <string.h>
#include <stddef.h>

int Write(int obj, void *buffer, int size)
{
	int msgSize = sizeof(struct IOMsg) + size - 1;
	char *msgBuf = malloc(msgSize);
	struct IOMsg *msg = (struct IOMsg*)msgBuf;
	int ret;

	msg->type = IOMsgTypeWrite;
	msg->u.write.size = size;
	memcpy(msg->u.write.data, buffer, size);
	ret = SendMessage(obj, msg, msgSize, NULL, 0);
	free(msgBuf);
	return ret;
}
