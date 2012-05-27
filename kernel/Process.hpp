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

	/*!
	 * \brief Address space used by this process
	 * \return Address space
	 */
	AddressSpace *addressSpace() { return mAddressSpace; }

	/*!
	 * \brief Memory area containing process heap
	 * \return Heap
	 */
	MemAreaPages *heap() { return mHeap; }
	/*!
	 * \brief Set heap area
	 * \param heap Heap area
	 */
	void setHeap(MemAreaPages *heap) { mHeap = heap; }

	/*!
	 * \brief Address of the top of the heap
	 * \return Heap top
	 */
	char *heapTop() { return mHeapTop; }
	/*!
	 * \brief Set top of heap
	 * \param heapTop Top of heap
	 */
	void setHeapTop(char *heapTop) { mHeapTop = heapTop; }

	/*!
	 * \brief Top of heap memory area
	 * \return Heap area top
	 */
	char *heapAreaTop() { return mHeapAreaTop; }
	/*!
	 * \brief Set top of heap area
	 * \param heapAreaTop Top of heap area
	 */
	void setHeapAreaTop(char *heapAreaTop) { mHeapAreaTop = heapAreaTop; }

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

	void addTask(Task *task) { mTasks.addHead(task); }
	void removeTask(Task *task) { mTasks.remove(task); }

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
