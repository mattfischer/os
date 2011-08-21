#include "include/System.h"
#include "include/Message.h"

#include <kernel/include/Syscalls.h>
#include <kernel/include/ProcManagerFmt.h>

#include "Internal.h"

#include <string.h>
#include <stddef.h>

void SetName(const char *name, int obj)
{
	struct MessageHeader hdr;
	struct ProcManagerMsg msg;
	struct BufferSegment segs[] = { &msg, sizeof(msg) };

	hdr.segments = segs;
	hdr.numSegments = 1;
	hdr.objectsSize = 1;
	hdr.objectsOffset = offsetof(struct ProcManagerMsg, u.set.obj);

	msg.type = ProcManagerNameSet;
	strcpy(msg.u.set.name, name);
	msg.u.set.obj = obj;
	SendMessagexs(__ProcessManager, &hdr, NULL, 0);
}

int LookupName(const char *name)
{
	struct MessageHeader send;
	struct ProcManagerMsg msgSend;
	struct MessageHeader reply;
	int object;
	struct BufferSegment segs[] = { &object, sizeof(object) };

	msgSend.type = ProcManagerNameLookup;
	strcpy(msgSend.u.lookup.name, name);

	reply.segments = segs;
	reply.numSegments = 1;

	SendMessagesx(__ProcessManager, &msgSend, sizeof(msgSend), &reply);

	return object;
}