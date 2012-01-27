#include <Message.h>
#include <Object.h>
#include <Kernel.h>

#include <kernel/include/NameFmt.h>

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

	msgSend.msg.type = NameMsgTypeLookup;
	strcpy(msgSend.msg.u.lookup.name, name);

	while(__NameServer == OBJECT_INVALID) {
		__NameServer = Kernel_GetObject(KernelObjectNameServer);
	}

	Object_Send(__NameServer, &msgSend, sizeof(msgSend), &object, sizeof(object));

	return object;
}

int Name_Open(const char *name)
{
	union NameMsg msg;
	int obj;
	int ret;

	while(__NameServer == OBJECT_INVALID) {
		__NameServer = Kernel_GetObject(KernelObjectNameServer);
	}

	msg.msg.type = NameMsgTypeOpen;
	strcpy(msg.msg.u.open.name, name);

	ret = Object_Send(__NameServer, &msg, sizeof(msg), &obj, sizeof(obj));

	return obj;
}