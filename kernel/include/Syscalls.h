#ifndef SYSCALLS_H
#define SYSCALLS_H

enum KernelObject {
	KernelObjectProcManager,
	KernelObjectNameServer,
	KernelObjectCount
};

enum Syscall {
	SyscallYield,
	SyscallObjectSend,
	SyscallObjectReceive,
	SyscallMessageRead,
	SyscallMessageReply,
	SyscallObjectCreate,
	SyscallObjectRelease,
	SyscallKernelGetObject,
	SyscallKernelSetObject
};

#endif