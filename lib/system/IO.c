#include <System.h>
#include <Message.h>

#include <kernel/include/IOFmt.h>

#include <stddef.h>

int Write(int obj, void *buffer, int size)
{
	struct IOMsg msg;
	struct BufferSegment segs[] = { &msg, sizeof(msg), buffer, size };
	struct MessageHeader hdr = { segs, 2, 0, 0 };
	int ret;

	msg.type = IOMsgTypeWrite;
	msg.u.write.size = size;

	ret = Object_Sendxs(obj, &hdr, NULL, 0);
	return ret;
}
