#ifndef PROCESS_H
#define PROCESS_H

#include "AddressSpace.h"

class Object;
class Message;

class Process {
public:
	Process(AddressSpace *addressSpace = NULL);

	static void init();

	AddressSpace *addressSpace() { return mAddressSpace; }

	MemAreaPages *heap() { return mHeap; }
	void setHeap(MemAreaPages *heap) { mHeap = heap; }

	char *heapTop() { return mHeapTop; }
	void setHeapTop(char *heapTop) { mHeapTop = heapTop; }

	char *heapAreaTop() { return mHeapAreaTop; }
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

	void *operator new(size_t size) { return sSlab.allocate(); }

private:
	AddressSpace *mAddressSpace;
	MemAreaPages *mHeap;
	char *mHeapTop;
	char *mHeapAreaTop;
	Object *mObjects[16];
	Message *mMessages[16];

	static SlabAllocator<Process> sSlab;
};

#endif