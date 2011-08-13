#include "include/Name.h"
#include "include/Message.h"

#include <kernel/include/Syscalls.h>
#include <kernel/include/ProcManagerFmt.h>

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
	SendMessage(0, &hdr, NULL);
}

int LookupName(const char *name)
{
	struct MessageHeader send;
	struct ProcManagerMsg msgSend;
	struct MessageHeader reply;
	struct ProcManagerMsgNameLookupReply msgReply;

	send.size = sizeof(msgSend);
	send.body = &msgSend;
	send.objectsSize = 0;
	send.objectsOffset = 0;

	msgSend.type = ProcManagerNameLookup;
	strcpy(msgSend.u.lookup.name, name);

	reply.size = sizeof(msgReply);
	reply.body = &msgReply;

	SendMessage(0, &send, &reply);

	return msgReply.obj;
}