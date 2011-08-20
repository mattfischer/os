#include "Process.h"
#include "Slab.h"
#include "Util.h"

static struct SlabAllocator<struct Process> processSlab;

Process *Process::Kernel;

Process::Process(struct AddressSpace *addressSpace)
{
	mAddressSpace = addressSpace;
	mHeap = NULL;
	mHeapTop = NULL;
	memset(mObjects, 0, sizeof(struct Object*) * 16);
	memset(mMessages, 0, sizeof(struct Message*) * 16);
}

struct Object *Process::Object(int obj)
{
	if(obj == INVALID_OBJECT) {
		return NULL;
	} else {
		return mObjects[obj];
	}
}

int Process::RefObject(struct Object *object)
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

int Process::RefObjectTo(int obj, struct Object *object)
{
	if(mObjects[obj] != NULL || object == NULL) {
		return INVALID_OBJECT;
	}

	mObjects[obj] = object;
	return obj;
}

void Process::UnrefObject(int obj)
{
	if(obj != INVALID_OBJECT) {
		mObjects[obj] = NULL;
	}
}

int Process::DupObjectRef(Process *sourceProcess, int sourceObj)
{
	if(sourceObj == INVALID_OBJECT) {
		return INVALID_OBJECT;
	}

	return RefObject(sourceProcess->Object(sourceObj));
}

int Process::DupObjectRefTo(int obj, Process *sourceProcess, int sourceObj)
{
	if(sourceObj == INVALID_OBJECT) {
		return INVALID_OBJECT;
	}

	return RefObjectTo(obj, sourceProcess->Object(sourceObj));
}

struct Message *Process::Message(int msg)
{
	return mMessages[msg];
}

int Process::RefMessage(struct Message *message)
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

void Process::UnrefMessage(int msg)
{
	mMessages[msg] = NULL;
}

void Process::Init()
{
	Kernel = new Process(KernelSpace);
}

void *Process::operator new(size_t size)
{
	return processSlab.Allocate();
}