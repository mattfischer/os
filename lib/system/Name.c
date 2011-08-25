#include "include/System.h"
#include "include/Message.h"
#include "include/Object.h"

#include <kernel/include/Syscalls.h>
#include <kernel/include/NameFmt.h>
#include "Internal.h"

#include <kernel/include/NameFmt.h>
#include <kernel/include/MessageFmt.h>

#include <string.h>
#include <stddef.h>

extern int __NameServer;

void Name_Set(const char *name, int obj)
{
	struct NameMsg msg;
	struct BufferSegment segs[] = { &msg, sizeof(msg) };
	struct MessageHeader hdr = { segs, 1, offsetof(struct NameMsg, u.set.obj), 1 };

	msg.type = NameMsgTypeSet;
	strcpy(msg.u.set.name, name);
	msg.u.set.obj = obj;

	while(__NameServer == INVALID_OBJECT) {
		__NameServer = Kernel_GetObject(KernelObjectNameServer);
	}

	Object_Sendxs(__NameServer, &hdr, NULL, 0);
}

int Name_Lookup(const char *name)
{
	struct MessageHeader send;
	struct NameMsg msgSend;
	int object;
	struct BufferSegment segs[] = { &object, sizeof(object) };
	struct MessageHeader reply = { segs, 1, 0, 0};

	msgSend.type = NameMsgTypeLookup;
	strcpy(msgSend.u.lookup.name, name);

	while(__NameServer == INVALID_OBJECT) {
		__NameServer = Kernel_GetObject(KernelObjectNameServer);
	}

	Object_Sendsx(__NameServer, &msgSend, sizeof(msgSend), &reply);

	return object;
}