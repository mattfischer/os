#ifndef PROCESS_H
#define PROCESS_H

#include "AddressSpace.h"
#include "Object.h"

class Process {
public:
	Process(struct AddressSpace *addressSpace);

	static void Init();
	static Process *Kernel;

	struct AddressSpace *AddressSpace() { return mAddressSpace; }

	struct MemArea *Heap() { return mHeap; }
	void SetHeap(struct MemArea *heap) { mHeap = heap; }

	char *HeapTop() { return mHeapTop; }
	void SetHeapTop(char *heapTop) { mHeapTop = heapTop; }

	char *HeapAreaTop() { return mHeapAreaTop; }
	void SetHeapAreaTop(char *heapAreaTop) { mHeapAreaTop = heapAreaTop; }

	struct Object *Object(int obj);
	int RefObject(struct Object *object);
	int RefObjectTo(int obj, struct Object *object);
	void UnrefObject(int obj);

	int DupObjectRef(Process *sourceProcess, int sourceObj);
	int DupObjectRefTo(int obj, Process *sourceProcess, int sourceObj);

	struct Message *Message(int msg);
	int RefMessage(struct Message *message);
	void UnrefMessage(int msg);

	static void *operator new(size_t size);

private:
	struct AddressSpace *mAddressSpace;
	struct MemArea *mHeap;
	char *mHeapTop;
	char *mHeapAreaTop;
	struct Object *mObjects[16];
	struct Message *mMessages[16];
};

#endif