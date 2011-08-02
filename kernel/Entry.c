#include "Sched.h"
#include "Page.h"
#include "AddressSpace.h"
#include "Defs.h"
#include "ProcManager.h"
#include "Object.h"

#include <kernel/include/Syscalls.h>

char InitStack[256];

SECTION_LOW void EntryLow()
{
	Page_InitLow();
	AddressSpace_InitLow();
}

void Entry()
{
	AddressSpace_Init();
	Sched_Init();
	Object_Init();

	ProcManager_Start();
}

int SysEntry(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3, unsigned int arg4)
{
	struct Object *object;
	struct Message *message;
	int ret;

	switch(code) {
		case SyscallYield:
			Sched_RunNext();
			return 0;

		case SyscallSendMessage:
			object = Current->process->objects[arg0];
			ret = Object_SendMessage(object, (void*)arg1, arg2, (void*)arg3, arg4);
			return ret;

		case SyscallReceiveMessage:
			object = Current->process->objects[arg0];
			message = Object_ReceiveMessage(object, (void*)arg1, arg2);
			ret = Process_RefMessage(Current->process, message);
			return ret;

		case SyscallReplyMessage:
			message = Current->process->messages[arg0];
			ret = Object_ReplyMessage(message, (void*)arg1, arg2);
			Process_UnrefMessage(Current->process, arg0);
			return ret;
	}
}