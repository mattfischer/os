#include <IO.h>
#include <Object.h>
#include <Name.h>
#include <kernel/include/IOFmt.h>
#include <kernel/include/NameFmt.h>

#include <stddef.h>
#include <string.h>

int File_Write(int obj, void *buffer, int size)
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

