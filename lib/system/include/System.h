#ifndef SYSTEM_H
#define SYSTEM_H

int Write(int obj, void *buffer, int size);

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size);

void SetName(const char *name, int obj);
int LookupName(const char *name);

int GetProcessManager();

void SpawnProcess(const char *name, int stdinObject, int stoutObject, int stderrObject);

void Yield();

#endif