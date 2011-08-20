#include "Sched.h"
#include "Page.h"
#include "AddressSpace.h"
#include "Defs.h"
#include "ProcessManager.h"
#include "Object.h"
#include "Name.h"
#include "Message.h"
#include "Kernel.h"

#include "include/Syscalls.h"

char InitStack[256];

extern "C" {
	void EntryLow();
	void Entry();
	int SysEntry(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);
}

SECTION_LOW void EntryLow()
{
	Page::initLow();
	Kernel::initLow();
}

extern void *__CtorsStart[];
extern void *__CtorsEnd[];
typedef void (*CtorFunc)();

static void initCtors()
{
	int i;
	int len;
	CtorFunc *ctors;

	ctors = (CtorFunc*)__CtorsStart;
	len = (CtorFunc*)__CtorsEnd - (CtorFunc*)__CtorsStart;

	for(i=0; i<len; i++) {
		ctors[i]();
	}
}

void Entry()
{
	initCtors();
	Kernel::init();
	ProcessManager::start();
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
			return Sched::current()->process()->dupObjectRef(Kernel::process(), ProcessManager::object());
	}
}