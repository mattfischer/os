#include "Server.hpp"

#include "Sched.hpp"
#include "InitFs.hpp"
#include "AddressSpace.hpp"
#include "Process.hpp"
#include "Object.hpp"
#include "Message.hpp"
#include "Kernel.hpp"
#include "Interrupt.hpp"
#include "MemArea.hpp"
#include "Log.hpp"
#include "Channel.hpp"
#include "UserProcess.hpp"

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

//!< Object id for the process manager itself
static int channel;

static int kernelObject;

int startUserProcess(const char *cmdline, int stdinObject, int stdoutObject, int stderrObject, int nameserverObject)
{
	// Create a new process
	Process *process = new Process();

	// Construct the process object, to which userspace will send messages
	// in order to access process services
	int processObject = Object_Create(channel, (unsigned)process);

	UserProcess::start(process, cmdline, stdinObject, stdoutObject, stderrObject, kernelObject, processObject, nameserverObject);

	return processObject;
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
					int obj = startUserProcess(
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
			Process *process = (Process*)targetData;

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