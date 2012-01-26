#ifndef SHARED_KERNEL_H
#define SHARED_KERNEL_H

#include <kernel/include/Syscalls.h>

#ifdef __cplusplus
extern "C" {
#endif

int Kernel_GetObject(enum KernelObject idx);
void Kernel_SetObject(enum KernelObject idx, int obj);

#ifdef __cplusplus
}
#endif

#endif