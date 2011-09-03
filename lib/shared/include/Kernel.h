#ifndef SHARED_KERNEL_H
#define SHARED_KERNEL_H

#include <kernel/include/Syscalls.h>

int Kernel_GetObject(enum KernelObject idx);
void Kernel_SetObject(enum KernelObject idx, int obj);

#endif