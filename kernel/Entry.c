#include "Sched.h"
#include "Page.h"
#include "AddressSpace.h"
#include "Defs.h"
#include "ProcManager.h"
#include "Object.h"
#include "Name.h"
#include "Message.h"

#include "include/Syscalls.h"

char InitStack[256];

SECTION_LOW void EntryLow()
{
	Page_InitLow();
	PageTable_InitLow();
	AddressSpace_InitLow();
}

void Entry()
{
	AddressSpace_Init();
	Sched_Init();
	Object_Init();
	Name_Init();

	ProcManager_Start();
}

int SysEntry(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2)
{
	struct Object *object;
	struct Message *message;
	int ret;

	switch(code) {
		case SyscallYield:
			Sched_RunNext();
			return 0;

		case SyscallSendMessage:
			return SendMessagex(arg0, (struct MessageHeader*)arg1, (struct MessageHeader*)arg2);

		case SyscallReceiveMessage:
			return ReceiveMessagex(arg0, (struct MessageHeader*)arg1);

		case SyscallReplyMessage:
			return ReplyMessagex(arg0, (int)arg1, (struct MessageHeader*)arg2);

		case SyscallCreateObject:
			return CreateObject();

		case SyscallReleaseObject:
			ReleaseObject(arg0);
			return 0;
	}
}