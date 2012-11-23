#include "Process.hpp"

#include "Slab.hpp"
#include "Object.hpp"
#include "AddressSpace.hpp"
#include "Message.hpp"
#include "MemArea.hpp"
#include "PageTable.hpp"

#include <string.h>

//!< Slab allocator for processes
Slab<Process> Process::sSlab;

#define HEAP_START (char*)0x10000000

/*!
 * \brief Constructor
 * \param addressSpace Address space to use for process, or NULL to allocate a new one
 */
Process::Process(AddressSpace *addressSpace)
{
	if(addressSpace == NULL) {
		addressSpace = new AddressSpace();
	}

	mAddressSpace = addressSpace;
	mHeap = NULL;
	mHeapTop = HEAP_START;
	mHeapAreaTop = HEAP_START;
	mState = StateRunning;
	memset(mObjects, 0, sizeof(Object*) * 16);
	memset(mMessages, 0, sizeof(Message*) * 16);
	memset(mWaiters, 0, sizeof(int) * 16);
}

Process::~Process()
{
}

void Process::growHeap(int increment)
{
	if(mHeap == NULL) {
		// No heap allocated yet.  Create a new memory segment and map it
		// into the process.
		int size = PAGE_SIZE_ROUND_UP(increment);

		mHeap = new MemAreaPages(size);
		mAddressSpace->map(mHeap, HEAP_START, 0, size);
		mHeapTop = (char*)HEAP_START + increment;
		mHeapAreaTop = (char*)HEAP_START + size;
	} else {
		if(mHeapTop + increment < mHeapAreaTop) {
			// Request can be serviced without expanding the heap area.  Just
			// record the new heap top and return.
			mHeapTop += increment;
		} else {
			// We must expand the heap area in order to service the request.
			int size = PAGE_SIZE_ROUND_UP(increment);
			int extraPages = size >> PAGE_SHIFT;

			// Allocate new pages, link them into the heap area, and map them into the process
			for(int i=0; i<extraPages; i++) {
				Page *page = Page::alloc();
				mHeap->pages().addTail(page);
				mAddressSpace->pageTable()->mapPage(mHeapAreaTop, page->paddr(), PageTable::PermissionRW);
				mHeapAreaTop += PAGE_SIZE;
			}

			// Return old heap top, and increment.
			mHeapTop += increment;
		}
	}
}

/*!
 * \brief Retrieve object
 * \param obj Object number
 * \return Object, or NULL
 */
Object *Process::object(int obj)
{
	Object::Handle *handle = objectHandle(obj);
	if(handle) {
		return handle->object();
	} else {
		return NULL;
	}
}

/*!
 * \brief Retrieve object handle
 * \param obj Object number
 * \return Object handle, or NULL
 */
Object::Handle *Process::objectHandle(int obj)
{
	if(obj == OBJECT_INVALID) {
		return NULL;
	} else {
		return mObjects[obj];
	}
}

/*!
 * \brief Reference an object into the process object table
 * \param object Object
 * \return Index of new object, or OBJECT_INVALID
 */
int Process::refObject(Object *object, Object::Handle::Type type)
{
	if(object == NULL) {
		return OBJECT_INVALID;
	}

	// Find an empty slot
	for(int i=0; i<16; i++) {
		if(mObjects[i] == NULL) {
			Object::Handle *handle = object->handle(type);
			handle->ref();
			mObjects[i] = handle;
			return i;
		}
	}

	return OBJECT_INVALID;
}

/*!
 * \brief Reference an object into the given slot of a process object table
 * \param obj Object index
 * \param object Object
 * \return Index of new object, or OBJECT_INVALID
 */
int Process::refObjectTo(int obj, Object *object, Object::Handle::Type type)
{
	// Check if slot is empty
	if(mObjects[obj] != NULL || object == NULL) {
		return OBJECT_INVALID;
	}

	Object::Handle *handle = object->handle(type);
	handle->ref();
	mObjects[obj] = handle;

	return obj;
}

/*!
 * \brief Unreference an object
 * \param obj Object number to unref
 */
void Process::unrefObject(int obj)
{
	if(obj == OBJECT_INVALID) {
		return;
	}

	if(mObjects[obj]) {
		mObjects[obj]->unref();
		mObjects[obj] = NULL;
	}
}

/*!
 * \brief Duplicate an object reference from another process
 * \param sourceProcess Process to duplicate from
 * \param sourceObj Object number in source process
 * \return Object index in new process, or OBJECT_INVALID
 */
int Process::dupObjectRef(Process *sourceProcess, int sourceObj, Object::Handle::Type type)
{
	if(sourceObj == OBJECT_INVALID) {
		return OBJECT_INVALID;
	}

	return refObject(sourceProcess->object(sourceObj), type);
}

/*!
 * \brief Duplicate an object reference from another process into a given slot
 * \param obj Object slot to duplicate into
 * \param sourceProcess Process to duplicate from
 * \param sourceObj Object number in source process
 * \return Object index in new process, or OBJECT_INVALID
 */
int Process::dupObjectRefTo(int obj, Process *sourceProcess, int sourceObj, Object::Handle::Type type)
{
	if(sourceObj == OBJECT_INVALID) {
		return OBJECT_INVALID;
	}

	return refObjectTo(obj, sourceProcess->object(sourceObj), type);
}

/*!
 * \brief Retrieve a message by index
 * \param msg Message index
 * \return Message, or NULL
 */
struct Message *Process::message(int msg)
{
	return mMessages[msg - 1];
}

/*!
 * \brief Reference a message
 * \param message Message
 * \return Message index, or 0
 */
int Process::refMessage(Message *message)
{
	if(!message) {
		return 0;
	}

	// Find an empty slot
	for(int i=0; i<16; i++) {
		if(mMessages[i] == NULL) {
			mMessages[i] = message;
			return i + 1;
		}
	}

	return -1;
}

/*!
 * \brief Unreference a message
 * \param msg Message index
 */
void Process::unrefMessage(int msg)
{
	mMessages[msg - 1] = NULL;
}

int Process::refSubscription(Interrupt::Subscription *subscription)
{
	// Find an empty slot
	for(int i=0; i<16; i++) {
		if(mSubscriptions[i] == NULL) {
			mSubscriptions[i] = subscription;
			return i;
		}
	}

	return -1;
}

void Process::unrefSubscription(int sub)
{
	mSubscriptions[sub] = NULL;
}

void Process::addWaiter(int msg)
{
	for(int i=0; i<16; i++) {
		if(mWaiters[i] == 0) {
			mWaiters[i] = msg;
			break;
		}
	}
}

Task *Process::newTask(Page *stack)
{
	Task *task = new Task(this, stack);
	mTasks.addHead(task);
	task->ref();

	return task;
}

void Process::kill()
{
	mState = StateDead;

	Task *next;
	for(Task *task = mTasks.head(); task != NULL; task = next)
	{
		next = mTasks.next(task);
		task->kill();
		mTasks.remove(task);
		task->unref();
	}

	delete mAddressSpace;

	if(mHeap != NULL) {
		delete mHeap;
	}

	for(int i=0; i<16; i++) {
		unrefObject(i);
	}

	for(int i=0; i<16; i++) {
		if(mMessages[i] != NULL) {
			mMessages[i]->cancel();
			unrefMessage(i);
		}
	}

	for(int i=0; i<16; i++) {
		if(mSubscriptions[i] != NULL) {
			Interrupt::unsubscribe(mSubscriptions[i]);
			delete mSubscriptions[i];
		}
	}
}