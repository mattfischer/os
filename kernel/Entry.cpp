#include "Kernel.h"
#include "ProcessManager.h"

#include "include/Syscalls.h"

extern "C" {
	void Entry();
	int SysEntry(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);
}

extern void *__CtorsStart[];
extern void *__CtorsEnd[];
typedef void (*CtorFunc)();

static void initCtors()
{
	CtorFunc *ctors = (CtorFunc*)__CtorsStart;
	int len = (CtorFunc*)__CtorsEnd - (CtorFunc*)__CtorsStart;

	for(int i=0; i<len; i++) {
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
	return Kernel::syscall(code, arg0, arg1, arg2, arg3);
}