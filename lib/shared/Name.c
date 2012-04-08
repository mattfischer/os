#include <Message.h>
#include <Object.h>
#include <Kernel.h>
#include <Name.h>

#include <kernel/include/NameFmt.h>

#include <string.h>
#include <stddef.h>

void Name_Set(const char *name, int obj)
{
	union NameMsg msg;
	struct BufferSegment segs[] = { &msg, sizeof(msg) };
	struct MessageHeader hdr = { segs, 1, offsetof(union NameMsg, msg.u.set.obj), 1 };
	int server;

	msg.msg.type = NameMsgTypeSet;
	strcpy(msg.msg.u.set.name, name);
	msg.msg.u.set.obj = obj;

	server = Kernel_GetObject(KernelObjectNameServer);

	Object_Sendxs(server, &hdr, NULL, 0);
	Object_Release(server);
}

int Name_Lookup(const char *name)
{
	struct MessageHeader send;
	union NameMsg msgSend;
	int object;
	int server;

	msgSend.msg.type = NameMsgTypeLookup;
	strcpy(msgSend.msg.u.lookup.name, name);

	server = Kernel_GetObject(KernelObjectNameServer);

	Object_Send(server, &msgSend, sizeof(msgSend), &object, sizeof(object));
	Object_Release(server);

	return object;
}

int Name_Open(const char *name)
{
	union NameMsg msg;
	int obj;
	int ret;
	int server;

	server = Kernel_GetObject(KernelObjectNameServer);

	msg.msg.type = NameMsgTypeOpen;
	strcpy(msg.msg.u.open.name, name);

	ret = Object_Send(server, &msg, sizeof(msg), &obj, sizeof(obj));
	Object_Release(server);

	return obj;
}

int Name_OpenDir(const char *name)
{
	union NameMsg msg;
	int obj;
	int ret;
	int server;

	server = Kernel_GetObject(KernelObjectNameServer);

	msg.msg.type = NameMsgTypeOpenDir;
	strcpy(msg.msg.u.open.name, name);

	ret = Object_Send(server, &msg, sizeof(msg), &obj, sizeof(obj));
	Object_Release(server);

	return obj;
}

void Name_Wait(const char *name)
{
	union NameMsg msg;
	int obj;
	int server;
	int ret;

	msg.msg.type = NameMsgTypeWait;
	strcpy(msg.msg.u.wait.name, name);

	ret = -1;
	while(ret == -1) {
		server = Kernel_GetObject(KernelObjectNameServer);
		ret = Object_Send(server, &msg, sizeof(msg), NULL, 0);
		Object_Release(server);
	}
}
