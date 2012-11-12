#ifndef SHARED_KERNEL_H
#define SHARED_KERNEL_H

#include <kernel/include/Syscalls.h>

#ifdef __cplusplus
extern "C" {
#endif

int Kernel_GetNameServer();
void Kernel_SetNameServer(int obj);

#ifdef __cplusplus
}
#endif

#endif