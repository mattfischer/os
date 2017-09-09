#include <Message.h>
#include <Object.h>
#include <Name.h>

#include <kernel/include/NameFmt.h>
#include <kernel/include/Objects.h>

#include <string.h>
#include <stddef.h>

void Name_Set(const char *name, int obj)
{
	struct NameMsg msg;

	msg.type = NameMsgTypeSet;
	strcpy(msg.set.name, name);
	msg.set.obj = obj;

	int objectsOffset = offsetof(struct NameMsg, set.obj);
	Object_Sendhs(NAMESERVER_NO, &msg, sizeof(msg), objectsOffset, 1, NULL, 0);
}

int Name_Lookup(const char *name)
{
	struct MessageHeader send;
	struct NameMsg msgSend;
	int object;

	msgSend.type = NameMsgTypeLookup;
	strcpy(msgSend.lookup.name, name);

	Object_Send(NAMESERVER_NO, &msgSend, sizeof(msgSend), &object, sizeof(object));

	return object;
}

int Name_Open(const char *name)
{
	struct NameMsg msg;
	int obj;
	int ret;

	msg.type = NameMsgTypeOpen;
	strcpy(msg.open.name, name);

	ret = Object_Send(NAMESERVER_NO, &msg, sizeof(msg), &obj, sizeof(obj));

	return obj;
}

int Name_OpenDir(const char *name)
{
	struct NameMsg msg;
	int obj;
	int ret;

	msg.type = NameMsgTypeOpenDir;
	strcpy(msg.open.name, name);

	ret = Object_Send(NAMESERVER_NO, &msg, sizeof(msg), &obj, sizeof(obj));

	return obj;
}

void Name_Wait(const char *name)
{
	struct NameMsg msg;
	int obj;
	int ret;

	msg.type = NameMsgTypeWait;
	strcpy(msg.wait.name, name);

	ret = -1;
	while(ret == -1) {
		ret = Object_Send(NAMESERVER_NO, &msg, sizeof(msg), NULL, 0);
	}
}
