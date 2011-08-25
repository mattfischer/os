#ifndef SYSTEM_H
#define SYSTEM_H

#include <kernel/include/Syscalls.h>

int Write(int obj, void *buffer, int size);

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size);

void SetName(const char *name, int obj);
int LookupName(const char *name);

int GetKernelObject(enum KernelObject idx);
void SetKernelObject(enum KernelObject idx, int obj);

void SpawnProcess(const char *name, int stdinObject, int stoutObject, int stderrObject);

void Yield();

#endif