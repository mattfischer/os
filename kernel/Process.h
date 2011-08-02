#ifndef PROCESS_H
#define PROCESS_H

#include "AddressSpace.h"
#include "Object.h"

struct Process {
	struct AddressSpace *addressSpace;
	struct Object *objects[16];
	struct Message *messages[16];
};

struct Process *Process_Create();

int Process_RefObject(struct Process *process, struct Object *object);
void Process_UnrefObject(struct Process *process, int n);

int Process_RefMessage(struct Process *process, struct Message *message);
void Process_UnrefMessage(struct Process *process, int n);

void Process_Init();
#endif