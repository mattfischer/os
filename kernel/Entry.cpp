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

extern "C" {
	void EntryLow();
	void Entry();
	int SysEntry(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);
}

SECTION_LOW void EntryLow()
{
	Page_InitLow();
	PageTable::initLow();
}

extern void *__ConstructorsStart[];
extern void *__ConstructorsEnd[];
typedef void (*ConstructorFunc)();

static void Constructors_Init()
{
	int i;
	int len;
	ConstructorFunc *constructors;

	constructors = (ConstructorFunc*)__ConstructorsStart;
	len = (ConstructorFunc*)__ConstructorsEnd - (ConstructorFunc*)__ConstructorsStart;

	for(i=0; i<len; i++) {
		constructors[i]();
	}
}

void Entry()
{
	Constructors_Init();
	PageTable::init();
	AddressSpace::init();
	Process::init();
	Name_Init();

	ProcManager_Start();
}

int SysEntry(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
	struct Object *object;
	struct Message *message;
	int ret;

	switch(code) {
		case SyscallYield:
			Sched::runNext();
			return 0;

		case SyscallSendMessage:
			return SendMessagex(arg0, (struct MessageHeader*)arg1, (struct MessageHeader*)arg2);

		case SyscallReceiveMessage:
			return ReceiveMessagex(arg0, (struct MessageHeader*)arg1);

		case SyscallReadMessage:
			return ReadMessage(arg0, (void*)arg1, (int)arg2, (int)arg3);

		case SyscallReplyMessage:
			return ReplyMessagex(arg0, (int)arg1, (struct MessageHeader*)arg2);

		case SyscallCreateObject:
			return CreateObject();

		case SyscallReleaseObject:
			ReleaseObject(arg0);
			return 0;

		case SyscallGetProcessManager:
			return Sched::current()->process()->dupObjectRef(Process::Kernel, ProcessManager);
	}
}