#include "Process.h"
#include "Slab.h"
#include "Util.h"
#include "Object.h"

SlabAllocator<Process> Process::sSlab;

Process::Process(AddressSpace *addressSpace)
{
	if(addressSpace == NULL) {
		addressSpace = new AddressSpace();
	}

	mAddressSpace = addressSpace;
	mHeap = NULL;
	mHeapTop = NULL;
	memset(mObjects, 0, sizeof(struct Object*) * 16);
	memset(mMessages, 0, sizeof(struct Message*) * 16);
}

struct Object *Process::object(int obj)
{
	if(obj == INVALID_OBJECT) {
		return NULL;
	} else {
		return mObjects[obj];
	}
}

int Process::refObject(struct Object *object)
{
	int i;

	if(object == NULL) {
		return INVALID_OBJECT;
	}

	for(i=0; i<16; i++) {
		if(mObjects[i] == NULL) {
			mObjects[i] = object;
			return i;
		}
	}

	return INVALID_OBJECT;
}

int Process::refObjectTo(int obj, struct Object *object)
{
	if(mObjects[obj] != NULL || object == NULL) {
		return INVALID_OBJECT;
	}

	mObjects[obj] = object;
	return obj;
}

void Process::unrefObject(int obj)
{
	if(obj != INVALID_OBJECT) {
		mObjects[obj] = NULL;
	}
}

int Process::dupObjectRef(Process *sourceProcess, int sourceObj)
{
	if(sourceObj == INVALID_OBJECT) {
		return INVALID_OBJECT;
	}

	return refObject(sourceProcess->object(sourceObj));
}

int Process::dupObjectRefTo(int obj, Process *sourceProcess, int sourceObj)
{
	if(sourceObj == INVALID_OBJECT) {
		return INVALID_OBJECT;
	}

	return refObjectTo(obj, sourceProcess->object(sourceObj));
}

struct Message *Process::message(int msg)
{
	return mMessages[msg];
}

int Process::refMessage(struct Message *message)
{
	int i;

	for(i=0; i<16; i++) {
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
