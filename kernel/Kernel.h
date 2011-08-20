#ifndef KERNEL_H
#define KERNEL_H

#include "Process.h"
#include "include/Syscalls.h"

class Kernel {
public:
	static void init();
	static Process *process() { return sProcess; }
	static int syscall(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);

	static void initLow();
private:
	static Process *sProcess;
};

#endif