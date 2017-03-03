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

Server::Server()
{
	// Create and register the process manager object
	mChannel = Channel_Create();
	mKernelObject = Object_Create(mChannel, 0);
}

int Server::startUserProcess(const char *cmdline, int stdinObject, int stdoutObject, int stderrObject, int nameserverObject)
{
	// Create a new process
	Process *process = new Process();

	// Construct the process object, to which userspace will send messages
	// in order to access process services
	int processObject = Object_Create(mChannel, (unsigned)process);

	UserProcess::start(process, cmdline, stdinObject, stdoutObject, stderrObject, mKernelObject, processObject, nameserverObject);

	return processObject;
}

// Main task for process manager
void Server::run()
{
	while(1) {
		// Wait on the process manager object for incoming messages
		union Message {
			ProcessMsg process;
			KernelMsg kernel;
		};
		union Message message;
		unsigned targetData;
		int msg = Channel_Receive(mChannel, &message, sizeof(message), &targetData);

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