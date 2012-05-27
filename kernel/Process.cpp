#include "Process.hpp"

#include "Slab.hpp"
#include "Object.hpp"
#include "AddressSpace.hpp"
#include "Message.hpp"

#include <string.h>

//!< Slab allocator for processes
Slab<Process> Process::sSlab;

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
	mHeapTop = NULL;
	memset(mObjects, 0, sizeof(Object*) * 16);
	memset(mMessages, 0, sizeof(Message*) * 16);
}

/*!
 * \brief Retrieve object
 * \param obj Object number
 * \return Object, or NULL
 */
Object *Process::object(int obj)
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
int Process::refObject(Object *object)
{
	if(object == NULL) {
		return OBJECT_INVALID;
	}

	// Find an empty slot
	for(int i=0; i<16; i++) {
		if(mObjects[i] == NULL) {
			mObjects[i] = object;
			object->ref();
			object->post(SysEventObjectRef, 0);
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
int Process::refObjectTo(int obj, Object *object)
{
	// Check if slot is empty
	if(mObjects[obj] != NULL || object == NULL) {
		return OBJECT_INVALID;
	}

	mObjects[obj] = object;
	object->ref();
	object->post(SysEventObjectRef, 0);

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
		mObjects[obj]->post(SysEventObjectUnref, 0);
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
int Process::dupObjectRef(Process *sourceProcess, int sourceObj)
{
	if(sourceObj == OBJECT_INVALID) {
		return OBJECT_INVALID;
	}

	return refObject(sourceProcess->object(sourceObj));
}

/*!
 * \brief Duplicate an object reference from another process into a given slot
 * \param obj Object slot to duplicate into
 * \param sourceProcess Process to duplicate from
 * \param sourceObj Object number in source process
 * \return Object index in new process, or OBJECT_INVALID
 */
int Process::dupObjectRefTo(int obj, Process *sourceProcess, int sourceObj)
{
	if(sourceObj == OBJECT_INVALID) {
		return OBJECT_INVALID;
	}

	return refObjectTo(obj, sourceProcess->object(sourceObj));
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

Task *Process::newTask(Page *stack)
{
	Task *task = new Task(this, stack);
	mTasks.addHead(task);
	task->ref();

	return task;
}

void Process::killTask(Task *task)
{
	task->kill();
	mTasks.remove(task);
	task->unref();
}
