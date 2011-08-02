#include "Process.h"
#include "Slab.h"

static struct SlabAllocator processSlab;

struct Process *Process_Create()
{
	struct Process *process = Slab_Allocate(&processSlab);

	process->addressSpace = AddressSpace_Create();

	return process;
}

void Process_Init()
{
	Slab_Init(&processSlab, sizeof(struct Process));
}
