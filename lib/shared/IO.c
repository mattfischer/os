#include <IO.h>
#include <Object.h>
#include <Name.h>
#include <kernel/include/IOFmt.h>
#include <kernel/include/NameFmt.h>

#include <stddef.h>
#include <string.h>

int File_Open(const char *name)
{
	int entry;
	union NameMsg msg;
	int obj;
	int ret;

	entry = Name_Lookup(name);

	msg.msg.type = NameMsgTypeOpen;
	strcpy(msg.msg.u.open.name, name);

	ret = Object_Send(entry, &msg, sizeof(msg), &obj, sizeof(obj));
	Object_Release(entry);

	return obj;
}

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

