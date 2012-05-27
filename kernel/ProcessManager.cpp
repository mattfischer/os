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

#include <kernel/include/ProcManagerFmt.h>

#include <string.h>

struct StartupInfo {
	char name[16];
};

class EventSubscription : public Interrupt::Subscription
{
public:
	EventSubscription(Object *object, unsigned type, unsigned value)
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
	void operator delete(void *p) { sSlab.free((EventSubscription*)p); }

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

	// Load the executable into the process
	Elf::Entry entry = Elf::load(process->addressSpace(), startupInfo->name);

	// Everything is now set up in the new process.  The time has come at last
	// to enter userspace.  This call never returns--any transfer back to kernel
	// mode from this task will come in the form of syscalls.
	EnterUser(entry, stackVAddr + stackSize);

	// Poof!
}

// Start the named process in userspace
static void startUserProcess(const char *name, int stdinObject, int stdoutObject, int stderrObject)
{
	// Create a new process, and duplicate the handles into it
	Process *process = new Process();
	process->dupObjectRefTo(0, Sched::current()->process(), stdinObject);
	process->dupObjectRefTo(1, Sched::current()->process(), stdoutObject);
	process->dupObjectRefTo(2, Sched::current()->process(), stdoutObject);

	// Construct the process object, to which userspace will send messages
	// in order to access process services
	Object *processObject = new Object(Kernel::process()->object(manager), process);
	process->setProcessObject(processObject);

	// Create a task within the process, and copy the startup info into it
	Task *task = process->newTask();

	struct StartupInfo *startupInfo = (struct StartupInfo *)task->stackAllocate(sizeof(struct StartupInfo));
	strcpy(startupInfo->name, name);

	// Start the task--it will load the executable on its own thread, to avoid
	// blocking this task.
	task->start(startUser, startupInfo);
}

// Main task for process manager
void ProcessManager::start()
{
	// Create and register the process manager object
	manager = Object_Create(OBJECT_INVALID, NULL);

	// Start the InitFs file server, to serve up files from the
	// built-in filesystem that is compiled into the kernel
	InitFs::start();

	// Kernel initialization is now complete.  Start the first userspace process.
	startUserProcess("/boot/init", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);

	// Now that userspace is up and running, the only remaining role of this task
	// is to service messages that it sends to us.
	while(1) {
		// Wait on the process manager object for incoming messages
		union ProcManagerMsg message;
		struct BufferSegment recvSegs[] = { &message, sizeof(message) };
		struct MessageHeader recvHdr = { recvSegs, 1, 0, 0 };
		int msg = Object_Receivex(manager, &recvHdr);

		if(msg == 0) {
			// Received an event--ignore it.
			continue;
		}

		// Grab the process to which this message was directed
		struct MessageInfo info;
		Process *process;
		Message_Info(msg, &info);
		process = (Process*)info.targetData;

		switch(message.msg.type) {
			case ProcManagerMapPhys:
			{
				// Map physical memory request.  Create a physical memory area and map it into
				// the sending process.
				MemArea *area = new MemAreaPhys(message.msg.u.mapPhys.size, message.msg.u.mapPhys.paddr);
				process->addressSpace()->map(area, (void*)message.msg.u.mapPhys.vaddr, 0, area->size());

				Message_Reply(msg, 0, NULL, 0);
				break;
			}

			case ProcManagerSbrk:
			{
				// Expand heap request.
				int increment = message.msg.u.sbrk.increment;
				int ret;

				if(process->heapTop() == NULL) {
					// No heap allocated yet.  Create a new memory segment and map it
					// into the process.
					int size = PAGE_SIZE_ROUND_UP(increment);

					process->setHeap(new MemAreaPages(size));
					process->setHeapTop((char*)(0x10000000 + increment));
					process->setHeapAreaTop((char*)(0x10000000 + size));
					process->addressSpace()->map(process->heap(), (void*)0x10000000, 0, size);
					ret = 0x10000000;
				} else {
					if(process->heapTop() + increment < process->heapAreaTop()) {
						// Request can be serviced without expanding the heap area.  Just
						// record the new heap top and return.
						ret = (int)process->heapTop();
						process->setHeapTop(process->heapTop() + increment);
					} else {
						// We must expand the heap area in order to service the request.
						int size = PAGE_SIZE_ROUND_UP(increment);
						int extraPages = size >> PAGE_SHIFT;

						// Allocate new pages, link them into the heap area, and map them into the process
						for(int i=0; i<extraPages; i++) {
							Page *page = Page::alloc();
							process->heap()->pages().addTail(page);
							process->addressSpace()->pageTable()->mapPage(process->heapAreaTop(), page->paddr(), PageTable::PermissionRW);
							process->setHeapAreaTop(process->heapAreaTop() + PAGE_SIZE);
						}

						// Return old heap top, and increment.
						ret = (int)process->heapTop();
						process->setHeapTop(process->heapTop() + increment);
					}
				}

				Message_Reply(msg, ret, NULL, 0);
				break;
			}

			case ProcManagerSpawnProcess:
			{
				// Spawn a new process.
				startUserProcess(message.msg.u.spawn.name, message.msg.u.spawn.stdinObject, message.msg.u.spawn.stdoutObject, message.msg.u.spawn.stderrObject);
				Object_Release(message.msg.u.spawn.stdinObject);
				Object_Release(message.msg.u.spawn.stdoutObject);
				Object_Release(message.msg.u.spawn.stderrObject);

				Message_Reply(msg, 0, NULL, 0);
				break;
			}

			case ProcManagerSubInt:
			{
				EventSubscription *subscription = new EventSubscription(Sched::current()->process()->object(message.msg.u.subInt.object), message.msg.u.subInt.type, message.msg.u.subInt.value);
				Interrupt::subscribe(message.msg.u.subInt.irq, subscription);
				Object_Release(message.msg.u.subInt.object);

				int sub = process->refSubscription(subscription);
				Message_Reply(msg, 0, &sub, sizeof(sub));
				break;
			}

			case ProcManagerUnsubInt:
			{
				Interrupt::Subscription *subscription = process->subscription(message.msg.u.unsubInt.sub);
				Interrupt::unsubscribe(message.msg.u.unsubInt.irq, subscription);

				process->unrefSubscription(message.msg.u.unsubInt.sub);
				Message_Reply(msg, 0, NULL, 0);
				break;
			}

			case ProcManagerAckInt:
			{
				Interrupt::Subscription *subscription = process->subscription(message.msg.u.ackInt.sub);
				Interrupt::acknowledge(message.msg.u.ackInt.irq, subscription);
				Message_Reply(msg, 0, NULL, 0);
				break;
			}
		}
	}
}