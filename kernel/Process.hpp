#ifndef PROCESS_H
#define PROCESS_H

#include "Slab.hpp"
#include "Interrupt.hpp"
#include "Task.hpp"
#include "Object.hpp"
#include "Channel.hpp"

class Message;
class MemAreaPages;
class AddressSpace;

/*!
 * \brief Represents a single process, including an address space and a list of tasks
 */
class Process {
public:
	enum State {
		StateRunning, //!< Running
		StateDead     //!< Dead
	};

	Process(AddressSpace *addressSpace = 0);
	~Process();

	State state() { return mState; }

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

	Channel *channel(int chan);
	int refChannel(Channel *channel);
	void unrefChannel(int chan);

	void addWaiter(int msg);
	int waiter(int waiter) { return mWaiters[waiter]; }

	Task *newTask(Page *stack = 0);

	void kill();

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
	int mWaiters[16]; //!< Waiting processes
	Channel *mChannels[16];
	ListAux<Task, &Task::mProcessListEntry> mTasks;
	State mState;

	static Slab<Process> sSlab;
};

#endif
