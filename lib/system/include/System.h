#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size);

int SpawnProcess(const char *argv[], int stdinObject, int stoutObject, int stderrObject);
int SpawnProcessx(const char *argv[], int stdinObject, int stoutObject, int stderrObject, int nameserverObject);
void WaitProcess(int process);

void Yield();

int Interrupt_Subscribe(unsigned irq, int object, unsigned type, unsigned value);
void Interrupt_Unsubscribe(int sub);
void Interrupt_Acknowledge(int sub);

#ifdef __cplusplus
}
#endif

#endif