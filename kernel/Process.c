#include "Process.h"
#include "Slab.h"
#include "Util.h"

static struct SlabAllocator processSlab;

struct Process *Process_Create()
{
	struct Process *process = Slab_Allocate(&processSlab);

	process->addressSpace = AddressSpace_Create();
	memset(process->objects, 0, sizeof(struct Object*) * 16);
	memset(process->messages, 0, sizeof(struct Message*) * 16);

	return process;
}

int Process_RefObject(struct Process *process, struct Object *object)
{
	int i;

	for(i=0; i<16; i++) {
		if(process->objects[i] == NULL) {
			process->objects[i] = object;
			return i;
		}
	}

	return -1;
}

void Process_UnrefObject(struct Process *process, int n)
{
	process->objects[n] = NULL;
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
	Slab_Init(&processSlab, sizeof(struct Process));
}
