#ifndef SYSTEM_H
#define SYSTEM_H

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size);

void SpawnProcess(const char *name, int stdinObject, int stoutObject, int stderrObject);

void Yield();

void Interrupt_Subscribe(unsigned irq, int object, unsigned type, unsigned value);
void Interrupt_Unsubscribe(unsigned irq, int sub);
void Interrupt_Acknowledge(unsigned irq, int sub);

#endif