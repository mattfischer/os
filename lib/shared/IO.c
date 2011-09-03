#include <IO.h>
#include <Object.h>
#include <Name.h>
#include <kernel/include/IOFmt.h>
#include <kernel/include/NameFmt.h>

#include <stddef.h>
#include <string.h>

int File_Open(char *name)
{
	int entry = Name_Lookup(name);
	union NameEntryMsg msg;
	int obj;
	struct BufferSegment segs[] = { &obj, sizeof(obj) };
	struct MessageHeader hdr = { segs, 1, 0, 0 };
	int ret;

	msg.msg.type = NameEntryMsgTypeOpen;
	strcpy(msg.msg.u.open.name, name);

	ret = Object_Sendsx(entry, &msg, sizeof(msg), &hdr);
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

