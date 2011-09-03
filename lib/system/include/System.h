#ifndef SYSTEM_H
#define SYSTEM_H

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size);

void SpawnProcess(const char *name, int stdinObject, int stoutObject, int stderrObject);

void Yield();

#endif