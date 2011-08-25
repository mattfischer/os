#ifndef SYSTEM_H
#define SYSTEM_H

#include <kernel/include/Syscalls.h>

int Write(int obj, void *buffer, int size);

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size);

void Name_Set(const char *name, int obj);
int Name_Lookup(const char *name);

int Kernel_GetObject(enum KernelObject idx);
void Kernel_SetObject(enum KernelObject idx, int obj);

void SpawnProcess(const char *name, int stdinObject, int stoutObject, int stderrObject);

void Yield();

#endif