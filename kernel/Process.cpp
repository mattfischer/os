#include "Process.h"
#include "Slab.h"
#include "Util.h"

static struct SlabAllocator<struct Process> processSlab;

struct Process *KernelProcess;

struct Process *Process_Create(struct AddressSpace *addressSpace)
{
	struct Process *process = processSlab.Allocate();

	process->addressSpace = addressSpace;
	process->heap = NULL;
	process->heapTop = NULL;
	memset(process->objects, 0, sizeof(struct Object*) * 16);
	memset(process->messages, 0, sizeof(struct Message*) * 16);

	return process;
}

int Process_RefObject(struct Process *process, struct Object *object)
{
	int i;

	if(object == NULL) {
		return INVALID_OBJECT;
	}

	for(i=0; i<16; i++) {
		if(process->objects[i] == NULL) {
			process->objects[i] = object;
			return i;
		}
	}

	return INVALID_OBJECT;
}

int Process_RefObjectTo(struct Process *process, int obj, struct Object *object)
{
	if(process->objects[obj] != NULL || object == NULL) {
		return INVALID_OBJECT;
	}

	process->objects[obj] = object;
	return obj;
}

void Process_UnrefObject(struct Process *process, int n)
{
	if(n != INVALID_OBJECT) {
		process->objects[n] = NULL;
	}
}

int Process_DupObjectRef(struct Process *process, struct Process *sourceProcess, int sourceObject)
{
	if(sourceObject == INVALID_OBJECT) {
		return INVALID_OBJECT;
	}

	return Process_RefObject(process, sourceProcess->objects[sourceObject]);
}

int Process_DupObjectRefTo(struct Process *process, int obj, struct Process *sourceProcess, int sourceObject)
{
	if(sourceObject == INVALID_OBJECT) {
		return INVALID_OBJECT;
	}

	return Process_RefObjectTo(process, obj, sourceProcess->objects[sourceObject]);
}

int Process_RefMessage(struct Process *process, struct Message *message)
{
	int i;

	for(i=0; i<16; i++) {
		if(process->messages[i] == NULL) {
			process->messages[i] = message;
			return i;
		}
	}

	return -1;
}

void Process_UnrefMessage(struct Process *process, int n)
{
	process->messages[n] = NULL;
}

void Process_Init()
{
	KernelProcess = Process_Create(KernelSpace);
}
