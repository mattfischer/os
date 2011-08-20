#ifndef PROCESS_H
#define PROCESS_H

#include "AddressSpace.h"

class Object;

class Process {
public:
	Process(AddressSpace *addressSpace = NULL);

	static void init();
	static Process *Kernel;

	AddressSpace *addressSpace() { return mAddressSpace; }

	struct MemAreaPages *heap() { return mHeap; }
	void setHeap(struct MemAreaPages *heap) { mHeap = heap; }

	char *heapTop() { return mHeapTop; }
	void setHeapTop(char *heapTop) { mHeapTop = heapTop; }

	char *heapAreaTop() { return mHeapAreaTop; }
	void setHeapAreaTop(char *heapAreaTop) { mHeapAreaTop = heapAreaTop; }

	struct Object *object(int obj);
	int refObject(struct Object *object);
	int refObjectTo(int obj, struct Object *object);
	void unrefObject(int obj);

	int dupObjectRef(Process *sourceProcess, int sourceObj);
	int dupObjectRefTo(int obj, Process *sourceProcess, int sourceObj);

	struct Message *message(int msg);
	int refMessage(struct Message *message);
	void unrefMessage(int msg);

	void *operator new(size_t size) { return sSlab.allocate(); }

private:
	AddressSpace *mAddressSpace;
	struct MemAreaPages *mHeap;
	char *mHeapTop;
	char *mHeapAreaTop;
	struct Object *mObjects[16];
	struct Message *mMessages[16];

	static SlabAllocator<Process> sSlab;
};

#endif