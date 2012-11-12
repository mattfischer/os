#ifndef KERNEL_H
#define KERNEL_H

#include "include/Syscalls.h"

class Process;

/*!
 * \brief Contains basic kernel state information
 */
class Kernel {
public:
	static void init();

	/*!
	 * \brief Returns the process corresponding to the kernel itself
	 * \return Kernel process
	 */
	static Process *process() { return sProcess; }
	static int syscall(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);

private:
	static Process *sProcess;
};

#endif
