#include "include/Name.h"
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

	hdr.size = sizeof(msg);
	hdr.body = &msg;
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

	msgSend.type = ProcManagerNameLookup;
	strcpy(msgSend.u.lookup.name, name);

	reply.size = sizeof(object);
	reply.body = &object;

	SendMessagesx(__ProcessManager, &msgSend, sizeof(msgSend), &reply);

	return object;
}