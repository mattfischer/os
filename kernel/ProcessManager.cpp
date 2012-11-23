#include "ProcessManager.hpp"

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

#include <kernel/include/ProcManagerFmt.h>

#include <string.h>

struct ProcessInfo {
	Process *process;
	int obj;
};

Slab<ProcessInfo> processInfoSlab;

struct StartupInfo {
	char cmdline[PROC_MANAGER_CMDLINE_LEN];
};

class EventSubscription : public Interrupt::Subscription
{
public:
	EventSubscription(int irq, Object *object, unsigned type, unsigned value)
	: Interrupt::Subscription(irq)
	{
		mObject = object;
		mType = type;
		mValue = value;
	}

	virtual void dispatch()
	{
		mObject->post(mType, mValue);
	}

	//! Allocator
	void *operator new(size_t size) { return sSlab.allocate(); }
	void operator delete(void *p) { ((EventSubscription*)p)->free(); }
	virtual void free() { sSlab.free(this); }

private:
	Object *mObject;
	unsigned mType;
	unsigned mValue;

	static Slab<EventSubscription> sSlab;
};

Slab<EventSubscription> EventSubscription::sSlab;

//!< Object id for the process manager itself
static int manager;

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
	char *cmdlineVAddr = (char*)(KERNEL_START - PROC_MANAGER_CMDLINE_LEN);
	memcpy(cmdlineVAddr, startupInfo->cmdline, PROC_MANAGER_CMDLINE_LEN);

	// Load the executable into the process
	Elf::Entry entry = Elf::load(process->addressSpace(), startupInfo->cmdline);

	// Everything is now set up in the new process.  The time has come at last
	// to enter userspace.  This call never returns--any transfer back to kernel
	// mode from this task will come in the form of syscalls.
	EnterUser(entry, cmdlineVAddr, cmdlineVAddr);

	// Poof!
}

// Start the named process in userspace
static ProcessInfo *startUserProcess(const char *cmdline, int stdinObject, int stdoutObject, int stderrObject)
{
	Log::printf("processManager: start process %s\n", cmdline);

	// Create a new process
	Process *process = new Process();

	ProcessInfo *processInfo = (ProcessInfo*)processInfoSlab.allocate();
	processInfo->process = process;

	// Construct the process object, to which userspace will send messages
	// in order to access process services
	int obj = Object_Create(manager, processInfo);
	processInfo->obj = obj;

	// Duplicate handles into the newly-created process
	process->dupObjectRefTo(0, Sched::current()->process(), stdinObject, Object::Handle::TypeClient);
	process->dupObjectRefTo(1, Sched::current()->process(), stdoutObject, Object::Handle::TypeClient);
	process->dupObjectRefTo(2, Sched::current()->process(), stdoutObject, Object::Handle::TypeClient);
	process->dupObjectRefTo(3, Sched::current()->process(), obj, Object::Handle::TypeClient);

	// Create a task within the process, and copy the startup info into it
	Task *task = process->newTask();

	struct StartupInfo *startupInfo = (struct StartupInfo *)task->stackAllocate(sizeof(struct StartupInfo));
	memcpy(startupInfo->cmdline, cmdline, PROC_MANAGER_CMDLINE_LEN);

	// Start the task--it will load the executable on its own thread, to avoid
	// blocking this task.
	task->start(startUser, startupInfo);
	return processInfo;
}

// Main task for process manager
void ProcessManager::start()
{
	// Create and register the process manager object
	manager = Object_Create(OBJECT_INVALID, 0);

	// Start the InitFs file server, to serve up files from the
	// built-in filesystem that is compiled into the kernel
	InitFs::start();

	// Start the log buffer
	Log::start();

	// Kernel initialization is now complete.  Start the first userspace process.
	startUserProcess("/boot/init", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);

	// Now that userspace is up and running, the only remaining role of this task
	// is to service messages that it sends to us.
	while(1) {
		// Wait on the process manager object for incoming messages
		union ProcManagerMsg message;
		int msg = Object_Receive(manager, &message, sizeof(message));

		if(msg == 0) {
			switch(message.event.type) {
				case SysEventObjectClosed:
				{
					ProcessInfo *processInfo = (ProcessInfo*)message.event.targetData;
					Object_Release(processInfo->obj);
					delete processInfo->process;
					break;
				}
			}
			continue;
		}

		// Grab the process to which this message was directed
		struct MessageInfo info;
		Message_Info(msg, &info);
		ProcessInfo *processInfo = (ProcessInfo*)info.targetData;
		Process *process = processInfo->process;

		switch(message.msg.type) {
			case ProcManagerMapPhys:
			{
				// Map physical memory request.  Create a physical memory area and map it into
				// the sending process.
				MemArea *area = new MemAreaPhys(message.msg.u.mapPhys.size, message.msg.u.mapPhys.paddr);
				process->addressSpace()->map(area, (void*)message.msg.u.mapPhys.vaddr, 0, area->size());

				Message_Reply(msg, 0, 0, 0);
				break;
			}

			case ProcManagerSbrk:
			{
				// Expand heap request.
				int ret = (int)process->heapTop();
				process->growHeap(message.msg.u.sbrk.increment);

				Message_Reply(msg, ret, 0, 0);
				break;
			}

			case ProcManagerSpawnProcess:
			{
				// Spawn a new process.
				ProcessInfo *processInfo = startUserProcess(message.msg.u.spawn.cmdline, message.msg.u.spawn.stdinObject, message.msg.u.spawn.stdoutObject, message.msg.u.spawn.stderrObject);
				Object_Release(message.msg.u.spawn.stdinObject);
				Object_Release(message.msg.u.spawn.stdoutObject);
				Object_Release(message.msg.u.spawn.stderrObject);

				int obj = processInfo->obj;
				Message_Replyh(msg, 0, &obj, sizeof(obj), 0, 1);
				break;
			}

			case ProcManagerSubInt:
			{
				EventSubscription *subscription = new EventSubscription(message.msg.u.subInt.irq, Sched::current()->process()->object(message.msg.u.subInt.object), message.msg.u.subInt.type, message.msg.u.subInt.value);
				Interrupt::subscribe(subscription);
				Object_Release(message.msg.u.subInt.object);

				int sub = process->refSubscription(subscription);
				Message_Reply(msg, 0, &sub, sizeof(sub));
				break;
			}

			case ProcManagerUnsubInt:
			{
				Interrupt::Subscription *subscription = process->subscription(message.msg.u.unsubInt.sub);
				Interrupt::unsubscribe(subscription);

				process->unrefSubscription(message.msg.u.unsubInt.sub);
				Message_Reply(msg, 0, 0, 0);
				break;
			}

			case ProcManagerAckInt:
			{
				Interrupt::Subscription *subscription = process->subscription(message.msg.u.ackInt.sub);
				Interrupt::acknowledge(subscription);
				Message_Reply(msg, 0, 0, 0);
				break;
			}

			case ProcManagerKill:
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

			case ProcManagerWait:
			{
				process->addWaiter(msg);
				break;
			}
		}
	}
}