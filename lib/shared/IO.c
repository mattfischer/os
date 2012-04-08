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
	struct BufferSegment segs[] = { &msg, offsetof(union IOMsg, msg.u.rw) + sizeof(struct IOMsgReadWriteHdr), buffer, size };
	struct MessageHeader hdr = { segs, 2, 0, 0 };
	int ret;

	msg.msg.type = IOMsgTypeWrite;
	msg.msg.u.rw.size = size;

	ret = Object_Sendxs(obj, &hdr, NULL, 0);
	return ret;
}

int File_Read(int obj, void *buffer, int size)
{
	union IOMsg msg;
	int ret;

	msg.msg.type = IOMsgTypeRead;
	msg.msg.u.rw.size = size;

	ret = Object_Send(obj, &msg, sizeof(msg), buffer, size);
	return ret;
}

void File_Seek(int obj, int pointer)
{
	union IOMsg msg;

	msg.msg.type = IOMsgTypeSeek;
	msg.msg.u.seek.pointer = pointer;

	Object_Send(obj, &msg, sizeof(msg), NULL, 0);
}

int File_ReadDir(int obj, char *name)
{
	union IOMsg msg;
	struct IOMsgReadDirRet ret;
	int status;

	msg.msg.type = IOMsgTypeReadDir;

	status = Object_Send(obj, &msg, sizeof(msg), &ret, sizeof(ret));
	strcpy(name, ret.name);

	return status;
}