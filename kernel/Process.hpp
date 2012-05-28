#ifndef PROCESS_H
#define PROCESS_H

#include "Slab.hpp"
#include "Interrupt.hpp"
#include "Task.hpp"

class Object;
class Message;
class MemAreaPages;
class AddressSpace;

/*!
 * \brief Represents a single process, including an address space and a list of tasks
 */
class Process {
public:
	Process(AddressSpace *addressSpace = NULL);
	~Process();

	/*!
	 * \brief Address space used by this process
	 * \return Address space
	 */
	AddressSpace *addressSpace() { return mAddressSpace; }

	/*!
	 * \brief Address of the top of the heap
	 * \return Heap top
	 */
	char *heapTop() { return mHeapTop; }
	void growHeap(int increment);

	Object *object(int obj);
	int refObject(Object *object);
	int refObjectTo(int obj, Object *object);
	void unrefObject(int obj);

	int dupObjectRef(Process *sourceProcess, int sourceObj);
	int dupObjectRefTo(int obj, Process *sourceProcess, int sourceObj);

	Message *message(int msg);
	int refMessage(Message *message);
	void unrefMessage(int msg);

	Interrupt::Subscription *subscription(int sub) { return mSubscriptions[sub]; }
	int refSubscription(Interrupt::Subscription *subscription);
	void unrefSubscription(int sub);

	Object *processObject() { return mProcessObject; }
	void setProcessObject(Object *processObject) { mProcessObject = processObject; }

	Task *newTask(Page *stack = NULL);
	void killTask(Task *task);

	//! Allocator
	void *operator new(size_t size) { return sSlab.allocate(); }
	void operator delete(void *p) { sSlab.free((Process*)p); }

private:
	AddressSpace *mAddressSpace; //!< Address space of process
	MemAreaPages *mHeap; //!< Memory area for heap
	char *mHeapTop; //!< Top of heap
	char *mHeapAreaTop; //!< Top of heap area
	Object *mObjects[16]; //!< Object list
	Message *mMessages[16]; //!< Outstanding messages
	Interrupt::Subscription *mSubscriptions[16]; //!< Interrupt subscriptions
	Object *mProcessObject;
	ListAux<Task, &Task::mProcessListEntry> mTasks;

	static Slab<Process> sSlab;
};

#endif
