#ifndef KERNEL_H
#define KERNEL_H

#include "Process.h"

class Kernel {
public:
	static void init();
	static Process *process() { return sProcess; }

	static void initLow();
private:
	static Process *sProcess;
};

#endif