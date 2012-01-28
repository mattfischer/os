#include "ProcessManager.h"
#include "Sched.h"
#include "InitFs.h"
#include "AsmFuncs.h"
#include "Elf.h"
#include "Util.h"
#include "AddressSpace.h"
#include "Process.h"
#include "Object.h"
#include "Message.h"
#include "Kernel.h"

#include <kernel/include/ProcManagerFmt.h>

struct StartupInfo {
	char name[16];
};

//!< Object id for the process manager itself
int ProcessManager::sObject;

// Shim function to kickstart newly-spawned processes.
static void startUser(void *param)
{
	struct StartupInfo *startupInfo = (struct StartupInfo*)param;

	// Allocate a userspace stack for the task.
	int stackSize = PAGE_SIZE;
	MemArea *stackArea = new MemAreaPages(stackSize);
	char *stackVAddr = (char*)(KERNEL_START - stackArea->size());
	Sched::current()->process()->addressSpace()->map(stackArea, stackVAddr, 0, stackArea->size());

	// Load the executable into the process
	int size;
	void *data = InitFs_Lookup(startupInfo->name, &size);
	Elf::Entry entry = Elf::load(Sched::current()->process()->addressSpace(), data, size);

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

	// Create a task within the process, and copy the startup info into it
	Task *task = new Task(process);

	struct StartupInfo *startupInfo = (struct StartupInfo *)task->stackAllocate(sizeof(struct StartupInfo));
	strcpy(startupInfo->name, name);

	// Start the task--it will load the executable on its own thread, to avoid
	// blocking this task.
	task->start(startUser, startupInfo);
}

// Main task for process manager
void ProcessManager::main(void *param)
{
	// Create and register the process manager object
	sObject = Object_Create(OBJECT_INVALID, NULL);
	Kernel::setObject(KernelObjectProcManager, sObject);

	// Kernel initialization is now complete.  Start the first userspace process.
	startUserProcess("init", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);

	// Now that userspace is up and running, the only remaining role of this task
	// is to service messages that it sends to us.
	while(1) {
		// Wait on the process manager object for incoming messages
		union ProcManagerMsg message;
		struct BufferSegment recvSegs[] = { &message, sizeof(message) };
		struct MessageHeader recvHdr = { recvSegs, 1, 0, 0 };
		int msg = Object_Receivex(object(), &recvHdr);

		if(msg == 0) {
			// Received an event--ignore it.
			continue;
		}

		switch(message.msg.type) {
			case ProcManagerMapPhys:
			{
				// Map physical memory request.  Create a physical memory area and map it into
				// the sending process.
				MemArea *area = new MemAreaPhys(message.msg.u.mapPhys.size, message.msg.u.mapPhys.paddr);
				Task *sender = Sched::current()->process()->message(msg)->sender();
				sender->process()->addressSpace()->map(area, (void*)message.msg.u.mapPhys.vaddr, 0, area->size());

				Message_Reply(msg, 0, NULL, 0);
				break;
			}

			case ProcManagerSbrk:
			{
				// Expand heap request.
				Process *process = Sched::current()->process()->message(msg)->sender()->process();
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
			}
		}
	}
}

/*!
 * \brief Start the process manager--never returns
 */
void ProcessManager::start()
{
	// Execution up until now has not been in the context of a
	// task, as the scheduler and task system had not been initialized
	// yet.  The task has thus far been running on InitStack, a statically
	// allocated stack that is not associated with any task.  We
	// are now ready to construct an actual task, and begin executing
	// on its stack instead.  Construct this task now, and get it ready
	// to begin execution.
	Task *task = new Task(Kernel::process());
	task->start(main, NULL);

	// Start the task.  This call never returns--execution will
	// continue from the task's startup function, main().
	Sched::runFirst();

	// Poof!
}