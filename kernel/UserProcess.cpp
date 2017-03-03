#include "UserProcess.hpp"

#include "Sched.hpp"
#include "Elf.hpp"
#include "AddressSpace.hpp"
#include "MemArea.hpp"
#include "AsmFuncs.hpp"
#include "Log.hpp"

#include <kernel/include/KernelFmt.h>
#include <kernel/include/Objects.h>

#include <string.h>

struct StartupInfo {
	char cmdline[KERNEL_CMDLINE_LEN];
};

// Shim function to kickstart newly-spawned processes.
static void startUser(void *param)
{
	struct StartupInfo *startupInfo = (struct StartupInfo*)param;
	Process *process = Sched::current()->process();

	// Allocate a userspace stack for the task.
	int stackSize = PAGE_SIZE;
	MemArea *stackArea = new MemAreaPages(stackSize);
	char *stackVAddr = (char*)(KERNEL_START - stackArea->size());
	process->addressSpace()->map(stackArea, stackVAddr, 0, stackArea->size());

	// Copy the command line into the top of the userspace stack
	char *cmdlineVAddr = (char*)(KERNEL_START - KERNEL_CMDLINE_LEN);
	memcpy(cmdlineVAddr, startupInfo->cmdline, KERNEL_CMDLINE_LEN);

	// Load the executable into the process
	Elf::Entry entry = Elf::load(process->addressSpace(), startupInfo->cmdline);

	// Everything is now set up in the new process.  The time has come at last
	// to enter userspace.  This call never returns--any transfer back to kernel
	// mode from this task will come in the form of syscalls.
	EnterUser(entry, cmdlineVAddr, cmdlineVAddr);

	// Poof!
}

// Start the named process in userspace
void UserProcess::start(Process *process, const char *cmdline, int stdinObject, int stdoutObject, int stderrObject, int kernelObject, int processObject, int nameserverObject)
{
	Log::printf("processManager: start process %s\n", cmdline);

	// Duplicate handles into the newly-created process
	process->dupObjectRefTo(STDIN_NO, Sched::current()->process(), stdinObject);
	process->dupObjectRefTo(STDOUT_NO, Sched::current()->process(), stdoutObject);
	process->dupObjectRefTo(STDERR_NO, Sched::current()->process(), stdoutObject);
	process->dupObjectRefTo(KERNEL_NO, Sched::current()->process(), kernelObject);
	process->dupObjectRefTo(PROCESS_NO, Sched::current()->process(), processObject);
	process->dupObjectRefTo(NAMESERVER_NO, Sched::current()->process(), nameserverObject);

	// Create a task within the process, and copy the startup info into it
	Task *task = process->newTask();

	struct StartupInfo *startupInfo = (struct StartupInfo *)task->stackAllocate(sizeof(struct StartupInfo));
	memcpy(startupInfo->cmdline, cmdline, KERNEL_CMDLINE_LEN);

	// Start the task--it will load the executable on its own thread, to avoid
	// blocking this task.
	task->start(startUser, startupInfo);
}
