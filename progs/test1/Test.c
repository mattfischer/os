#include <kernel/include/Syscalls.h>
#include <kernel/include/MessageFmt.h>

int swi(unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);

int SendMessage(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	return swi(SyscallSendMessage, (unsigned int)obj, (unsigned int)sendMsg, (unsigned int)replyMsg);
}

void _start()
{
	int x;
	struct MessageHeader header;
	int r;

	x = 0;
	while(1) {
		header.size = sizeof(int);
		header.body = &x;
		header.objectsOffset = 0;
		header.objectsSize = 0;

		SendMessage(0, &header, &header);
	}
}