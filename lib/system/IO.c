#include "include/IO.h"

#include "include/Message.h"

#include <kernel/include/IOFmt.h>

#include <stddef.h>

int Write(int obj, void *buffer, int size)
{
	struct IOMsg msg;
	struct MessageHeader hdr;
	struct BufferSegment segs[] = { &msg, sizeof(msg), buffer, size };
	int ret;

	msg.type = IOMsgTypeWrite;
	msg.u.write.size = size;

	hdr.segments = segs;
	hdr.numSegments = 2;

	ret = SendMessagexs(obj, &hdr, NULL, 0);
	return ret;
}
