#include "Server.hpp"

#include "Sched.hpp"
#include "InitFs.hpp"
#include "AsmFuncs.hpp"
#include "Elf.hpp"
#include "AddressSpace.hpp"
#include "Process.hpp"
#include "Object.hpp"
#include "Message.hpp"
#include "Kernel.hpp"
#include "Interrupt.hpp"
#include "MemArea.hpp"
#include "PageTable.hpp"
#include "Log.hpp"
#include "Channel.hpp"

#include <kernel/include/ProcessFmt.h>
#include <kernel/include/KernelFmt.h>
#include <kernel/include/Objects.h>

#include <string.h>

#include <algorithm>

struct ProcessInfo {
	Process *process;
	int obj;
};

Slab<ProcessInfo> processInfoSlab;

struct StartupInfo {
	char cmdline[KERNEL_CMDLINE_LEN];
};

//!< Object id for the process manager itself
static int channel;

static int kernelObject;

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
static ProcessInfo *startUserProcess(const char *cmdline, int stdinObject, int stdoutObject, int stderrObject, int nameserverObject)
{
	Log::printf("processManager: start process %s\n", cmdline);

	// Create a new process
	Process *process = new Process();

	ProcessInfo *processInfo = (ProcessInfo*)processInfoSlab.allocate();
	processInfo->process = process;

	// Construct the process object, to which userspace will send messages
	// in order to access process services
	int obj = Object_Create(channel, (unsigned)processInfo);
	processInfo->obj = obj;

	// Duplicate handles into the newly-created process
	process->dupObjectRefTo(STDIN_NO, Sched::current()->process(), stdinObject);
	process->dupObjectRefTo(STDOUT_NO, Sched::current()->process(), stdoutObject);
	process->dupObjectRefTo(STDERR_NO, Sched::current()->process(), stdoutObject);
	process->dupObjectRefTo(KERNEL_NO, Sched::current()->process(), kernelObject);
	process->dupObjectRefTo(PROCESS_NO, Sched::current()->process(), obj);
	process->dupObjectRefTo(NAMESERVER_NO, Sched::current()->process(), nameserverObject);

	// Create a task within the process, and copy the startup info into it
	Task *task = process->newTask();

	struct StartupInfo *startupInfo = (struct StartupInfo *)task->stackAllocate(sizeof(struct StartupInfo));
	memcpy(startupInfo->cmdline, cmdline, KERNEL_CMDLINE_LEN);

	// Start the task--it will load the executable on its own thread, to avoid
	// blocking this task.
	task->start(startUser, startupInfo);
	return processInfo;
}

// Main task for process manager
void Server::start()
{
	// Create and register the process manager object
	channel = Channel_Create();
	kernelObject = Object_Create(channel, 0);

	// Start the InitFs file server, to serve up files from the
	// built-in filesystem that is compiled into the kernel
	InitFs::start();

	// Kernel initialization is now complete.  Start the first userspace process.
	startUserProcess("/boot/name\0/boot/init\0\0", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID, InitFs::nameServer());

	// Now that userspace is up and running, the only remaining role of this task
	// is to service messages that it sends to us.
	while(1) {
		// Wait on the process manager object for incoming messages
		union Message {
			ProcessMsg process;
			KernelMsg kernel;
		};
		union Message message;
		unsigned targetData;
		int msg = Channel_Receive(channel, &message, sizeof(message), &targetData);

		if(targetData == 0) {
			switch(message.kernel.msg.type) {
				case KernelSpawnProcess:
				{
					// Spawn a new process.
					ProcessInfo *processInfo = startUserProcess(
						message.kernel.msg.u.spawn.cmdline,
						message.kernel.msg.u.spawn.stdinObject,
						message.kernel.msg.u.spawn.stdoutObject,
						message.kernel.msg.u.spawn.stderrObject,
						message.kernel.msg.u.spawn.nameserverObject
					);
					Object_Release(message.kernel.msg.u.spawn.stdinObject);
					Object_Release(message.kernel.msg.u.spawn.stdoutObject);
					Object_Release(message.kernel.msg.u.spawn.stderrObject);
					Object_Release(message.kernel.msg.u.spawn.nameserverObject);

					int obj = processInfo->obj;
					Message_Replyh(msg, 0, &obj, sizeof(obj), 0, 1);
					Object_Release(obj);
					break;
				}

				case KernelSubInt:
				{
					bool success = Interrupt::subscribe(
						message.kernel.msg.u.subInt.irq,
						Sched::current()->process()->object(message.kernel.msg.u.subInt.object),
						message.kernel.msg.u.subInt.type,
						message.kernel.msg.u.subInt.value
					);
					Object_Release(message.kernel.msg.u.subInt.object);
					Message_Reply(msg, success ? 1 : 0, 0, 0);
					break;
				}

				case KernelUnmaskInt:
				{
					Interrupt::unmask(message.kernel.msg.u.unmaskInt.irq);
					Message_Reply(msg, 0, 0, 0);
					break;
				}

				case KernelReadLog:
				{
					const char *data;
					int size;

					size = Log::read(message.kernel.msg.u.readLog.offset, &data);
					size = std::min(size, message.kernel.msg.u.readLog.size);
					Message_Reply(msg, size, data, size);
					break;
				}
			}
		} else {
			// Grab the process to which this message was directed
			ProcessInfo *processInfo = (ProcessInfo*)targetData;
			Process *process = processInfo->process;

			if(msg == 0) {
				switch(message.process.event.type) {
					case SysEventObjectClosed:
					{
						delete process;
						break;
					}
				}
				continue;
			}

			switch(message.process.msg.type) {
				case ProcessMapPhys:
				{
					// Map physical memory request.  Create a physical memory area and map it into
					// the sending process.
					MemArea *area = new MemAreaPhys(message.process.msg.u.mapPhys.size, message.process.msg.u.mapPhys.paddr);
					process->addressSpace()->map(area, (void*)message.process.msg.u.mapPhys.vaddr, 0, area->size());

					Message_Reply(msg, 0, 0, 0);
					break;
				}

				case ProcessSbrk:
				{
					// Expand heap request.
					int ret = (int)process->heapTop();
					process->growHeap(message.process.msg.u.sbrk.increment);

					Message_Reply(msg, ret, 0, 0);
					break;
				}

				case ProcessKill:
				{
					for(int i=0; i<16; i++) {
						int m = process->waiter(i);
						if(m != 0) {
							Message_Reply(m, 0, 0, 0);
						}
					}
					process->kill();
					Message_Reply(msg, 0, 0, 0);
					break;
				}

				case ProcessWait:
				{
					process->addWaiter(msg);
					break;
				}
			}
		}
	}
}