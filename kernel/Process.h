#ifndef PROCESS_H
#define PROCESS_H

#include "AddressSpace.h"

struct Process {
	struct AddressSpace *addressSpace;
};

struct Process *Process_Create();

void Process_Init();
#endif