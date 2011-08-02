#include <kernel/include/Syscalls.h>
#include <kernel/include/MessageFmt.h>

int swi(unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);

int SendMessage(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	return swi(SyscallSendMessage, (unsigned int)obj, (unsigned int)sendMsg, (unsigned int)replyMsg);
}

void UnrefObject(int obj)
{
	swi(SyscallObjectUnref, obj, 0, 0);
}

struct Msg {
	int x;
	int obj;
};

void _start()
{
	int x;

	x = 0;
	while(1) {
		struct MessageHeader header;
		struct Msg msg;

		header.size = sizeof(struct Msg);
		header.body = &msg;
		header.objectsOffset = 0;
		header.objectsSize = 0;

		msg.x = x;

		SendMessage(0, &header, &header);

		x = msg.x;
		UnrefObject(msg.obj);
	}
}