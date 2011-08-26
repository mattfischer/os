#include <System.h>
#include <Message.h>

#include <kernel/include/IOFmt.h>

#include <stddef.h>

int Write(int obj, void *buffer, int size)
{
	union IOMsg msg;
	struct BufferSegment segs[] = { &msg, offsetof(union IOMsg, msg.u.write) + sizeof(struct IOMsgWriteHdr), buffer, size };
	struct MessageHeader hdr = { segs, 2, 0, 0 };
	int ret;

	msg.msg.type = IOMsgTypeWrite;
	msg.msg.u.write.size = size;

	ret = Object_Sendxs(obj, &hdr, NULL, 0);
	return ret;
}
