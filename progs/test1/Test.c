#include <kernel/include/Syscalls.h>

int swi(unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3, unsigned int arg4, unsigned int arg5);

int SendMessage(int obj, void *sendBuf, int sendSz, void *replyBuf, int replySz)
{
	return swi(SyscallSendMessage, (unsigned int)obj, (unsigned int)sendBuf, (unsigned int)sendSz, (unsigned int)replyBuf, (unsigned int)replySz);
}

void _start()
{
	int x;
	int r;

	x = 0;
	while(1) {
		SendMessage(0, &x, sizeof(x), &r, sizeof(r));
		x = r;
	}
}