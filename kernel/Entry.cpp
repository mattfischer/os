#include "Page.h"
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
	return Kernel::syscall(code, arg0, arg1, arg2, arg3);
}