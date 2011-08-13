#include "Sched.h"
#include "Page.h"
#include "AddressSpace.h"
#include "Defs.h"
#include "ProcManager.h"
#include "Object.h"
#include "Name.h"

#include <kernel/include/Syscalls.h>

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
			object = Current->process->objects[arg0];
			ret = Object_SendMessage(object, (struct MessageHeader*)arg1, (struct MessageHeader*)arg2);
			return ret;

		case SyscallReceiveMessage:
			object = Current->process->objects[arg0];
			message = Object_ReceiveMessage(object, (struct MessageHeader*)arg1);
			ret = Process_RefMessage(Current->process, message);
			return ret;

		case SyscallReplyMessage:
			message = Current->process->messages[arg0];
			ret = Object_ReplyMessage(message, arg1, (struct MessageHeader*)arg2);
			Process_UnrefMessage(Current->process, arg0);
			return ret;

		case SyscallObjectCreate:
			object = Object_Create();
			ret = Process_RefObject(Current->process, object);
			return ret;

		case SyscallObjectUnref:
			Process_UnrefObject(Current->process, arg0);
			ret = 0;
			return ret;
	}
}