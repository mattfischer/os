#include <IO.h>
#include <Object.h>
#include <Name.h>
#include <kernel/include/IOFmt.h>
#include <kernel/include/NameFmt.h>

#include <stddef.h>
#include <string.h>

int File_Write(int obj, void *buffer, int size)
{
	struct IOMsg msg;
	struct BufferSegment segs[] = { &msg, offsetof(struct IOMsg, rw) + sizeof(struct IOMsgReadWriteHdr), buffer, size };
	struct MessageHeader hdr = { segs, 2, 0, 0 };
	int ret;

	msg.type = IOMsgTypeWrite;
	msg.rw.size = size;

	ret = Object_Sendxs(obj, &hdr, NULL, 0);
	return ret;
}

int File_Read(int obj, void *buffer, int size)
{
	struct IOMsg msg;
	int ret;

	msg.type = IOMsgTypeRead;
	msg.rw.size = size;

	ret = Object_Send(obj, &msg, sizeof(msg), buffer, size);
	return ret;
}

void File_Seek(int obj, int pointer)
{
	struct IOMsg msg;

	msg.type = IOMsgTypeSeek;
	msg.seek.pointer = pointer;

	Object_Send(obj, &msg, sizeof(msg), NULL, 0);
}

int File_ReadDir(int obj, char *name)
{
	struct IOMsg msg;
	struct IOMsgReadDirRet ret;
	int status;

	msg.type = IOMsgTypeReadDir;

	status = Object_Send(obj, &msg, sizeof(msg), &ret, sizeof(ret));
	strcpy(name, ret.name);

	return status;
}