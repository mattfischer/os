#ifndef SYSCALLS_H
#define SYSCALLS_H

enum KernelObject {
	KernelObjectProcManager,
	KernelObjectNameServer,
	KernelObjectCount
};

enum Syscall {
	SyscallYield,
	SyscallObjectCreate,
	SyscallObjectRelease,
	SyscallObjectSend,
	SyscallObjectReceive,
	SyscallMessageRead,
	SyscallMessageReply,
	SyscallMessageInfo,
	SyscallKernelGetObject,
	SyscallKernelSetObject
};

#endif