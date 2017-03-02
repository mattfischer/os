#include <Message.h>
#include <Object.h>
#include <Name.h>

#include <kernel/include/NameFmt.h>

#include <string.h>
#include <stddef.h>

#define NAMESERVER_NO 4

void Name_Set(const char *name, int obj)
{
	union NameMsg msg;

	msg.msg.type = NameMsgTypeSet;
	strcpy(msg.msg.u.set.name, name);
	msg.msg.u.set.obj = obj;

	int objectsOffset = offsetof(union NameMsg, msg.u.set.obj);
	Object_Sendhs(NAMESERVER_NO, &msg, sizeof(msg), objectsOffset, 1, NULL, 0);
}

int Name_Lookup(const char *name)
{
	struct MessageHeader send;
	union NameMsg msgSend;
	int object;

	msgSend.msg.type = NameMsgTypeLookup;
	strcpy(msgSend.msg.u.lookup.name, name);

	Object_Send(NAMESERVER_NO, &msgSend, sizeof(msgSend), &object, sizeof(object));

	return object;
}

int Name_Open(const char *name)
{
	union NameMsg msg;
	int obj;
	int ret;

	msg.msg.type = NameMsgTypeOpen;
	strcpy(msg.msg.u.open.name, name);

	ret = Object_Send(NAMESERVER_NO, &msg, sizeof(msg), &obj, sizeof(obj));

	return obj;
}

int Name_OpenDir(const char *name)
{
	union NameMsg msg;
	int obj;
	int ret;

	msg.msg.type = NameMsgTypeOpenDir;
	strcpy(msg.msg.u.open.name, name);

	ret = Object_Send(NAMESERVER_NO, &msg, sizeof(msg), &obj, sizeof(obj));

	return obj;
}

void Name_Wait(const char *name)
{
	union NameMsg msg;
	int obj;
	int ret;

	msg.msg.type = NameMsgTypeWait;
	strcpy(msg.msg.u.wait.name, name);

	ret = -1;
	while(ret == -1) {
		ret = Object_Send(NAMESERVER_NO, &msg, sizeof(msg), NULL, 0);
	}
}
