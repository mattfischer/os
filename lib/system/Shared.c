#include "Shared.h"

#include <kernel/include/Syscalls.h>
#include <kernel/include/ProcManagerFmt.h>

#include <stddef.h>

int swi(unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);

int SendMessage(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	return swi(SyscallSendMessage, (unsigned int)obj, (unsigned int)sendMsg, (unsigned int)replyMsg);
}

int ReceiveMessage(int obj, struct MessageHeader *recvMsg)
{
	return swi(SyscallReceiveMessage, (unsigned int)obj, (unsigned int)recvMsg, 0);
}

int ReplyMessage(int message, struct MessageHeader *replyMsg)
{
	return swi(SyscallReplyMessage, (unsigned int)message, (unsigned int)replyMsg, 0);
}

int CreateObject()
{
	return swi(SyscallObjectCreate, 0, 0, 0);
}

void UnrefObject(int obj)
{
	swi(SyscallObjectUnref, obj, 0, 0);
}

char *strcpy(char *dest, const char *src)
{
	int i;

	for(i=0; src[i] != '\0'; i++) {
		dest[i] = src[i];
	}

	return dest;
}

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