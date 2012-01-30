#include "Kernel.hpp"
#include "ProcessManager.hpp"

#include "include/Syscalls.h"

/*!
 * \file
 * \brief C++ entry points, for both system startup and syscalls
 */

// Entry points from assembly code.  C linkage to avoid name mangling.
extern "C" {
	void Entry();
	int SysEntry(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);
}

// These symbols are created in the linker script, to point to the constructor list.
extern void *__CtorsStart[];
extern void *__CtorsEnd[];
typedef void (*CtorFunc)();

// Run global C++ constructors
static void runCtors()
{
	// Iterate through the constructor area and call each in turn
	CtorFunc *ctors = (CtorFunc*)__CtorsStart;
	int len = (CtorFunc*)__CtorsEnd - (CtorFunc*)__CtorsStart;

	for(int i=0; i<len; i++) {
		ctors[i]();
	}
}

/*!
 * \brief Entry point for C++ code, called by assembly code after basic init is done
 */
void Entry()
{
	// Run C++ constructors to get global objects initialized
	runCtors();

	// Initialize the kernel data structures
	Kernel::init();

	// Set up the process manager, and start userspace.  This call never returns.
	ProcessManager::start();

	// Poof!
}

/*!
 * \brief Syscall entry point for C++ code, called from assembly shim
 */
int SysEntry(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
	return Kernel::syscall(code, arg0, arg1, arg2, arg3);
}