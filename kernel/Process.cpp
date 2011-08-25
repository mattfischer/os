#include "Process.h"
#include "Slab.h"
#include "Util.h"
#include "Object.h"

Slab<Process> Process::sSlab;

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

Object *Process::object(int obj)
{
	if(obj == OBJECT_INVALID) {
		return NULL;
	} else {
		return mObjects[obj];
	}
}

int Process::refObject(Object *object)
{
	if(object == NULL) {
		return OBJECT_INVALID;
	}

	for(int i=0; i<16; i++) {
		if(mObjects[i] == NULL) {
			mObjects[i] = object;
			return i;
		}
	}

	return OBJECT_INVALID;
}

int Process::refObjectTo(int obj, Object *object)
{
	if(mObjects[obj] != NULL || object == NULL) {
		return OBJECT_INVALID;
	}

	mObjects[obj] = object;
	return obj;
}

void Process::unrefObject(int obj)
{
	if(obj != OBJECT_INVALID) {
		mObjects[obj] = NULL;
	}
}

int Process::dupObjectRef(Process *sourceProcess, int sourceObj)
{
	if(sourceObj == OBJECT_INVALID) {
		return OBJECT_INVALID;
	}

	return refObject(sourceProcess->object(sourceObj));
}

int Process::dupObjectRefTo(int obj, Process *sourceProcess, int sourceObj)
{
	if(sourceObj == OBJECT_INVALID) {
		return OBJECT_INVALID;
	}

	return refObjectTo(obj, sourceProcess->object(sourceObj));
}

struct Message *Process::message(int msg)
{
	return mMessages[msg];
}

int Process::refMessage(Message *message)
{
	for(int i=0; i<16; i++) {
		if(mMessages[i] == NULL) {
			mMessages[i] = message;
			return i;
		}
	}

	return -1;
}

void Process::unrefMessage(int msg)
{
	mMessages[msg] = NULL;
}
