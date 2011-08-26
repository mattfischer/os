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
	SyscallObjectPost,
	SyscallObjectReceive,
	SyscallMessageRead,
	SyscallMessageReply,
	SyscallMessageInfo,
	SyscallKernelGetObject,
	SyscallKernelSetObject
};

#endif