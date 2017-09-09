#include "Kernel.hpp"
#include "Server.hpp"
#include "Interrupt.hpp"
#include "Page.hpp"
#include "Sched.hpp"
#include "Task.hpp"
#include "Process.hpp"
#include "Object.hpp"
#include "InitFs.hpp"

#include "include/KernelFmt.h"
#include "include/ProcessFmt.h"
#include "include/MessageFmt.h"
#include "include/Syscalls.h"
#include "include/Objects.h"

/*!
 * \file
 * \brief C++ entry points, for both system startup and syscalls
 */

// Entry points from assembly code.  C linkage to avoid name mangling.
extern "C" {
	void Entry();
	int SysEntry(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);
	void IRQEntry();
	void AbortEntry();
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

	// Initialize the page list and kernel data structures
	Page::init();
	Kernel::init();
	Interrupt::init();

	// Start the InitFs file server, to serve up files from the
	// built-in filesystem that is compiled into the kernel
	InitFs initfs;
	initfs.start();

	// Kernel initialization is now complete.  Set up the userspace server, and the first userspace process
	Server server;
	int obj = server.startUserProcess("/boot/name\0/boot/init\0\0", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID, initfs.object());
	Object_Release(obj);

	// Start userspace.  This call never returns.
	server.run();
}

/*!
 * \brief Syscall entry point for C++ code, called from assembly shim
 */
int SysEntry(enum Syscall code, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
	return Kernel::syscall(code, arg0, arg1, arg2, arg3);
}

/*!
 * \brief IRQ entry point for C++ code, called from assembly shim
 */
void IRQEntry()
{
	Interrupt::dispatch();
}

/*!
 * \brief Abort entry point for C++ code, called from assembly shim
 */
void AbortEntry()
{
	// Just kill the process
	ProcessMsg message;
	message.type = ProcessKill;
	Object_Send(PROCESS_NO, &message, sizeof(message), 0, 0);

	// Poof!
}