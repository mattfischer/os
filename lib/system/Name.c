#include <System.h>
#include <Message.h>
#include <Object.h>

#include <kernel/include/Syscalls.h>
#include <kernel/include/NameFmt.h>

#include "Internal.h"

#include <string.h>
#include <stddef.h>

extern int __NameServer;

void Name_Set(const char *name, int obj)
{
	union NameMsg msg;
	struct BufferSegment segs[] = { &msg, sizeof(msg) };
	struct MessageHeader hdr = { segs, 1, offsetof(union NameMsg, msg.u.set.obj), 1 };

	msg.msg.type = NameMsgTypeSet;
	strcpy(msg.msg.u.set.name, name);
	msg.msg.u.set.obj = obj;

	while(__NameServer == OBJECT_INVALID) {
		__NameServer = Kernel_GetObject(KernelObjectNameServer);
	}

	Object_Sendxs(__NameServer, &hdr, NULL, 0);
}

int Name_Lookup(const char *name)
{
	struct MessageHeader send;
	union NameMsg msgSend;
	int object;
	struct BufferSegment segs[] = { &object, sizeof(object) };
	struct MessageHeader reply = { segs, 1, 0, 0};

	msgSend.msg.type = NameMsgTypeLookup;
	strcpy(msgSend.msg.u.lookup.name, name);

	while(__NameServer == OBJECT_INVALID) {
		__NameServer = Kernel_GetObject(KernelObjectNameServer);
	}

	Object_Sendsx(__NameServer, &msgSend, sizeof(msgSend), &reply);

	return object;
}